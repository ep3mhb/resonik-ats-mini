#ifndef DIGI_DECODERS_H
#define DIGI_DECODERS_H

// RESONIK R5: RTTY, NAVTEX (SITOR-B) and PSK31 decoders for the radio's
// audio, as plain C++ with no Arduino dependencies so they are tested on
// the host (tests/TestDigi.cpp). Each takes samples at a fixed rate and the
// audio frequency of the signal (the page finds it on its spectrum).
//
// RTTY: two tone filters one bit long, start-bit framing, ITA2 with
// unshift-on-space. Both polarities are decoded at once and the one with
// fewer framing errors is shown, so LSB, USB and reversed stations all read.
// NAVTEX: 100 baud, 170 Hz shift, a bit clock from the tone transitions,
// CCIR 476 (4 of 7 bits set) with its time-diversity copy five characters
// earlier used when the first one is spoiled.
// PSK31: carrier mixed to zero, filtered one symbol long, sampled where the
// symbol is strongest, phase reversal = 0, varicode between "00" gaps.

#include <stdint.h>
#include <math.h>
#include <string.h>

namespace Digi {

static const int TEXT_MAX = 120;

// The whole text, line breaks kept: the last `size` characters in a ring the
// page provides, for reading back on the radio and for the browser. A head
// line (mode, frequency, time) is written before the first character that
// follows each start of a decoder, so a start that hears nothing leaves no
// trace. Holds printable ASCII and '\n' only, at most one empty line in a row.
struct Log
{
  char *buf = nullptr;
  uint32_t size = 0;
  uint32_t total = 0;              // Characters ever written
  uint8_t breaks = 1;              // Line breaks at the end (1: at a line start)
  char head[72] = "";              // Written before the next character
  void raw(char c) { buf[total % size] = c; total++; }
  void put(char c)
  {
    if(!buf || !size) return;
    if(c == '\n') { if(breaks < 2 && !head[0]) { raw(c); breaks++; } return; }
    if(c < 32 || c > 126) return;
    if(head[0])
    {
      if(c == ' ') return;
      while(total && breaks < 2) { raw('\n'); breaks++; }
      for(const char *h = head; *h; h++) raw(*h);
      raw('\n');
      head[0] = '\0';
    }
    raw(c);
    breaks = 0;
  }
  uint32_t first() const { return(total > size ? total - size : 0); }
  char at(uint32_t i) const { return(buf[i % size]); }
};

// Text kept for the screen: the last TEXT_MAX characters, one line
struct Text
{
  char buf[TEXT_MAX + 1];
  int len = 0;
  uint32_t count = 0;              // Characters ever added (to spot news)
  Log *log = nullptr;              // The whole text goes there too
  void clear() { len = 0; buf[0] = '\0'; }
  void put(char c)
  {
    if(log) log->put(c);
    if(c == '\r') return;
    if(c == '\n') c = ' ';
    if(c == ' ' && len && buf[len - 1] == ' ') return;
    if(len >= TEXT_MAX) { memmove(buf, buf + 1, TEXT_MAX - 1); len = TEXT_MAX - 1; }
    buf[len++] = c;
    buf[len] = '\0';
    count++;
  }
};

// A tone filter: mixes the tone to zero and averages over n samples
struct ToneFilter
{
  float c = 1, s = 0, dc = 1, ds = 0;   // Oscillator and its step
  float *re = nullptr, *im = nullptr;
  int n = 0, pos = 0;
  float sumRe = 0, sumIm = 0;
  float store[2 * 400];
  void begin(float fs, float hz, int len)
  {
    if(len > 400) len = 400;
    n = len; pos = 0; sumRe = sumIm = 0;
    re = store; im = store + 400;
    memset(store, 0, sizeof(store));
    setFreq(fs, hz);
  }
  void setFreq(float fs, float hz) { dc = cosf(2 * (float)M_PI * hz / fs); ds = sinf(2 * (float)M_PI * hz / fs); }
  float push(float x)
  {
    float r = x * c, i = -x * s;
    float nc = c * dc - s * ds, ns = c * ds + s * dc;
    c = nc; s = ns;
    sumRe += r - re[pos]; sumIm += i - im[pos];
    re[pos] = r; im[pos] = i;
    if(++pos >= n) { pos = 0; float g = 1.5f - 0.5f * (c * c + s * s); c *= g; s *= g; }
    return(sqrtf(sumRe * sumRe + sumIm * sumIm) / n);
  }
};

//
// RTTY
//
class Rtty
{
public:
  void begin(float fs, float baud, float shift, float centre)
  {
    this->fs = fs; this->baud = baud; this->shift = shift;
    spb = fs / baud;
    int len = (int)(spb + 0.5f);
    mark.begin(fs, centre + shift / 2, len);
    space.begin(fs, centre - shift / 2, len);
    for(int p = 0; p < 2; p++) { fr[p] = Framer(); fr[p].spb = spb; }
    text.clear();
    score[0] = score[1] = 0;
    pick = 0;
  }
  void setCentre(float centre) { mark.setFreq(fs, centre + shift / 2); space.setFreq(fs, centre - shift / 2); }
  void push(const int16_t *x, int n)
  {
    for(int i = 0; i < n; i++)
    {
      float m = mark.push(x[i]), s = space.push(x[i]);
      // Each tone against its own recent peak, so a faded tone still counts
      mPk = m > mPk ? m : mPk * 0.9995f;
      sPk = s > sPk ? s : sPk * 0.9995f;
      float d = m / (mPk + 1e-3f) - s / (sPk + 1e-3f);
      level = (m + s) * 0.001f + level * 0.999f;
      for(int p = 0; p < 2; p++)
      {
        int code = fr[p].push(p ? -d : d);
        if(code != -1) score[p] *= 0.97f;   // Per character, about the last 30
        if(code == -2) score[p] -= 1.0f;
        else if(code >= 0)
        {
          score[p] += 1.0f;
          // The polarity with clearly fewer framing errors is shown
          if(score[p] > score[1 - p] + 3.0f) pick = p;
          if(p == pick) out(p, code);
        }
      }
    }
  }
  Text text;
  float level = 0;
  bool reversed() const { return(pick == 1); }

private:
  struct Framer
  {
    float spb = 132;
    float prev = 1, t = 0;
    int state = 0, bits = 0, k = 0;
    // Returns a 5-bit code, -2 for a framing error, -1 for nothing yet
    int push(float d)
    {
      int r = -1;
      if(state == 0)
      {
        if(prev > 0 && d <= 0) { state = 1; t = 0; }   // Mark to space: a start bit?
      }
      else
      {
        t += 1;
        float at = (k + 0.5f) * spb;
        if(t >= at)
        {
          bool one = d > 0;
          if(k == 0) { if(one) state = 0; else { bits = 0; k = 1; } }
          else if(k <= 5) { if(one) bits |= 1 << (k - 1); k++; }
          else { r = one ? bits : -2; state = 0; k = 0; }
          if(state == 0) k = 0;
        }
      }
      prev = d;
      return(r);
    }
  };
  float fs = 6000, baud = 45.45f, shift = 170, spb = 132;
  ToneFilter mark, space;
  Framer fr[2];
  float score[2] = { 0, 0 };
  int pick = 0;
  float mPk = 0, sPk = 0;
  bool figs[2] = { false, false };

  void out(int p, int code)
  {
    static const char ltrs[32] = { 0, 'E', '\n', 'A', ' ', 'S', 'I', 'U', '\r', 'D', 'R', 'J', 'N', 'F', 'C', 'K',
      'T', 'Z', 'L', 'W', 'H', 'Y', 'P', 'Q', 'O', 'B', 'G', 0, 'M', 'X', 'V', 0 };
    static const char fig[32] = { 0, '3', '\n', '-', ' ', '\'', '8', '7', '\r', '$', '4', '\'', ',', '!', ':', '(',
      '5', '+', ')', '2', '#', '6', '0', '1', '9', '?', '&', 0, '.', '/', '=', 0 };
    if(code == 0x1B) { figs[p] = true; return; }
    if(code == 0x1F) { figs[p] = false; return; }
    char c = figs[p] ? fig[code] : ltrs[code];
    if(code == 0x04) figs[p] = false;          // Unshift on space
    if(c) text.put(c);
  }
};

//
// NAVTEX (SITOR-B, CCIR 476)
//
class Navtex
{
public:
  void begin(float fs, float centre)
  {
    this->fs = fs;
    spb = fs / 100.0f;
    int len = (int)(spb + 0.5f);
    mark.begin(fs, centre + 85, len);
    space.begin(fs, centre - 85, len);
    phase = 0; prev = 0; nbits = 0; synced = false; figs = false; sinceSync = 0;
    text.clear();
    buildTables();
  }
  void setCentre(float centre) { mark.setFreq(fs, centre + 85); space.setFreq(fs, centre - 85); }
  void push(const int16_t *x, int n)
  {
    for(int i = 0; i < n; i++)
    {
      float m = mark.push(x[i]), s = space.push(x[i]);
      mPk = m > mPk ? m : mPk * 0.999f;
      sPk = s > sPk ? s : sPk * 0.999f;
      float d = m / (mPk + 1e-3f) - s / (sPk + 1e-3f);
      level = (m + s) * 0.001f + level * 0.999f;
      // Bit clock: a transition should fall half a bit from the sample point
      if((d > 0) != (prev > 0))
      {
        float err = phase - 0.5f;   // 0.5 is mid-way between samples
        phase -= err * 0.15f;
      }
      prev = d;
      phase += 1.0f / spb;
      if(phase >= 1.0f) { phase -= 1.0f; takeBit(d); }
    }
  }
  Text text;
  float level = 0;
  bool synced = false;

private:
  float fs = 6000, spb = 60, phase = 0, prev = 0;
  ToneFilter mark, space;
  float mPk = 0, sPk = 0;
  static const int HIST = 256;
  int8_t hist[HIST];               // Bit history, +1 mark (B), -1 space (Y)
  int nbits = 0;                   // Bits stored
  int align = 0;                   // Bit number of alpha characters, mod 14
  int sinceSync = 0;
  bool figs = false;
  bool inverted = false;
  int searches = 0;
  char ltrs[128], fgs[128];

  void buildTables()
  {
    memset(ltrs, 0, sizeof(ltrs)); memset(fgs, 0, sizeof(fgs));
    static const struct { uint8_t c; char l, f; } t[] = {
      {0x17,'J','\''},{0x1B,'F','!'},{0x1D,'C',':'},{0x1E,'K','('},{0x27,'W','2'},{0x2B,'Y','6'},{0x2D,'P','0'},
      {0x2E,'Q','1'},{0x35,'G','&'},{0x39,'M','.'},{0x3A,'X','/'},{0x3C,'V',';'},{0x47,'A','-'},{0x4B,'S','\a'},
      {0x4D,'I','8'},{0x4E,'U','7'},{0x53,'D','$'},{0x55,'R','4'},{0x56,'E','3'},{0x59,'N',','},{0x5C,' ',' '},
      {0x63,'Z','"'},{0x65,'L',')'},{0x69,'H','#'},{0x6C,'\n','\n'},{0x71,'O','9'},{0x72,'B','?'},{0x74,'T','5'},
      {0x78,'\r','\r'},
    };
    for(unsigned i = 0; i < sizeof(t) / sizeof(t[0]); i++) { ltrs[t[i].c] = t[i].l; fgs[t[i].c] = t[i].f; }
  }
  static const int LTRS = 0x5A, FIGS = 0x36, ALPHA = 0x0F, REP = 0x66, BETA = 0x33, C32 = 0x6A;
  static bool valid(int c) { int n = 0; for(int i = 0; i < 7; i++) n += (c >> i) & 1; return(n == 4); }
  // The 7-bit code whose first bit is bit number j (first sent = lowest bit)
  int codeAbs(int j) const
  {
    int c = 0;
    for(int i = 0; i < 7; i++) if(hist[(j + i) % HIST] > 0) c |= 1 << i;
    return(c);
  }

  void takeBit(float d)
  {
    hist[nbits % HIST] = (d > 0) != inverted ? 1 : -1;
    nbits++;
    if(nbits < 160) return;
    if(!synced)
    {
      // Look for the alpha stream: the offset (mod 14) with the most valid
      // characters whose copy, five characters (35 bits) earlier, matches
      if(nbits % 7) return;
      int best = -1, bestScore = 0;
      for(int a = 0; a < 14; a++)
      {
        int score = 0, reps = 0;
        for(int j = nbits - 7; j >= nbits - 120; j--)
        {
          if(j % 14 != a) continue;
          int c = codeAbs(j);
          if(!valid(c)) continue;
          score++;
          int r = codeAbs(j - 35);
          if(r == c && c != ALPHA && c != REP) reps++;
          else if(c == ALPHA && codeAbs(j - 7) == REP) reps++;
        }
        if(reps >= 3 && score + reps > bestScore) { bestScore = score + reps; best = a; }
      }
      if(best >= 0 && bestScore >= 10) { synced = true; align = best; sinceSync = 0; }
      // Nothing valid this way round for a while: try the other polarity
      else if(++searches >= 40) { searches = 0; inverted = !inverted; for(int i = 0; i < HIST; i++) hist[i] = -hist[i]; }
      return;
    }
    // One alpha character every 14 bits, once all 7 bits of it are in
    int j = nbits - 7;
    if(j % 14 != align) return;
    int c = codeAbs(j);
    if(!valid(c))
    {
      int r = codeAbs(j - 35);
      c = valid(r) ? r : -1;
    }
    if(c < 0) { if(++sinceSync > 20) synced = false; text.put('_'); return; }
    sinceSync = 0;
    if(c == LTRS) { figs = false; return; }
    if(c == FIGS) { figs = true; return; }
    if(c == ALPHA || c == REP || c == BETA || c == C32) return;
    char ch = figs ? fgs[c] : ltrs[c];
    if(ch == '\a') return;
    if(ch) text.put(ch);
  }
};

//
// PSK31 (BPSK, 31.25 baud, varicode)
//
class Psk31
{
public:
  void begin(float fs, float carrier)
  {
    this->fs = fs;
    spb = fs / 31.25f;
    n = (int)(spb + 0.5f);
    if(n > 400) n = 400;
    freq = carrier;
    memset(re, 0, sizeof(re)); memset(im, 0, sizeof(im));
    pos = 0; sRe = sIm = 0; ph = 0; t = 0;
    for(int i = 0; i < 16; i++) bins[i] = 0;
    pRe = pIm = 0; code = 0; zeros = 0;
    quality = 0;
    text.clear();
  }
  void setCarrier(float hz) { freq = hz; }
  float carrier() const { return(freq); }
  void push(const int16_t *x, int count)
  {
    for(int i = 0; i < count; i++)
    {
      ph += 2 * (float)M_PI * freq / fs;
      if(ph > (float)M_PI) ph -= 2 * (float)M_PI;
      float r = x[i] * cosf(ph), q = -x[i] * sinf(ph);
      sRe += r - re[pos]; sIm += q - im[pos];
      re[pos] = r; im[pos] = q;
      if(++pos >= n) pos = 0;
      // Strength by position in the symbol (16 bins), slowly averaged:
      // the symbol is sampled in the strongest bin
      float a = sRe * sRe + sIm * sIm;
      int b = (int)(t * 16 / spb);
      if(b > 15) b = 15;
      bins[b] = bins[b] * 0.99f + a * 0.01f;
      if(b == best)
      {
        if(!taken) { symbol(sRe, sIm); taken = true; }
      }
      else taken = false;
      t += 1;
      if(t >= spb)
      {
        t -= spb;
        int m = 0;
        for(int k = 1; k < 16; k++) if(bins[k] > bins[m]) m = k;
        best = m;
      }
    }
  }
  Text text;
  float quality = 0;               // 0..1: how cleanly the phases are 0 or 180 degrees

private:
  float fs = 6000, spb = 192, freq = 1000, ph = 0, t = 0;
  int n = 192, pos = 0, best = 0;
  bool taken = false;
  float re[400], im[400], sRe = 0, sIm = 0;
  float bins[16];
  float pRe = 0, pIm = 0;
  uint32_t code = 0;
  int zeros = 0;

  void symbol(float r, float q)
  {
    // Phase change from the previous symbol
    float dRe = r * pRe + q * pIm, dIm = q * pRe - r * pIm;
    pRe = r; pIm = q;
    float mag = sqrtf(dRe * dRe + dIm * dIm) + 1e-6f;
    int bit = dRe > 0 ? 1 : 0;
    // Carrier tracking from the residual phase (folded to the nearest of 0, 180)
    float sRe2 = bit ? dRe : -dRe, sIm2 = bit ? dIm : -dIm;
    float err = atan2f(sIm2, sRe2);
    freq += err * fs / (2 * (float)M_PI * spb) * 0.08f;
    quality = quality * 0.95f + 0.05f * (fabsf(dRe) / mag);
    if(bit) { zeros = 0; code = (code << 1) | 1; if(code > 0x3FF) code = 0x3FF; }
    else if(++zeros == 2) { if(code) put(code); code = 0; }
    else if(zeros == 1) code <<= 1;
  }
  void put(uint32_t c)
  {
    // The code was shifted once for its first trailing zero
    c >>= 1;
    static const uint16_t vari[128] = {
    0x2AB, 0x2DB, 0x2ED, 0x377, 0x2EB, 0x35F, 0x2EF, 0x2FD,
    0x2FF, 0x0EF, 0x01D, 0x36F, 0x2DD, 0x01F, 0x375, 0x3AB,
    0x2F7, 0x2F5, 0x3AD, 0x3AF, 0x35B, 0x36B, 0x36D, 0x357,
    0x37B, 0x37D, 0x3B7, 0x355, 0x35D, 0x3BB, 0x2FB, 0x37F,
    0x001, 0x1FF, 0x15F, 0x1F5, 0x1DB, 0x2D5, 0x2BB, 0x17F,
    0x0FB, 0x0F7, 0x16F, 0x1DF, 0x075, 0x035, 0x057, 0x1AF,
    0x0B7, 0x0BD, 0x0ED, 0x0FF, 0x177, 0x15B, 0x16B, 0x1AD,
    0x1AB, 0x1B7, 0x0F5, 0x1BD, 0x1ED, 0x055, 0x1D7, 0x2AF,
    0x2BD, 0x07D, 0x0EB, 0x0AD, 0x0B5, 0x077, 0x0DB, 0x0FD,
    0x155, 0x07F, 0x1FD, 0x17D, 0x0D7, 0x0BB, 0x0DD, 0x0AB,
    0x0D5, 0x1DD, 0x0AF, 0x06F, 0x06D, 0x157, 0x1B5, 0x15D,
    0x175, 0x17B, 0x2AD, 0x1F7, 0x1EF, 0x1FB, 0x2BF, 0x16D,
    0x2DF, 0x00B, 0x05F, 0x02F, 0x02D, 0x003, 0x03D, 0x05B,
    0x02B, 0x00D, 0x1EB, 0x0BF, 0x01B, 0x03B, 0x00F, 0x007,
    0x03F, 0x1BF, 0x015, 0x017, 0x005, 0x037, 0x07B, 0x06B,
    0x0DF, 0x05D, 0x1D5, 0x2B7, 0x1BB, 0x2B5, 0x2D7, 0x3B5,
    };
    if(quality < 0.6f) return;     // Squelch: noise prints nothing
    for(int i = 0; i < 128; i++) if(vari[i] == c)
    {
      if(i == '\n' || (i >= 32 && i < 127)) text.put((char)i);
      return;
    }
  }
};

//
// WEFAX (radiofax): 120 lines a minute, IOC 576, black 1500 Hz, white
// 2300 Hz. The frequency of the carrier is measured sample by sample and
// averaged into 300 pixels a line; phasing lines (black with a short white
// pulse at the line start) set where a line begins.
//
class Wefax
{
public:
  static const int W = 300;          // Pixels per line kept
  static const int LINES = 128;      // Lines kept
  void begin(float fs)
  {
    this->fs = fs;
    spl = fs * 0.5f;                 // Samples per line
    ph = 0; pRe = 1; pIm = 0;
    for(int i = 0; i < 4; i++) { bRe[i] = bIm[i] = 0; }
    bPos = 0; aRe = aIm = 0;
    t = 0; px = 0; acc = 0; accN = 0;
    lines = 0; head = 0; align = 0;
    memset(pulseVotes, 0, sizeof(pulseVotes));
    memset(img, 0, sizeof(img));
  }
  void push(const int16_t *x, int n)
  {
    for(int i = 0; i < n; i++)
    {
      // Mix 1900 Hz to zero, smooth four samples, measure the phase step
      ph += 2 * (float)M_PI * 1900 / fs;
      if(ph > (float)M_PI) ph -= 2 * (float)M_PI;
      float r = x[i] * cosf(ph), q = -x[i] * sinf(ph);
      aRe += r - bRe[bPos]; aIm += q - bIm[bPos];
      bRe[bPos] = r; bIm[bPos] = q;
      bPos = (bPos + 1) & 3;
      float dRe = aRe * pRe + aIm * pIm, dIm = aIm * pRe - aRe * pIm;
      pRe = aRe; pIm = aIm;
      float hz = 1900 + atan2f(dIm, dRe) * fs / (2 * (float)M_PI);
      float v = (hz - 1500) / 800;
      acc += v < 0 ? 0 : v > 1 ? 1 : v;
      accN++;
      t += 1;
      int p = (int)(t * W / spl);
      if(p != px && p < W)
      {
        line[px] = (uint8_t)(acc / accN * 255);
        acc = 0; accN = 0; px = p;
      }
      if(t >= spl)
      {
        t -= spl;
        line[W - 1] = accN ? (uint8_t)(acc / accN * 255) : line[W - 2];
        acc = 0; accN = 0; px = 0;
        endLine();
      }
    }
  }
  const uint8_t *row(int back) const { return(img[(head - 1 - back + LINES) % LINES]); }
  int count() const { return(lines < LINES ? lines : LINES); }
  uint32_t total() const { return(lines); }

private:
  float fs = 6000, spl = 3000, ph = 0, pRe = 1, pIm = 0, t = 0;
  float bRe[4], bIm[4], aRe = 0, aIm = 0;
  int bPos = 0, px = 0, accN = 0;
  float acc = 0;
  uint8_t line[W];
  uint8_t img[LINES][W];
  uint32_t lines = 0;
  int head = 0, align = 0;
  uint16_t pulseVotes[W];

  void endLine()
  {
    // A phasing line: mostly black with one short white stretch (about 5 %
    // of the line). Its start, seen over several lines, is the line start.
    int white = 0, first = -1;
    for(int i = 0; i < W; i++) if(line[i] > 160) { if(first < 0) first = i; white++; }
    if(white >= W / 40 && white <= W / 8 && first >= 0)
    {
      pulseVotes[first]++;
      int best = 0;
      for(int i = 1; i < W; i++) if(pulseVotes[i] > pulseVotes[best]) best = i;
      if(pulseVotes[best] >= 5) align = best;
    }
    uint8_t *dst = img[head];
    for(int i = 0; i < W; i++) dst[i] = line[(i + align) % W];
    head = (head + 1) % LINES;
    lines++;
  }
};

} // namespace Digi

#endif // DIGI_DECODERS_H
