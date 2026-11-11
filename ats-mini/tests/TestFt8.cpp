// Host test for Ft8Rx.h: FT8 signals are synthesised with ft8_lib's encoder,
// mixed with noise, turned into 12-bit ADC samples as GPIO11 gives them and
// decoded through the same code the radio runs.
#include "../Ft8Rx.h"
#include <assert.h>
#include <stdlib.h>
#include <vector>
#include <string>

struct Tx { const char *text; float freq; float snr; float start; };

static double gauss()
{
  static bool have = false;
  static double spare;
  if(have) { have = false; return(spare); }
  double u, v, s;
  do { u = 2.0 * rand() / RAND_MAX - 1; v = 2.0 * rand() / RAND_MAX - 1; s = u * u + v * v; } while(s >= 1 || s == 0);
  s = sqrt(-2.0 * log(s) / s);
  spare = v * s;
  have = true;
  return(u * s);
}

// One 15 s slot of ADC samples (7.5 s for FT4)
static std::vector<int16_t> slot(const std::vector<Tx> &txs, double noiseCounts, int dcOffset, bool ft4 = false)
{
  const int fs = Ft8Rx::FS;
  const int n = ft4 ? fs * 15 / 2 : fs * 15;
  const int nn = ft4 ? FT4_NN : FT8_NN;
  const double spacing = ft4 ? 12000.0 / 576 : 6.25;
  std::vector<double> a(n, 0.0);
  for(const Tx &t : txs)
  {
    ftx_message_t msg;
    ftx_message_init(&msg);
    assert(ftx_message_encode(&msg, nullptr, t.text) == FTX_MESSAGE_RC_OK);
    uint8_t tones[FT4_NN];
    if(ft4) ft4_encode(msg.payload, tones); else ft8_encode(msg.payload, tones);
    // Noise power in 2500 Hz against the signal power A^2 / 2
    double n2500 = noiseCounts * noiseCounts * 2500.0 / (fs / 2.0);
    double amp = sqrt(2.0 * n2500 * pow(10.0, t.snr / 10.0));
    double phase = 0;
    int sps = ft4 ? 576 : 1920;
    int first = (int)(t.start * fs);
    for(int s = 0 ; s < nn ; s++)
      for(int k = 0 ; k < sps ; k++)
      {
        int i = first + s * sps + k;
        phase += 2 * M_PI * (t.freq + tones[s] * spacing) / fs;
        if(i >= 0 && i < n) a[i] += amp * sin(phase);
      }
  }
  std::vector<int16_t> out(n);
  for(int i = 0 ; i < n ; i++)
  {
    long v = lround(dcOffset + a[i] + noiseCounts * gauss());
    out[i] = (int16_t)(v < 0 ? 0 : v > 4095 ? 4095 : v);
  }
  return(out);
}

static int run(Ft8Rx &rx, const std::vector<Tx> &txs, double noise, Ft8Rx::Msg *msgs)
{
  std::vector<int16_t> s = slot(txs, noise, 1900, rx.ft4());
  rx.reset();
  // Fed in small pieces, as the sampler hands them over
  for(size_t i = 0 ; i < s.size() ; i += 240) rx.push(&s[i], (int)std::min<size_t>(240, s.size() - i));
  return(rx.decode(msgs, 32));
}

static const Ft8Rx::Msg *find(const Ft8Rx::Msg *m, int n, const char *text)
{
  for(int i = 0 ; i < n ; i++) if(!strcmp(m[i].text, text)) return(&m[i]);
  return(nullptr);
}

int main()
{
  setvbuf(stdout, NULL, _IONBF, 0);
  srand(1);
  Ft8Rx rx;
  assert(rx.begin());
  assert(rx.bins() == 433);
  assert(rx.maxBlocks() == 93);

  Ft8Rx::Msg m[32];

  // Three stations at good strength
  std::vector<Tx> busy =
  {
    { "CQ EP2MHB LL58", 800, -5, 0.5f },
    { "A61AB K1ABC -12", 1500, 0, 0.6f },
    { "K1ABC A61AB R-08", 2200, -10, 0.4f },
  };
  int n = run(rx, busy, 40, m);
  for(int i = 0 ; i < n ; i++) printf("  %+3d dB %4u Hz  %s\n", m[i].snr, m[i].freq, m[i].text);
  for(const Tx &t : busy)
  {
    const Ft8Rx::Msg *f = find(m, n, t.text);
    assert(f);
    assert(abs((int)f->freq - (int)t.freq) <= 7);
    assert(abs(f->snr - (int)t.snr) <= 4);
  }
  assert(rx.blocks() == 93);
  assert(rx.level() > 20 && rx.level() < 60);

  // Weak signals: FT8 should go down to about -18 dB
  int weakOk = 0;
  for(int snr : { -12, -15, -18, -21 })
  {
    n = run(rx, { { "CQ DX EP2MHB LL58", 1200, (float)snr, 0.5f } }, 40, m);
    bool ok = find(m, n, "CQ DX EP2MHB LL58") != nullptr;
    printf("  %d dB: %s\n", snr, ok ? "decoded" : "missed");
    if(snr >= -15) assert(ok);
    weakOk += ok;
  }

  // Start times: dt follows where the signal really starts
  for(float st : { 0.0f, 0.5f, 0.73f, 1.3f, -0.4f, 2.0f })
  {
    n = run(rx, { { "CQ EP2MHB LL58", 1000, -6, st } }, 40, m);
    const Ft8Rx::Msg *f = find(m, n, "CQ EP2MHB LL58");
    assert(f);
    printf("  start %+.2f s: dt %+.2f s, edge correction %+d ms\n", st, f->dt, (int)Ft8Rx::edgeCorrectionMs(f, 1));
    assert(fabsf(f->dt - st) <= 0.1f);
    assert(abs(Ft8Rx::edgeCorrectionMs(f, 1) - (int)lroundf((st - 0.5f) * 1000)) <= 100);
  }
  {
    Ft8Rx::Msg three[3] = {};
    three[0].dt = 1.4f; three[1].dt = 1.5f; three[2].dt = 4.0f;   // one odd station
    assert(Ft8Rx::edgeCorrectionMs(three, 3) == 1000);
  }

  // Off the 6.25 Hz grid and off the half-symbol time grid
  n = run(rx, { { "CQ EP2MHB LL58", 1203.1f, -12, 0.73f } }, 40, m);
  assert(find(m, n, "CQ EP2MHB LL58"));

  // Started late by a second (a clock that is a little off)
  n = run(rx, { { "CQ EP2MHB LL58", 1000, -8, 1.5f } }, 40, m);
  assert(find(m, n, "CQ EP2MHB LL58"));

  // Noise only: nothing comes out
  n = run(rx, {}, 40, m);
  assert(n == 0);

  // A very quiet input still decodes: 3 counts of noise on the ADC
  n = run(rx, { { "CQ EP2MHB LL58", 1000, -5, 0.5f } }, 3, m);
  assert(find(m, n, "CQ EP2MHB LL58"));

  // FT4: 7.5 s slots, 105 symbols of 48 ms, 20.8 Hz tone spacing
  assert(rx.begin(true));
  assert(rx.ft4() && rx.slotMs() == 7500);
  printf("  FT4: %d bins, %d blocks\n", rx.bins(), rx.maxBlocks());
  std::vector<Tx> ft4busy =
  {
    { "CQ EP2MHB LL58", 900, -5, 0.5f },
    { "K1ABC A61AB R-08", 2000, 0, 0.6f },
  };
  n = run(rx, ft4busy, 40, m);
  for(int i = 0 ; i < n ; i++) printf("  FT4 %+3d dB %4u Hz  %s\n", m[i].snr, m[i].freq, m[i].text);
  for(const Tx &t : ft4busy)
  {
    const Ft8Rx::Msg *f = find(m, n, t.text);
    assert(f);
    assert(abs((int)f->freq - (int)t.freq) <= 21);
    assert(abs(f->snr - (int)t.snr) <= 4);
  }
  for(int snr : { -8, -11, -14, -17 })
  {
    n = run(rx, { { "CQ DX EP2MHB LL58", 1300, (float)snr, 0.5f } }, 40, m);
    bool ok = find(m, n, "CQ DX EP2MHB LL58") != nullptr;
    printf("  FT4 %d dB: %s\n", snr, ok ? "decoded" : "missed");
    if(snr >= -11) assert(ok);
  }
  n = run(rx, {}, 40, m);
  assert(n == 0);
  for(float st : { 0.5f, 0.9f, -0.2f })
  {
    n = run(rx, { { "CQ EP2MHB LL58", 1100, -4, st } }, 40, m);
    const Ft8Rx::Msg *f = find(m, n, "CQ EP2MHB LL58");
    assert(f);
    printf("  FT4 start %+.2f s: dt %+.2f s\n", st, f->dt);
    assert(fabsf(f->dt - st) <= 0.06f);
  }

  // No clock: the radio's timer is 7.3 s off the band. Stations call in
  // every slot 0.5 s after the real edge; the windows are cut on our timer
  // and Ft8Sync has to find the real edges.
  for(int proto = 0 ; proto < 2 ; proto++)
  {
    bool ft4 = proto == 1;
    assert(rx.begin(ft4));
    const int slotMs = rx.slotMs();
    const int32_t timerOff = ft4 ? 3100 : 7300;  // our time = band time + timerOff
    Ft8Sync sync;
    int64_t bandMs = 2000;                        // band time now
    int lockedAt = -1, decodedAfter = 0;
    for(int k = 0 ; k < 16 ; k++)
    {
      // Next window edge on our timer, in band time
      int64_t our = bandMs + timerOff + sync.offsetMs;
      int64_t edgeOur = (our / slotMs + 1) * slotMs;
      int64_t W = edgeOur - timerOff - sync.offsetMs;
      // Transmissions overlapping [W, W + slot)
      std::vector<Tx> txs;
      for(int64_t s = (W / slotMs - 1) * slotMs ; s < W + slotMs ; s += slotMs)
      {
        float st = (float)(s + 500 - W) / 1000.0f;
        txs.push_back({ "CQ EP2MHB LL58", 1000, -6, st });
        txs.push_back({ "A61AB K1ABC -12", 1800, -9, st + 0.1f });
      }
      n = run(rx, txs, 40, m);
      bool moved = sync.afterSlot<Ft8Rx::Msg, Ft8Rx::edgeCorrectionMs>(m, n, ft4);
      int err = (int)(((W % slotMs) + slotMs) % slotMs);
      if(err > slotMs / 2) err -= slotMs;
      printf("  %s no clock, slot %2d: window %+5d ms off the band edge, %d decoded%s%s\n", ft4 ? "FT4" : "FT8",
             k, err, n, moved ? ", moved" : "", sync.state == Ft8Sync::LOCKED ? ", locked" : "");
      if(sync.state == Ft8Sync::LOCKED && lockedAt < 0) lockedAt = k;
      if(lockedAt >= 0 && k > lockedAt) { assert(abs(err) <= 100); if(n >= 2) decodedAfter++; }
      bandMs = W + slotMs + (ft4 ? 300 : 1500);  // decoding takes a moment
    }
    printf("  %s locked after %d slots\n", ft4 ? "FT4" : "FT8", lockedAt + 1);
    assert(lockedAt >= 0 && lockedAt <= (ft4 ? 8 : 6));
    assert(decodedAfter >= 16 - lockedAt - 3);
  }

  // Back to FT8 works
  assert(rx.begin(false));
  assert(!rx.ft4() && rx.maxBlocks() == 93);
  n = run(rx, { { "CQ EP2MHB LL58", 1000, -5, 0.5f } }, 40, m);
  assert(find(m, n, "CQ EP2MHB LL58"));

  rx.end();
  printf("TestFt8: all tests passed\n");
  return(0);
}
