#ifndef CW_DECODER_H
#define CW_DECODER_H

// RESONIK: Morse (CW) decoder for audio samples from the speaker line.
//
// A Goertzel filter measures the tone at the CW pitch in short blocks, an
// adaptive threshold between the noise floor and the signal peak turns that
// into key down / key up, and the mark and space lengths, measured against a
// running dot length, give dots, dashes, letters and words. The speed adapts
// by itself from about 5 to 40 WPM. Two more Goertzel filters, 350 Hz either
// side of the pitch, watch for broadband noise: static crashes and clicks
// raise them as much as the pitch and do not key (they printed rows of E).
// No Arduino dependencies, so it is tested
// on the host (tests/TestCwDecoder.cpp).

#include <stdint.h>
#include <math.h>
#include <string.h>

class CwDecoder
{
public:
  // fs: sample rate in Hz, tone: CW pitch in Hz
  void begin(float fs, float tone)
  {
    this->fs = fs;
    blockLen = (int)(fs * 0.006f + 0.5f);   // 6 ms blocks: fast enough for 40 WPM
    if(blockLen < 16) blockLen = 16;
    blockMs = 1000.0f * blockLen / fs;
    setTone(tone);
    reset();
  }

  void setTone(float tone)
  {
    this->tone = tone;
    coeff = 2.0f * cosf(2.0f * (float)M_PI * tone / fs);
    // Reference frequencies beside the pitch, kept inside 150 Hz .. fs/2-150 Hz
    float lo = tone - 350.0f, hi = tone + 350.0f;
    if(lo < 150.0f) lo = tone + 700.0f;
    if(hi > fs / 2 - 150.0f) hi = tone - 700.0f;
    coeffRef[0] = 2.0f * cosf(2.0f * (float)M_PI * lo / fs);
    coeffRef[1] = 2.0f * cosf(2.0f * (float)M_PI * hi / fs);
  }

  void reset()
  {
    q1 = q2 = 0;
    r1[0] = r1[1] = r2[0] = r2[1] = 0;
    n = 0;
    peak = 0;
    noise = 0;
    noiseRef[0] = noiseRef[1] = 0;
    keyDown = false;
    pending = 0;
    runMs = 0;
    ditMs = 60.0f;                            // 20 WPM until the signal tells otherwise
    symLen = 0;
    sym[0] = '\0';
    wordDone = true;
    textLen = 0;
    text[0] = '\0';
    level = 0;
  }

  // Feed audio samples (DC is removed here, any offset is fine)
  void push(const int16_t *s, int count)
  {
    for(int i = 0 ; i < count ; i++)
    {
      // Slow DC tracker so the raw ADC value can be fed in directly
      dc += ((float)s[i] - dc) * 0.002f;
      float x = (float)s[i] - dc;
      float q0 = coeff * q1 - q2 + x;
      q2 = q1;
      q1 = q0;
      for(int k = 0 ; k < 2 ; k++)
      {
        float r0 = coeffRef[k] * r1[k] - r2[k] + x;
        r2[k] = r1[k];
        r1[k] = r0;
      }
      if(++n >= blockLen)
      {
        float mag = sqrtf(fmaxf(q1 * q1 + q2 * q2 - coeff * q1 * q2, 0.0f)) / blockLen;
        float ref = 0;
        for(int k = 0 ; k < 2 ; k++)
        {
          float m = sqrtf(fmaxf(r1[k] * r1[k] + r2[k] * r2[k] - coeffRef[k] * r1[k] * r2[k], 0.0f)) / blockLen;
          // Reference noise floors follow the key-up blocks
          if(noiseRef[k] == 0) noiseRef[k] = m;
          else if(!keyDown) noiseRef[k] += (m - noiseRef[k]) * 0.02f;
          float rel = m / (noiseRef[k] + 1e-3f);
          if(rel > ref) ref = rel;
          r1[k] = r2[k] = 0;
        }
        q1 = q2 = 0;
        n = 0;
        block(mag, ref);
      }
    }
  }

  const char *getText() const { return(text); }
  int getWpm() const { return((int)(1200.0f / ditMs + 0.5f)); }
  bool isKeyDown() const { return(keyDown); }
  // Tone level against the threshold, 0..100, for a tuning bar
  uint8_t getLevel() const { return(level); }
  float getTone() const { return(tone); }

  void clearText() { textLen = 0; text[0] = '\0'; }

  static const int TEXT_MAX = 96;

private:
  float fs = 8000, tone = 700, coeff = 0;
  float coeffRef[2] = { 0, 0 }, r1[2] = { 0, 0 }, r2[2] = { 0, 0 }, noiseRef[2] = { 0, 0 };
  int blockLen = 48;
  float blockMs = 6;
  float q1 = 0, q2 = 0, dc = 0;
  int n = 0;

  float peak = 0, noise = 0;
  bool keyDown = false;
  int pending = 0;          // Blocks the opposite state has lasted (debounce)
  float runMs = 0;          // Length of the current mark or space
  float ditMs = 60;
  char sym[8];
  int symLen = 0;
  bool wordDone = true;
  uint8_t level = 0;

  char text[TEXT_MAX + 1];
  int textLen = 0;

  // mag: tone level; ref: the louder reference beside the pitch, against its own noise
  void block(float mag, float ref)
  {
    // Peak follows signals up at once and fades over about a second; the
    // noise floor follows quiet blocks and rises only slowly
    peak = mag > peak ? mag : peak * 0.995f;
    // The noise floor is the average of the key-up blocks; it starts from the
    // first blocks and falls quickly if a signal was there at the start
    if(noise == 0) noise = mag;
    else if(!keyDown) noise += (mag - noise) * (mag < noise ? 0.05f : 0.01f);

    float span = peak - noise;
    float hi = noise + span * 0.55f, lo = noise + span * 0.40f;
    // A tone must stand well clear of the noise (about 10 dB) to key
    float floorOn = noise * 3.2f + 1.0f, floorOff = noise * 2.2f + 1.0f;
    bool on = keyDown ? (mag > lo && mag > floorOff) : (mag > hi && mag > floorOn);
    // Broadband (a crash or click): the pitch is not well above what is
    // heard beside it. A real tone leaks only about 0.15 into the references.
    if(on && !keyDown && ref > 2.0f && ref > (mag / (noise + 1e-3f)) * 0.4f) on = false;

    float l = span > 0 ? (mag - noise) * 100.0f / span : 0;
    level = l < 0 ? 0 : l > 100 ? 100 : (uint8_t)l;

    runMs += blockMs;
    if(on != keyDown)
    {
      // Two blocks in the new state before it counts, to ride over clicks
      if(++pending >= 2)
      {
        float len = runMs - pending * blockMs;
        if(keyDown) mark(len); else space(len);
        keyDown = on;
        runMs = pending * blockMs;
        pending = 0;
      }
    }
    else pending = 0;

    // Long silence ends the letter and the word without waiting for a key
    if(!keyDown)
    {
      if(symLen && runMs > ditMs * 2.5f) endLetter();
      if(!wordDone && runMs > ditMs * 6.0f) { put(' '); wordDone = true; }
    }
  }

  void mark(float ms)
  {
    // A click or a burst of noise, not a dot: real dots are close to the
    // dot length (at least half of it); fast senders pull the dot length
    // down through their dashes
    if(ms < blockMs * 1.5f || ms < ditMs * 0.5f) return;
    bool dash = ms > ditMs * 2.0f;
    // Track the speed: a dot is one unit, a dash three
    float unit = dash ? ms / 3.0f : ms;
    if(unit > 25.0f && unit < 260.0f) ditMs += (unit - ditMs) * 0.25f;
    if(symLen < (int)sizeof(sym) - 1) { sym[symLen++] = dash ? '-' : '.'; sym[symLen] = '\0'; }
    else symLen = sizeof(sym);                       // Too long for any letter
    wordDone = false;
  }

  void space(float ms)
  {
    if(ms > ditMs * 2.5f) endLetter();
    if(ms > ditMs * 6.0f && !wordDone) { put(' '); wordDone = true; }
  }

  void endLetter()
  {
    if(!symLen) return;
    put(symLen < (int)sizeof(sym) ? lookup(sym) : '*');
    symLen = 0;
    sym[0] = '\0';
  }

  void put(char c)
  {
    if(c == ' ' && (textLen == 0 || text[textLen - 1] == ' ')) return;
    if(textLen >= TEXT_MAX)
    {
      memmove(text, text + 1, TEXT_MAX - 1);
      textLen = TEXT_MAX - 1;
    }
    text[textLen++] = c;
    text[textLen] = '\0';
  }

  static char lookup(const char *code)
  {
    static const char *const letters[] =
    {
      ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--",
      "-.", "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--", "-..-", "-.--", "--..",
    };
    static const char *const digits[] =
    {
      "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----.",
    };
    static const struct { const char *code; char c; } other[] =
    {
      { ".-.-.-", '.' }, { "--..--", ',' }, { "..--..", '?' }, { "-..-.", '/' },
      { "-...-", '=' }, { ".-.-.", '+' }, { "-....-", '-' }, { ".--.-.", '@' },
      { "---...", ':' }, { "-.--.", '(' }, { "-.--.-", ')' }, { ".----.", '\'' },
    };
    for(int i = 0 ; i < 26 ; i++) if(!strcmp(code, letters[i])) return('A' + i);
    for(int i = 0 ; i < 10 ; i++) if(!strcmp(code, digits[i])) return('0' + i);
    for(unsigned i = 0 ; i < sizeof(other) / sizeof(other[0]) ; i++)
      if(!strcmp(code, other[i].code)) return(other[i].c);
    return('*');
  }
};

#endif // CW_DECODER_H
