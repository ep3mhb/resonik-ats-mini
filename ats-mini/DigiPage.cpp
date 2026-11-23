//
// RESONIK R5: digital modes page (Menu -> Ham & SWL -> Digital): RTTY
// (amateur 45.45 baud / 170 Hz and DWD weather 50 baud / 450 Hz), NAVTEX
// (518 kHz) and PSK31, from the speaker line on GPIO11 like the FT8 page.
//
// The sampler (CwAudio.cpp) writes 6 kHz audio into this page's ring; a
// worker on core 1 feeds the decoder and makes a
// spectrum, on which the signal is found by itself: the strongest pair of
// tones the mode's shift apart (RTTY, NAVTEX) or the strongest carrier
// (PSK31). The knob tunes the radio, a click changes the mode, a short
// press leaves. The radio goes to USB 3.0k on opening and back on leaving.
//
// The whole text is kept (Digi::Log, 16 kB in PSRAM, line breaks as sent):
// push and turn reads it back on the screen, and the browser asks for what
// it has not got yet ("!dtext"). NAVTEX is channelled, as FT8 is: there the
// knob steps through its channels (518, 490 and 4209.5 kHz and the eight
// short-wave channels for safety information on the high seas) and the
// decoder finds the tones by itself, so there is nothing to tune.
//
#include "Common.h"
#include "Diag.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"
#include "CwAudio.h"
#include "Ft8.h"
#include "Resonik.h"
#include "DigiPage.h"
#include "DigiDecoders.h"
#include "src/ft8/kiss_fftr.h"

#include <time.h>
#include <esp_heap_caps.h>
#include <freertos/semphr.h>
#include <new>

#define FS           6000             // The decoders always run at 6 kHz
#define RING         (12000 * 4)      // Room for 4 s at the higher input rate
// Input rate from the Decoder Audio setting: 6000 (3 kHz of audio, used as
// it is) or 12000 (6 kHz of audio, two samples averaged into one)
static volatile int inRate = FS;
#define NFFT         1024
#define LINE_VOLUME  50
#define SPEC_LO      300              // Spectrum shown and searched, Hz
#define SPEC_HI      2700
#define SPEC_W       296
#define LOG_SIZE     16384            // The whole text: characters kept
#define LINES_MAX    2048             // Lines of it the read-back view indexes
#define READ_ROWS    7                // Lines on the screen while reading back

// NAVTEX channels and where the radio listens for them (USB, tones at
// 1500 Hz): the three NAVTEX channels, which the browser's timetable knows
// (tools/navtex_table.py goes by this order), then the short-wave channels
// for maritime safety information on the high seas, same kind of signal
static const struct { const char *name; uint32_t dial; } navCh[] =
{
  { "518 kHz",     516500 },
  { "490 kHz",     488500 },
  { "4209.5 kHz",  4208000 },
  { "4210 kHz",    4208500 },
  { "6314 kHz",    6312500 },
  { "8416.5 kHz",  8415000 },
  { "12579 kHz",   12577500 },
  { "16806.5 kHz", 16805000 },
  { "19680.5 kHz", 19679000 },
  { "22376 kHz",   22374500 },
  { "26100.5 kHz", 26099000 },
};
#define NAV_COUNT    ((int)(sizeof(navCh) / sizeof(navCh[0])))
static int nav = 0;

enum DigiMode { DM_RTTY, DM_DWD, DM_NAVTEX, DM_PSK31, DM_WEFAX, DM_COUNT };
static const char *const modeName[DM_COUNT] = { "RTTY", "RTTY DWD", "NAVTEX", "PSK31", "WEFAX" };
static const char *const modeInfo[DM_COUNT] = { "45.45 Bd 170 Hz", "50 Bd 450 Hz", "100 Bd 170 Hz", "31.25 Bd BPSK", "120 lpm, dial -1.9 kHz" };

static int16_t *ring = nullptr;
static SemaphoreHandle_t lock = nullptr;
static TaskHandle_t worker = nullptr;
static volatile bool running = false;
static volatile uint32_t generation = 0;
static bool pageActive = false;
static int mode = DM_RTTY;
static volatile int workMode = DM_RTTY;
static uint32_t cpuSaved = 0;
static bool ampNeeded = false;
static uint32_t openedAt = 0;

// Shared with drawing (under lock)
static Digi::Rtty *rtty = nullptr;
static Digi::Navtex *navtex = nullptr;
static Digi::Psk31 *psk = nullptr;
static Digi::Wefax *fax = nullptr;
static uint32_t faxSent = 0;          // Lines sent to the browser
static char shownText[Digi::TEXT_MAX + 1];
static uint8_t spec[SPEC_W];          // Spectrum, 0..255
static volatile float centre = 1500;  // Signal found, Hz
static volatile bool found = false;
static volatile uint32_t changes = 0;

// The whole text (under lock) and the read-back view of it: a copy taken
// when reading begins, with the offsets where its screen lines start
static Digi::Log textLog;
static char *snap = nullptr;
static uint32_t *lineAt = nullptr;
static uint32_t snapLen = 0;
static int lineCount = 0, topLine = 0, turned = 0;
static bool reading = false, reindex = false;

// Head line of the text that follows: mode, frequency, UTC time if known
static void logHead()
{
  static const char *const mon[12] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
  char when[28] = "";
  if(clockAvailable())
  {
    time_t now = time(NULL);
    struct tm t;
    gmtime_r(&now, &t);
    snprintf(when, sizeof(when), ", %d %s %02d:%02d UTC", t.tm_mday, mon[t.tm_mon % 12], t.tm_hour, t.tm_min);
  }
  if(workMode == DM_NAVTEX)
    snprintf(textLog.head, sizeof(textLog.head), "== NAVTEX %s%s ==", navCh[nav].name, when);
  else
    snprintf(textLog.head, sizeof(textLog.head), "== %s %lu.%lu kHz USB%s ==", modeName[workMode],
             (unsigned long)(currentFrequency / 1000), (unsigned long)(currentFrequency % 1000 / 100), when);
}

static Digi::Text *activeText(int m)
{
  switch(m)
  {
    case DM_RTTY: case DM_DWD: return(&rtty->text);
    case DM_NAVTEX: return(&navtex->text);
    default: return(&psk->text);   // (WEFAX has no text: PSK31's is cleared)
  }
}

static void beginDecoder(int m, float c)
{
  switch(m)
  {
    case DM_RTTY:   rtty->begin(FS, 45.45f, 170, c); break;
    case DM_DWD:    rtty->begin(FS, 50, 450, c); break;
    case DM_NAVTEX: navtex->begin(FS, c); break;
    case DM_WEFAX:  fax->begin(FS); psk->begin(FS, c); faxSent = 0; break;
    default:        psk->begin(FS, c); break;
  }
}

static void setCentre(int m, float c)
{
  switch(m)
  {
    case DM_RTTY: case DM_DWD: rtty->setCentre(c); break;
    case DM_NAVTEX: navtex->setCentre(c); break;
    default: psk->setCarrier(c); break;
  }
}

static void work(void *)
{
  static int16_t buf[512];
  static float fftIn[NFFT];
  static kiss_fft_cpx fftOut[NFFT / 2 + 1];
  static float power[NFFT / 2 + 1];
  static int16_t hist[NFFT];
  size_t memNeeded = 0;
  kiss_fftr_alloc(NFFT, 0, nullptr, &memNeeded);
  void *mem = heap_caps_malloc(memNeeded, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  kiss_fftr_cfg cfg = mem ? kiss_fftr_alloc(NFFT, 0, mem, &memNeeded) : nullptr;
  int histPos = 0, sinceFft = 0;
  uint32_t pos = 0, gen = 0;
  uint32_t step = 1;                 // Input samples per decoder sample
  float window[NFFT];
  for(int i = 0; i < NFFT; i++) window[i] = 0.5f - 0.5f * cosf(2 * (float)M_PI * i / (NFFT - 1));
  float avgPower[NFFT / 2 + 1] = { 0 };
  float lastCentre = 0;
  int wm = DM_RTTY;                  // The mode this task decodes (R5)
  // R5: RTTY and PSK31 read characters out of plain noise. The decoder gets
  // the sound only while the two tones (or the carrier) stand out of the
  // band, and for a second after, so that a fade does not cut a word
  // (Mohammad, 21 Nov: "RTTY, no signal" above lines of random letters).
  int quiet = 1000;                  // Spectra in a row without a signal

  for(;;)
  {
    if(!running)
    {
      ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(200));
      gen = generation - 1;
      continue;
    }
    uint32_t written = audioFt8Written();
    if(gen != generation)
    {
      gen = generation;
      // R5: the worker's own copy of the mode, taken with the generation: the
      // page sets workMode a moment before it counts a new generation, and
      // sound must never reach a decoder that has not been begun
      wm = workMode;
      pos = written;
      step = inRate >= 2 * FS ? 2 : 1;
      xSemaphoreTake(lock, portMAX_DELAY);
      beginDecoder(wm, centre);
      shownText[0] = '\0';
      logHead();
      xSemaphoreGive(lock);
      memset(avgPower, 0, sizeof(avgPower));
      found = false;
      lastCentre = 0;
      quiet = 1000;
      diagStep(20 + wm);
    }
    if(written - pos > RING - 1024) pos = written - 1024;   // Fell behind: skip

    while(written - pos >= step)
    {
      int n = 0;
      while(n < 512 && written - pos >= step)
      {
        int16_t s = ring[pos % RING];
        if(step == 2) s = (int16_t)(((int32_t)s + ring[(pos + 1) % RING]) / 2);
        pos += step;
        buf[n++] = s;
        hist[histPos] = s;
        histPos = (histPos + 1) % NFFT;
      }
      sinceFft += n;
      bool open = quiet < 6;         // Six spectra: about a second
      xSemaphoreTake(lock, portMAX_DELAY);
      switch(wm)
      {
        case DM_RTTY: case DM_DWD: if(open) rtty->push(buf, n); break;
        case DM_NAVTEX: navtex->push(buf, n); break;
        case DM_WEFAX: { uint32_t was = fax->total(); fax->push(buf, n); if(fax->total() != was) changes++; break; }
        default: if(open) psk->push(buf, n); break;
      }
      Digi::Text *t = activeText(wm);
      if(strcmp(shownText, t->buf)) { strcpy(shownText, t->buf); changes++; }
      xSemaphoreGive(lock);

      // Spectrum about six times a second
      if(sinceFft >= 1024 && cfg)
      {
        sinceFft = 0;
        float dc = 0;
        for(int i = 0; i < NFFT; i++) dc += hist[(histPos + i) % NFFT];
        dc /= NFFT;
        for(int i = 0; i < NFFT; i++) fftIn[i] = (hist[(histPos + i) % NFFT] - dc) * window[i];
        kiss_fftr(cfg, fftIn, fftOut);
        for(int k = 0; k <= NFFT / 2; k++)
        {
          power[k] = fftOut[k].r * fftOut[k].r + fftOut[k].i * fftOut[k].i;
          avgPower[k] = avgPower[k] * 0.7f + power[k] * 0.3f;
        }
        // Screen strip: log power, scaled to the band's own floor and peak
        float lo = 1e30f, hi = 0;
        const float hzPerBin = (float)FS / NFFT;
        for(int x = 0; x < SPEC_W; x++)
        {
          int k = (int)((SPEC_LO + (SPEC_HI - SPEC_LO) * x / (float)SPEC_W) / hzPerBin);
          float v = log10f(avgPower[k] + 1);
          spec[x] = 0;
          if(v < lo) lo = v;
          if(v > hi) hi = v;
          fftIn[x] = v;
        }
        for(int x = 0; x < SPEC_W; x++)
          spec[x] = hi > lo ? (uint8_t)constrain((fftIn[x] - lo) * 255 / (hi - lo), 0, 255) : 0;

        // Find the signal
        int m = wm;
        float bestScore = 0, bestHz = 0, floorP = 0;
        int k0 = (int)(SPEC_LO / hzPerBin), k1 = (int)(SPEC_HI / hzPerBin);
        for(int k = k0; k <= k1; k++) floorP += avgPower[k];
        floorP /= (k1 - k0 + 1);
        if(m == DM_WEFAX)
        {
          bestScore = 0;   // Fixed at 1900 Hz: no search
        }
        else if(m == DM_PSK31)
        {
          for(int k = k0; k <= k1; k++) if(avgPower[k] > bestScore) { bestScore = avgPower[k]; bestHz = k * hzPerBin; }
        }
        else
        {
          float shift = m == DM_DWD ? 450 : 170;
          int half = (int)(shift / 2 / hzPerBin + 0.5f);
          for(int k = k0 + half; k <= k1 - half; k++)
          {
            // Both tones must be there: the weaker of the two counts
            float a = avgPower[k - half] + avgPower[k - half + 1], b = avgPower[k + half] + avgPower[k + half - 1];
            float sc = a < b ? a : b;
            if(sc > bestScore) { bestScore = sc; bestHz = k * hzPerBin; }
          }
        }
        bool sig = m == DM_WEFAX ? true : bestScore > floorP * 12;
        if(m == DM_WEFAX) bestHz = 1900;
        found = sig;
        if(sig) quiet = 0;
        else if(quiet < 1000) quiet++;
        // Follow the signal; PSK31 tracks its own carrier once it reads
        if(sig && fabsf(bestHz - lastCentre) > (m == DM_PSK31 ? 4.0f : 12.0f) &&
           !(m == DM_PSK31 && psk->quality > 0.8f && fabsf(psk->carrier() - bestHz) < 15))
        {
          lastCentre = bestHz;
          centre = bestHz;
          xSemaphoreTake(lock, portMAX_DELAY);
          setCentre(m, bestHz);
          xSemaphoreGive(lock);
        }
        if(m == DM_PSK31) centre = psk->carrier();
        changes++;
      }
    }
    vTaskDelay(pdMS_TO_TICKS(30));
  }
}

static void audioOn()
{
  if(PIN_AMP_EN >= 0) digitalWrite(PIN_AMP_EN, ampNeeded ? HIGH : LOW);
  if(audioClamp) digitalWrite(AUDIO_MUTE, LOW);
  rx.setAudioMute(false);
  rx.setVolume(LINE_VOLUME);
}

static void audioOff()
{
  rx.setVolume(volume);
  bool muted = muteOn(MUTE_MAIN) || muteOn(MUTE_SQUELCH);
  if(muted)
  {
    if(audioClamp) digitalWrite(AUDIO_MUTE, HIGH);
    rx.setAudioMute(true);
  }
  if(PIN_AMP_EN >= 0 && !(muted && !audioClamp)) digitalWrite(PIN_AMP_EN, HIGH);
}

// RESONIK R5: each mode keeps its own frequency while the radio is on, so a
// click from NAVTEX (516.5 kHz) to RTTY goes straight to the RTTY band instead
// of leaving the radio on medium wave, and the page never works on FM, where
// these signals do not exist (Mohammad, 22 Nov). Start values: where each mode
// is heard most often (chapter 26 of the manual).
static uint32_t modeFreq[DM_COUNT] = { 14080000, 10099300, 0, 14070000, 8038100 };

// The browser started the page on a frequency of its list (digiWebOpen)
static bool openAtModeFreq = false;

static bool usableFreq(uint32_t hz) { return(currentMode != FM && hz >= 100000 && hz <= 30000000); }

// keep: on opening the page, stay where the radio is if it can work there
static void tuneForMode(bool keep = false)
{
  if(mode == DM_NAVTEX) { digiSetUsb(navCh[nav].dial); return; }
  if(keep && usableFreq(currentFrequency)) { digiSetUsb(0); modeFreq[mode] = currentFrequency; return; }
  digiSetUsb(modeFreq[mode]);
}

// The frequency the mode was left on, for the next time
static void keepModeFreq()
{
  if(mode != DM_NAVTEX && usableFreq(currentFrequency)) modeFreq[mode] = currentFrequency;
}

static bool start()
{
  diagStep(4);
  if(!lock) lock = xSemaphoreCreateMutex();
  if(!ring) ring = (int16_t *)heap_caps_malloc(RING * sizeof(int16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if(!fax) fax = new (heap_caps_malloc(sizeof(Digi::Wefax), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)) Digi::Wefax();
  if(!rtty) rtty = new (heap_caps_malloc(sizeof(Digi::Rtty), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)) Digi::Rtty();
  if(!navtex) navtex = new (heap_caps_malloc(sizeof(Digi::Navtex), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)) Digi::Navtex();
  if(!psk) psk = new (heap_caps_malloc(sizeof(Digi::Psk31), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)) Digi::Psk31();
  if(!textLog.buf)
  {
    textLog.buf = (char *)heap_caps_malloc(LOG_SIZE, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    textLog.size = LOG_SIZE;
  }
  if(!snap) snap = (char *)heap_caps_malloc(LOG_SIZE + 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if(!lineAt) lineAt = (uint32_t *)heap_caps_malloc((LINES_MAX + 1) * sizeof(uint32_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if(!ring || !rtty || !navtex || !psk || !fax || !textLog.buf || !snap || !lineAt)
  {
    statusShow("Digital", "Not enough memory");
    return(false);
  }
  rtty->text.log = navtex->text.log = psk->text.log = &textLog;
  reading = false;
  if(!worker) xTaskCreatePinnedToCore(work, "digi", 16384, nullptr, 1, &worker, 1);
  ft8SaveRadio();
  tuneForMode(!openAtModeFreq);
  openAtModeFreq = false;
  audioOn();
  cpuSaved = getCpuFrequencyMhz();
  if(cpuSaved < 240) setCpuFrequencyMhz(240);
  workMode = mode;
  centre = mode == DM_PSK31 ? 1000 : mode == DM_WEFAX ? 1900 : 1500;
  inRate = decoderSampleRate();
  audioFt8Ring(ring, RING, inRate);
  openedAt = millis();
  generation++;
  running = true;
  xTaskNotifyGive(worker);
  return(true);
}

static void stop()
{
  diagStep(5);
  keepModeFreq();
  running = false;
  reading = false;
  audioFt8Ring(nullptr, 0);
  if(cpuSaved && cpuSaved < 240) setCpuFrequencyMhz(cpuSaved);
  cpuSaved = 0;
  ft8RestoreRadio();
  audioOff();
}

void digiPageOpen() { currentCmd = CMD_DIGI; }

// RESONIK R5: mode, signal, text and spectrum for the browser
bool digiWebState(uint32_t *seen, String &j)
{
  if(!pageActive || !lock || *seen == changes) return(false);
  *seen = changes;
  j = "{\"dg\":{\"m\":\"";
  j += modeName[mode];
  if(mode == DM_NAVTEX) { j += ' '; j += navCh[nav].name; }
  else
  {
    // The frequency the radio is tuned to, as in the page's header
    char f[24];
    snprintf(f, sizeof(f), " %lu.%02lu kHz", (unsigned long)(currentFrequency / 1000), (unsigned long)(currentFrequency % 1000 / 10));
    j += f;
  }
  j += "\",\"n\":"; j += textLog.total;
  if(mode == DM_NAVTEX && navtex && navtex->synced) j += ",\"s\":1";
  j += ",\"c\":"; j += found ? (int)centre : 0;
  j += ",\"k\":"; j += inRate / 2000;
  j += ",\"t\":\"";
  xSemaphoreTake(lock, portMAX_DELAY);
  for(const char *c = shownText; *c; c++) if(*c >= 32 && *c != '"' && *c != '\\') j += *c;
  xSemaphoreGive(lock);
  j += "\"},\"sp\":{\"lo\":";
  j += SPEC_LO; j += ",\"hi\":"; j += SPEC_HI; j += ",\"v\":[";
  for(int x = 0; x < 74; x++) { if(x) j += ','; j += spec[x * SPEC_W / 74]; }
  j += "]}";
  // WEFAX: the lines not yet sent, 150 pixels each, at most 4 at a time
  if(mode == DM_WEFAX && fax)
  {
    uint32_t total = fax->total();
    if(total - faxSent > 4) faxSent = total - 4;
    if(total != faxSent)
    {
      j += ",\"fx\":[";
      bool first = true;
      xSemaphoreTake(lock, portMAX_DELAY);
      for(; faxSent < total; faxSent++)
      {
        const uint8_t *r = fax->row(total - 1 - faxSent);
        if(!first) j += ',';
        first = false;
        j += '[';
        for(int x = 0; x < 150; x++) { if(x) j += ','; j += r[x * 2]; }
        j += ']';
      }
      xSemaphoreGive(lock);
      j += ']';
    }
  }
  j += "}";
  return(true);
}

// RESONIK R5: the whole text from character `from` on, at most 1024 of them,
// for the browser: {"dgt":{"o":first sent,"e":one past the last,"n":total,"t":".."}}
void digiWebText(uint32_t from, String &j)
{
  static char part[1025];
  uint32_t total = 0, o = 0;
  int n = 0;
  if(lock && textLog.buf)
  {
    xSemaphoreTake(lock, portMAX_DELAY);
    total = textLog.total;
    o = from > total || from < textLog.first() ? textLog.first() : from;
    while(n < 1024 && o + n < total) { part[n] = textLog.at(o + n); n++; }
    xSemaphoreGive(lock);
  }
  j = "{\"dgt\":{\"o\":"; j += o;
  j += ",\"e\":"; j += o + n;
  j += ",\"n\":"; j += total;
  j += ",\"t\":\"";
  for(int i = 0; i < n; i++)
  {
    char c = part[i];
    if(c == '\n') j += "\\n";
    else if(c == '"' || c == '\\') { j += '\\'; j += c; }
    else j += c;
  }
  j += "\"}}";
}

// The open page goes to another mode (or NAVTEX to another channel)
static void goMode(int m, uint32_t hz = 0);

// Push and turn begins reading the text back; the knob then scrolls it.
// In NAVTEX the knob steps through the channels. False: the knob tunes.
bool digiTurn(int16_t enc, bool pressed)
{
  if(!pageActive) return(false);
  if(!reading)
  {
    if(!pressed)
    {
      if(mode != DM_NAVTEX) return(false);
      nav = (nav + (enc > 0 ? 1 : NAV_COUNT - 1)) % NAV_COUNT;
      goMode(mode);
      return(true);
    }
    if(mode == DM_WEFAX) return(false);
    reading = true;
    reindex = true;                  // The view is built when it is next drawn
    turned = enc;
    return(true);
  }
  turned += enc;
  return(true);
}

// The open page goes to another mode
static void goMode(int m, uint32_t hz)
{
  diagStep(1);
  if(pageActive) keepModeFreq();
  mode = m;
  if(hz && m != DM_NAVTEX) modeFreq[m] = hz;
  if(!pageActive) return;
  tuneForMode();
  diagStep(2);
  audioOn();
  workMode = mode;
  centre = mode == DM_PSK31 ? 1000 : mode == DM_WEFAX ? 1900 : 1500;
  generation++;
  diagStep(3);
}

void digiClick(bool shortPress)
{
  if(shortPress) { reading = false; currentCmd = CMD_NONE; return; }
  if(reading) { reading = false; return; }           // Back to the live text
  goMode((mode + 1) % DM_COUNT);
}

// RESONIK R5: Web Control starts the page in a mode, or changes mode.
// a: the mode, plus 256 times the NAVTEX channel, or for the other modes
// 256 times a frequency in tens of hertz (0: where the radio is)
void digiWebOpen(int a)
{
  int hi = a >> 8, m = a & 0xFF;
  if(m < 0 || m >= DM_COUNT || hi < 0) return;
  uint32_t hz = 0;
  if(m == DM_NAVTEX) { if(hi >= NAV_COUNT) return; }
  else if(hi)
  {
    hz = (uint32_t)hi * 10;
    if(hz < 100000 || hz > 30000000) return;
  }
  bool same = m == mode && (m != DM_NAVTEX || hi == nav);
  if(m == DM_NAVTEX) nav = hi;
  if(pageActive) { if(!same || hz) goMode(m, hz); return; }
  mode = m;
  if(hz) { modeFreq[m] = hz; openAtModeFreq = true; }
  currentCmd = CMD_DIGI;
}

// RESONIK R5: a frequency typed in Web Control while the page is open. The
// page stays in USB with its filter, as when the knob tunes; the Receiver
// tab's own way changed band and could change the mode (AM, LSB). NAVTEX
// has channels, not a frequency. True: taken here.
bool digiWebTune(uint32_t hz)
{
  if(currentCmd != CMD_DIGI) return(false);
  if(mode == DM_NAVTEX)
  {
    statusShow("NAVTEX", "Choose a channel, not a frequency");
    return(true);
  }
  if(hz < 100000 || hz > 30000000)
  {
    statusShow("Digital", "100 kHz to 30 MHz only");
    return(true);
  }
  if(!pageActive) { modeFreq[mode] = hz; openAtModeFreq = true; return(true); }
  digiSetUsb(hz);
  keepModeFreq();
  generation++;
  return(true);
}

// -1 while the page is closed; else the mode, plus 256 times the NAVTEX channel
int digiWebMode() { return(!pageActive ? -1 : mode == DM_NAVTEX ? mode | (nav << 8) : mode); }
int digiNavCount() { return(NAV_COUNT); }
const char *digiNavName(int i) { return(i >= 0 && i < NAV_COUNT ? navCh[i].name : ""); }
int digiModeCount() { return(DM_COUNT); }
const char *digiModeName(int m) { return(m >= 0 && m < DM_COUNT ? modeName[m] : ""); }

bool digiTick()
{
  bool want = currentCmd == CMD_DIGI && !sleepOn();
  if(want && !pageActive)
  {
    pageActive = start();
    if(!pageActive) currentCmd = CMD_NONE;
    return(true);
  }
  if(!want && pageActive)
  {
    pageActive = false;
    stop();
    return(true);
  }
  if(!pageActive) return(false);
  // Never on FM (a frequency set from Web Control, say): back to the mode's own
  if(currentMode == FM)
  {
    tuneForMode();
    statusShow("Digital", "Not on FM: back to short wave");
    return(true);
  }
  // Decoder Audio changed (from Web Control) while the page is open
  if(inRate != decoderSampleRate())
  {
    inRate = decoderSampleRate();
    audioFt8Ring(ring, RING, inRate);
    generation++;
  }
  // Silent input with the amplifier off: this board taps after it (as FT8)
  if(!ampNeeded && PIN_AMP_EN >= 0 && millis() - openedAt > 1500 && audioInputLevel() < 2)
  {
    ampNeeded = true;
    audioOn();
  }
  static uint32_t seen = 0, lastDraw = 0;
  if(changes != seen && millis() - lastDraw >= 150)
  {
    seen = changes;
    lastDraw = millis();
    return(true);
  }
  return(false);
}

#define DG_RGB(c)  ((uint16_t)((((c) >> 8) & 0xF800) | (((c) >> 5) & 0x07E0) | (((c) >> 3) & 0x001F)))

// Read-back: copy the text and find where its screen lines begin. A line
// ends at its line break, or after the last space that fits in maxW pixels.
static void readIndex(int maxW)
{
  static uint8_t cw[95];
  static bool measured = false;
  if(!measured)
  {
    char one[2] = " ";
    for(int c = 32; c < 127; c++) { one[0] = (char)c; cw[c - 32] = (uint8_t)spr.textWidth(one, FONT_SMALL); }
    measured = true;
  }
  xSemaphoreTake(lock, portMAX_DELAY);
  uint32_t first = textLog.first();
  snapLen = textLog.total - first;
  for(uint32_t i = 0; i < snapLen; i++) snap[i] = textLog.at(first + i);
  xSemaphoreGive(lock);
  snap[snapLen] = '\0';
  lineCount = 0;
  for(uint32_t i = 0; i < snapLen; )
  {
    if(lineCount >= LINES_MAX)     // More lines than the index holds: keep the newer half
    {
      memmove(lineAt, lineAt + LINES_MAX / 2, (LINES_MAX / 2) * sizeof(uint32_t));
      lineCount -= LINES_MAX / 2;
    }
    lineAt[lineCount++] = i;
    uint32_t j = i, space = 0;
    int w = 0;
    while(j < snapLen && snap[j] != '\n')
    {
      int c = cw[(uint8_t)snap[j] - 32];
      if(w + c > maxW) break;
      if(snap[j] == ' ') space = j;
      w += c;
      j++;
    }
    if(j < snapLen && snap[j] == '\n') j++;            // The break belongs to this line
    else if(j < snapLen && space > i) j = space + 1;   // Too long: wrap after the last space
    if(j == i) j++;
    i = j;
  }
  lineAt[lineCount] = snapLen;
  topLine = lineCount > READ_ROWS ? lineCount - READ_ROWS : 0;
}

void drawDigiPage()
{
  char line[Digi::TEXT_MAX + 1];
  spr.fillSprite(DG_RGB(0x041F29));
  if(reading && reindex) { reindex = false; readIndex(300); }
  int maxTop = lineCount > READ_ROWS ? lineCount - READ_ROWS : 0;
  if(reading) { topLine = constrain(topLine + turned, 0, maxTop); turned = 0; }

  // Header: mode, the frequency the radio is tuned to, the signal's audio
  // frequency. The speed and shift stood here; the page did not say where the
  // radio was tuned (Mohammad, 22 Nov), so they moved into the spectrum.
  spr.fillRoundRect(4, 3, 312, 22, 6, DG_RGB(0x073A4B));
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(DG_RGB(0xFFD167));
  spr.drawString(modeName[mode], 10, 14, uiFont(false));
  spr.setTextColor(DG_RGB(0x8FB3C0));
  // NAVTEX: its channel, which the page tunes 1.5 kHz lower by itself, and USB
  // as the other modes (23 Nov: "on NAVTEX showing freq only without mode")
  if(mode == DM_NAVTEX) snprintf(line, sizeof(line), reading ? "%s" : "%s USB", navCh[nav].name);
  else snprintf(line, sizeof(line), reading ? "%lu.%02lu kHz" : "%lu.%02lu kHz USB",
                (unsigned long)(currentFrequency / 1000), (unsigned long)(currentFrequency % 1000 / 10));
  spr.drawString(line, 10 + spr.textWidth(modeName[mode], uiFont(false)) + 8, 14, uiFont(true));
  spr.setTextDatum(MR_DATUM);
  bool sync = mode == DM_NAVTEX && navtex && navtex->synced;
  if(reading) snprintf(line, sizeof(line), "line %d/%d", lineCount ? topLine + 1 : 0, lineCount);
  else if(found && sync) snprintf(line, sizeof(line), "%d Hz, sync", (int)centre);
  else if(found) snprintf(line, sizeof(line), "%d Hz", (int)centre);
  else snprintf(line, sizeof(line), "No signal");
  spr.setTextColor(reading ? DG_RGB(0xFFD167) : found ? DG_RGB(0x06D7A0) : DG_RGB(0x8FB3C0));
  spr.drawString(line, 310, 14, uiFont(true));

  // Reading back: seven lines of the whole text, a bar for the place in it
  if(reading)
  {
    spr.setTextDatum(TL_DATUM);
    for(int r = 0; r < READ_ROWS && topLine + r < lineCount; r++)
    {
      uint32_t a = lineAt[topLine + r], b = lineAt[topLine + r + 1];
      while(b > a && (snap[b - 1] == '\n' || snap[b - 1] == ' ')) b--;
      int n = (int)(b - a) < Digi::TEXT_MAX ? (int)(b - a) : Digi::TEXT_MAX;
      memcpy(line, snap + a, n);
      line[n] = '\0';
      spr.setTextColor(n > 2 && line[0] == '=' && line[1] == '=' ? DG_RGB(0xFFD167) : DG_RGB(0xEEF5F7));
      spr.drawString(line, 8, 30 + r * 17, FONT_SMALL);
    }
    if(!lineCount)
    {
      spr.setTextDatum(MC_DATUM);
      spr.setTextColor(DG_RGB(0x8FB3C0));
      spr.drawString("Nothing decoded yet", 160, 88, uiFont(true));
    }
    if(lineCount > READ_ROWS)
    {
      const int by = 30, bh = READ_ROWS * 17;
      int h = bh * READ_ROWS / lineCount;
      if(h < 8) h = 8;
      spr.fillRect(315, by, 2, bh, DG_RGB(0x0E4A5E));
      spr.fillRect(315, by + (bh - h) * topLine / maxTop, 2, h, DG_RGB(0xFFD167));
    }
    spr.setTextDatum(BC_DATUM);
    spr.setTextColor(DG_RGB(0x8FB3C0));
    spr.drawString("Turn: scroll  Click: live text  Press: leave", 160, 169, uiFont(true));
    return;
  }

  // Spectrum 300..2700 Hz, with the tones the decoder listens to
  const int sx = 12, sy = 30, sh = 30;
  spr.drawRect(sx - 1, sy - 1, SPEC_W + 2, sh + 2, DG_RGB(0x0E4A5E));
  for(int x = 0; x < SPEC_W; x++)
  {
    int h = spec[x] * sh / 255;
    if(h) spr.drawFastVLine(sx + x, sy + sh - h, h, DG_RGB(0x108AB1));
  }
  auto hzX = [&](float hz) { return(sx + (int)((hz - SPEC_LO) * SPEC_W / (SPEC_HI - SPEC_LO))); };
  float half = mode == DM_DWD ? 225 : mode == DM_PSK31 ? 0 : 85;
  if(half) { spr.drawFastVLine(hzX(centre - half), sy, sh, DG_RGB(0xF04770)); spr.drawFastVLine(hzX(centre + half), sy, sh, DG_RGB(0xF04770)); }
  else spr.drawFastVLine(hzX(centre), sy, sh, DG_RGB(0xF04770));
  // What the mode expects, small, in the corner of the spectrum
  spr.setTextDatum(TR_DATUM);
  spr.setTextColor(DG_RGB(0x6F97A5));
  spr.drawString(modeInfo[mode], sx + SPEC_W - 3, sy + 2, uiFont(true));

  // WEFAX: the picture, newest line at the bottom
  if(mode == DM_WEFAX && fax && lock)
  {
    xSemaphoreTake(lock, portMAX_DELAY);
    int n = fax->count() < 98 ? fax->count() : 98;
    for(int r = 0; r < n; r++)
    {
      const uint8_t *row = fax->row(r);
      int y = 64 + 97 - r;
      for(int x = 0; x < 296; x++) { uint8_t v = row[x * Digi::Wefax::W / 296]; spr.drawPixel(12 + x, y, spr.color565(v, v, v)); }
    }
    xSemaphoreGive(lock);
    spr.setTextDatum(BC_DATUM);
    spr.setTextColor(DG_RGB(0x8FB3C0));
    spr.drawString("Turn: tune  Click: mode  Press: leave", 160, 169, uiFont(true));
    return;
  }

  // Text: the last five lines, broken between words. Lines of a fixed 32
  // characters cut words in two ("V" / "ALID UNTIL"); a word wider than the
  // screen, as a row of RYRY, is still cut where the line ends.
  char text[Digi::TEXT_MAX + 1];
  if(lock) { xSemaphoreTake(lock, portMAX_DELAY); strcpy(text, shownText); xSemaphoreGive(lock); }
  else text[0] = '\0';
  const int rows = 5, maxW = 304;
  static char lines[5][Digi::TEXT_MAX + 1];
  char cur[Digi::TEXT_MAX + 1] = "", trial[Digi::TEXT_MAX + 2], part[Digi::TEXT_MAX + 2];
  int count = 0;
  for(const char *p = text; *p; )
  {
    while(*p == ' ') p++;
    const char *w = p;
    while(*p && *p != ' ') p++;
    int wl = (int)(p - w);
    if(!wl) break;
    snprintf(trial, sizeof(trial), "%s%s%.*s", cur, cur[0] ? " " : "", wl, w);
    if(cur[0] && spr.textWidth(trial, FONT_SMALL) > maxW)
    {
      strcpy(lines[count++ % rows], cur);
      snprintf(trial, sizeof(trial), "%.*s", wl, w);
    }
    while(spr.textWidth(trial, FONT_SMALL) > maxW)
    {
      int n = strlen(trial), k = n;
      do { k--; memcpy(part, trial, k); part[k] = '\0'; } while(k > 1 && spr.textWidth(part, FONT_SMALL) > maxW);
      strcpy(lines[count++ % rows], part);
      memmove(trial, trial + k, n - k + 1);
    }
    strcpy(cur, trial);
  }
  if(cur[0]) strcpy(lines[count++ % rows], cur);
  int shown = count < rows ? count : rows;
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(DG_RGB(0xEEF5F7));
  for(int r = 0; r < shown; r++)
    spr.drawString(lines[(count - shown + r) % rows], 8, 64 + r * 17, FONT_SMALL);   // The fifth ends at 148, over the foot line
  spr.setTextDatum(BC_DATUM);
  spr.setTextColor(DG_RGB(0x8FB3C0));
  spr.drawString(mode == DM_NAVTEX ? "Turn: channel  Click: mode  Push+turn: text" :
                 "Turn: tune  Click: mode  Push+turn: text", 160, 169, uiFont(true));
}
