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
#include "Common.h"
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

enum DigiMode { DM_RTTY, DM_DWD, DM_NAVTEX, DM_PSK31, DM_WEFAX, DM_COUNT };
static const char *const modeName[DM_COUNT] = { "RTTY", "RTTY DWD", "NAVTEX", "PSK31", "WEFAX" };
static const char *const modeInfo[DM_COUNT] = { "45.45 Bd 170 Hz", "50 Bd 450 Hz", "100 Bd 518 kHz", "31.25 Bd BPSK", "120 lpm, dial -1.9 kHz" };

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

static Digi::Text *activeText()
{
  switch(workMode)
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
      pos = written;
      step = inRate >= 2 * FS ? 2 : 1;
      xSemaphoreTake(lock, portMAX_DELAY);
      beginDecoder(workMode, centre);
      shownText[0] = '\0';
      xSemaphoreGive(lock);
      memset(avgPower, 0, sizeof(avgPower));
      found = false;
      lastCentre = 0;
      quiet = 1000;
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
      switch(workMode)
      {
        case DM_RTTY: case DM_DWD: if(open) rtty->push(buf, n); break;
        case DM_NAVTEX: navtex->push(buf, n); break;
        case DM_WEFAX: { uint32_t was = fax->total(); fax->push(buf, n); if(fax->total() != was) changes++; break; }
        default: if(open) psk->push(buf, n); break;
      }
      Digi::Text *t = activeText();
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
        int m = workMode;
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

static void tuneForMode()
{
  // NAVTEX: 518 kHz, heard at 1500 Hz on USB; the others stay where they are
  digiSetUsb(mode == DM_NAVTEX ? 516500 : 0);
}

static bool start()
{
  if(!lock) lock = xSemaphoreCreateMutex();
  if(!ring) ring = (int16_t *)heap_caps_malloc(RING * sizeof(int16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if(!fax) fax = new (heap_caps_malloc(sizeof(Digi::Wefax), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)) Digi::Wefax();
  if(!rtty) rtty = new (heap_caps_malloc(sizeof(Digi::Rtty), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)) Digi::Rtty();
  if(!navtex) navtex = new (heap_caps_malloc(sizeof(Digi::Navtex), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)) Digi::Navtex();
  if(!psk) psk = new (heap_caps_malloc(sizeof(Digi::Psk31), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)) Digi::Psk31();
  if(!ring || !rtty || !navtex || !psk || !fax)
  {
    statusShow("Digital", "Not enough memory");
    return(false);
  }
  if(!worker) xTaskCreatePinnedToCore(work, "digi", 16384, nullptr, 1, &worker, 1);
  ft8SaveRadio();
  tuneForMode();
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
  running = false;
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
  j += "\",\"c\":"; j += found ? (int)centre : 0;
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

// The open page goes to another mode
static void goMode(int m)
{
  mode = m;
  if(!pageActive) return;
  tuneForMode();
  audioOn();
  workMode = mode;
  centre = mode == DM_PSK31 ? 1000 : mode == DM_WEFAX ? 1900 : 1500;
  generation++;
}

void digiClick(bool shortPress)
{
  if(shortPress) { currentCmd = CMD_NONE; return; }
  goMode((mode + 1) % DM_COUNT);
}

// RESONIK R5: Web Control starts the page in a mode, or changes mode
void digiWebOpen(int m)
{
  if(m < 0 || m >= DM_COUNT) return;
  if(pageActive) { if(m != mode) goMode(m); return; }
  mode = m;
  currentCmd = CMD_DIGI;
}

int digiWebMode() { return(pageActive ? mode : -1); }   // -1 while the page is closed
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

void drawDigiPage()
{
  char line[48];
  spr.fillSprite(DG_RGB(0x041F29));
  // Header: mode, its settings, the signal's audio frequency
  spr.fillRoundRect(4, 3, 312, 22, 6, DG_RGB(0x073A4B));
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(DG_RGB(0xFFD167));
  spr.drawString(modeName[mode], 10, 14, uiFont(false));
  spr.setTextColor(DG_RGB(0x8FB3C0));
  spr.drawString(modeInfo[mode], 10 + spr.textWidth(modeName[mode], uiFont(false)) + 8, 14, uiFont(true));
  spr.setTextDatum(MR_DATUM);
  if(found) snprintf(line, sizeof(line), "%d Hz", (int)centre);
  else snprintf(line, sizeof(line), "No signal");
  spr.setTextColor(found ? DG_RGB(0x06D7A0) : DG_RGB(0x8FB3C0));
  spr.drawString(line, 310, 14, uiFont(true));

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

  // Text: the last characters, five lines of 32
  char text[Digi::TEXT_MAX + 1];
  if(lock) { xSemaphoreTake(lock, portMAX_DELAY); strcpy(text, shownText); xSemaphoreGive(lock); }
  else text[0] = '\0';
  int len = strlen(text);
  const int per = 32, rows = 5;
  int start = len > per * rows ? len - per * rows : 0;
  start -= start % per;
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(DG_RGB(0xEEF5F7));
  for(int r = 0; r < rows; r++)
  {
    int at = start + r * per;
    if(at >= len) break;
    char row[per + 1];
    strncpy(row, text + at, per);
    row[per] = '\0';
    spr.drawString(row, 8, 64 + r * 17, FONT_SMALL);   // The fifth ends at 148, over the foot line
  }
  spr.setTextDatum(BC_DATUM);
  spr.setTextColor(DG_RGB(0x8FB3C0));
  spr.drawString("Turn: tune  Click: mode  Press: leave", 160, 169, uiFont(true));
}
