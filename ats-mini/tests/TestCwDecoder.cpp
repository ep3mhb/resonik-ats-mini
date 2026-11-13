#include "../CwDecoder.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <vector>

// Morse for the test text
static const char *morse(char c)
{
  static const char *const l[] = { ".-","-...","-.-.","-..",".","..-.","--.","....","..",".---","-.-",".-..","--",
    "-.","---",".--.","--.-",".-.","...","-","..-","...-",".--","-..-","-.--","--.." };
  static const char *const d[] = { "-----",".----","..---","...--","....-",".....","-....","--...","---..","----." };
  if(c >= 'A' && c <= 'Z') return l[c - 'A'];
  if(c >= '0' && c <= '9') return d[c - '0'];
  if(c == '?') return "..--..";
  if(c == '/') return "-..-.";
  return "";
}

static unsigned seed = 1;
static float noise()
{
  // Sum of uniforms: roughly Gaussian, unit variance
  float s = 0;
  for(int i = 0 ; i < 12 ; i++) { seed = seed * 1103515245u + 12345u; s += (seed >> 8 & 0xFFFF) / 65536.0f; }
  return s - 6.0f;
}

// Keyed tone at the given speed, as 12-bit ADC samples around mid scale
static void key(std::vector<int16_t> &out, const char *text, float wpm, float tone, float amp, float noiseAmp, float fs = 8000)
{
  float unit = 1.2f / wpm;
  float ph = 0;
  auto emit = [&](bool on, float units) {
    int n = (int)(units * unit * fs);
    for(int i = 0 ; i < n ; i++)
    {
      ph += 2 * (float)M_PI * tone / fs;
      float v = 2048 + (on ? amp * sinf(ph) : 0) + noiseAmp * noise();
      out.push_back((int16_t)v);
    }
  };
  emit(false, 10);
  for(const char *p = text ; *p ; p++)
  {
    if(*p == ' ') { emit(false, 4); continue; }   // 3 already after the letter, 7 in all
    for(const char *m = morse(*p) ; *m ; m++)
    {
      emit(true, *m == '-' ? 3 : 1);
      emit(false, 1);
    }
    emit(false, 2);
  }
  emit(false, 20);
}

static std::string decode(const std::vector<int16_t> &s, float tone = 700, float fs = 8000)
{
  CwDecoder d;
  d.begin(fs, tone);
  // Feed in chunks like the ADC task would
  for(size_t i = 0 ; i < s.size() ; i += 64)
    d.push(&s[i], (int)(s.size() - i < 64 ? s.size() - i : 64));
  return d.getText();
}

static bool endsWith(const std::string &s, const char *tail)
{
  std::string t(tail);
  std::string a = s;
  while(!a.empty() && a.back() == ' ') a.pop_back();
  return a.size() >= t.size() && a.compare(a.size() - t.size(), t.size(), t) == 0;
}

static void check(const char *name, const char *text, float wpm, float tone, float amp, float noiseAmp, float decTone = 700)
{
  std::vector<int16_t> s;
  key(s, text, wpm, tone, amp, noiseAmp);
  std::string got = decode(s, decTone);
  printf("  %-28s %5.1f WPM  -> \"%s\"\n", name, wpm, got.c_str());
  // The last call sign must come out exactly; a lead-in lets the speed settle
  const char *tail = strrchr(text, ' ') ? strrchr(text, ' ') + 1 : text;
  std::string want = std::string("DE EP2MHB ") + tail;
  assert(endsWith(got, want.c_str()));
}

int main()
{
  setvbuf(stdout, NULL, _IONBF, 0);
  const char *msg = "VVV CQ CQ DE EP2MHB K";
  check("clean, 20 WPM", msg, 20, 700, 1500, 20);
  check("clean, 12 WPM", msg, 12, 700, 1500, 20);
  check("clean, 30 WPM", msg, 30, 700, 1500, 20);
  check("noisy, 18 WPM", msg, 18, 700, 600, 250);
  check("off pitch by 40 Hz", msg, 20, 740, 1500, 50);
  check("weak, 25 WPM", msg, 25, 700, 300, 80);

  // Speed change mid stream: the decoder follows
  {
    std::vector<int16_t> s;
    key(s, "VVV CQ DE EP2MHB K", 15, 700, 1500, 30);
    key(s, "VVV TEST DE EP2MHB K", 28, 700, 1500, 30);
    std::string got = decode(s);
    printf("  %-28s 15->28 WPM -> \"%s\"\n", "speed change", got.c_str());
    assert(endsWith(got, "DE EP2MHB K"));
  }

  // Silence and noise alone must not produce text
  {
    std::vector<int16_t> s(8000 * 5, 2048);
    for(auto &v : s) v = (int16_t)(2048 + 300 * noise());
    std::string got = decode(s);
    printf("  %-28s           -> \"%s\"\n", "noise only", got.c_str());
    assert(got.size() <= 2);
  }

  // The radio samples at 4 kHz; fast senders too
  {
    std::vector<int16_t> s;
    key(s, msg, 20, 700, 1200, 60, 4000);
    std::string got = decode(s, 700, 4000);
    printf("  %-28s  20 WPM   -> \"%s\"\n", "4 kHz sampling", got.c_str());
    assert(endsWith(got, "DE EP2MHB K"));
    s.clear();
    key(s, "VVV VVV CQ DE EP2MHB K", 40, 700, 1200, 40, 4000);
    got = decode(s, 700, 4000);
    printf("  %-28s  40 WPM   -> \"%s\"\n", "4 kHz, fast", got.c_str());
    assert(endsWith(got, "DE EP2MHB K"));
  }

  // Receiver noise with static crashes (through a 2.5 kHz filter): before
  // the fix this printed a row of E's, about one every two seconds
  {
    std::vector<int16_t> s;
    float r = expf(-(float)M_PI * 2500 / 4000), c = 2 * r * cosf(2 * (float)M_PI * 700 / 4000), y1 = 0, y2 = 0;
    int crash = 0;
    float camp = 0;
    srand(3);
    for(int i = 0 ; i < 4000 * 60 ; i++)
    {
      float x = noise();
      if(crash > 0) { x += camp * noise(); crash--; }
      else if(rand() % 2000 == 0) { crash = 20 + rand() % 120; camp = 8 + rand() % 20; }
      float y = x + c * y1 - r * r * y2;
      y2 = y1;
      y1 = y;
      s.push_back((int16_t)(2048 + 60 * y * (1 - r)));
    }
    std::string got = decode(s, 700, 4000);
    int n = 0;
    for(char ch : got) if(ch != ' ') n++;
    printf("  %-28s  60 s     -> %d characters\n", "noise with static crashes", n);
    assert(n <= 3);
  }

  printf("TestCwDecoder: all tests passed\n");
  return 0;
}
