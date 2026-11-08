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

// One 15 s slot of ADC samples
static std::vector<int16_t> slot(const std::vector<Tx> &txs, double noiseCounts, int dcOffset)
{
  const int fs = Ft8Rx::FS;
  const int n = fs * 15;
  std::vector<double> a(n, 0.0);
  for(const Tx &t : txs)
  {
    ftx_message_t msg;
    ftx_message_init(&msg);
    assert(ftx_message_encode(&msg, nullptr, t.text) == FTX_MESSAGE_RC_OK);
    uint8_t tones[FT8_NN];
    ft8_encode(msg.payload, tones);
    // Noise power in 2500 Hz against the signal power A^2 / 2
    double n2500 = noiseCounts * noiseCounts * 2500.0 / (fs / 2.0);
    double amp = sqrt(2.0 * n2500 * pow(10.0, t.snr / 10.0));
    double phase = 0;
    int sps = fs * 0.16;
    int first = (int)(t.start * fs);
    for(int s = 0 ; s < FT8_NN ; s++)
      for(int k = 0 ; k < sps ; k++)
      {
        int i = first + s * sps + k;
        phase += 2 * M_PI * (t.freq + tones[s] * 6.25) / fs;
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
  std::vector<int16_t> s = slot(txs, noise, 1900);
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

  rx.end();
  printf("TestFt8: all tests passed\n");
  return(0);
}
