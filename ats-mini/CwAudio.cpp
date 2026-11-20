// RESONIK R3: CW decoder from the speaker line.
//
// The V4B board wires the speaker output to GPIO11, ADC2 channel 0. A task on
// core 0 samples it at 4 kHz and feeds CwDecoder.h; the main loop only draws.
// ADC2 is shared with the Wi-Fi radio, so the decoder needs Wi-Fi off: the
// full-screen page pauses Wi-Fi while it is open, the strip on the normal
// screen simply waits until Wi-Fi is off (for instance after the time sync).

#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"
#include "CwAudio.h"
#include <esp_heap_caps.h>
#include "CwDecoder.h"

#include <WiFi.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <driver/gptimer.h>
#include <freertos/semphr.h>

#define CW_FS         4000      // Sample rate, Hz
#define CW_BLOCK      24        // Samples handed to the decoder at a time
#define CW_ADC_UNIT   ADC_UNIT_2
#define CW_ADC_CH     ADC_CHANNEL_0   // GPIO11

uint8_t cwStripIdx = 0;   // Off by default: sampling GPIO11 can be heard faintly on the speaker
const char *const cwStripDesc[2] = { "Off", "On" };

static CwDecoder decoder;
static SemaphoreHandle_t lock = nullptr;
static adc_oneshot_unit_handle_t adc = nullptr;
static volatile bool want = false;        // The main loop wants samples
static bool padDetachPending = false;
static volatile bool sampling = false;    // The task is sampling
static volatile uint16_t audioLevel = 0;  // Mean deviation of the raw samples, ADC counts
static volatile uint32_t adcErrors = 0;
static volatile uint32_t timingRestarts = 0;  // Sampler fell behind and lost samples

static bool wifiPaused = false;           // The page switched Wi-Fi off and owes it back
static uint32_t lastPoll = 0;
static char shownText[CwDecoder::TEXT_MAX + 1];
static bool shownKey = false;
static int shownWpm = 0;

// FT8 (Ft8.cpp) borrows the sampler: at 12 kHz the samples go into its ring
#define FT8_FS        12000
static int16_t *volatile ft8Ring = nullptr;
static uint32_t ft8RingSize = 0;
static volatile uint32_t ft8Written = 0;   // Samples written since the start

// RESONIK R4 (test): audio for the Web Control page, 8 kHz, with Wi-Fi on.
// GPIO11 is on ADC2, which Wi-Fi shares, so reads can fail; they are counted.
#define WEB_FS        8000
#define WEB_RING      16384                  // About 2 s
static volatile bool webAudio = false;
static int16_t *webRing = nullptr;
static volatile uint32_t webWritten = 0;

// With Wi-Fi on, the sampler must not busy-wait: core 0 also runs Wi-Fi, and
// its idle task starved until the task watchdog restarted the radio (R4
// test 1). Then a hardware timer wakes the sampler for each sample instead.
static TaskHandle_t samplerTask = nullptr;
static gptimer_handle_t webTimer = nullptr;
static bool webTimerOn = false;
static int webTimerFs = 0;
static volatile bool timedSampling = false;  // Wi-Fi is on: use the timer
// Timed sampling statistics for the Web Control page (R4 test)
static volatile uint32_t readUsMax = 0, readUsAvg = 0, samplesLate = 0;
static volatile uint32_t samplesCaughtUp = 0;   // Missed ticks: filled in (web) or taken late (decoders)
static volatile uint32_t webOverrun = 0;        // Lost because the ring was not read in time

static bool IRAM_ATTR webTimerAlarm(gptimer_handle_t, const gptimer_alarm_event_data_t *, void *)
{
  BaseType_t woken = pdFALSE;
  if(samplerTask) vTaskNotifyGiveFromISR(samplerTask, &woken);
  return(woken == pdTRUE);
}

static void webTimerRun(bool on, int fs)
{
  if(on == webTimerOn && (!on || fs == webTimerFs)) return;
  if(!webTimer)
  {
    gptimer_config_t cfg = {};
    cfg.clk_src = GPTIMER_CLK_SRC_DEFAULT;
    cfg.direction = GPTIMER_COUNT_UP;
    cfg.resolution_hz = 1000000;
    if(gptimer_new_timer(&cfg, &webTimer) != ESP_OK) { webTimer = nullptr; return; }
    gptimer_event_callbacks_t cbs = {};
    cbs.on_alarm = webTimerAlarm;
    gptimer_register_event_callbacks(webTimer, &cbs, nullptr);
    gptimer_enable(webTimer);
  }
  if(webTimerOn) gptimer_stop(webTimer);
  webTimerOn = false;
  if(!on) return;
  // Whole microseconds: 125 us for 8 kHz, 250 for 4 kHz, 83 for 12 kHz
  // (slightly fast; the sampler skips a tick now and then to stay exact)
  gptimer_alarm_config_t alarm = {};
  alarm.alarm_count = 1000000 / fs;
  alarm.reload_count = 0;
  alarm.flags.auto_reload_on_alarm = true;
  gptimer_set_alarm_action(webTimer, &alarm);
  gptimer_start(webTimer);
  webTimerOn = true;
  webTimerFs = fs;
}

static void sampler(void *)
{
  int16_t buf[CW_BLOCK];
  int n = 0;
  int64_t next = 0;
  // Sample times as base + k * 1e6 / fs, so 12 kHz is exact: whole
  // microsecond steps gave 83 us = 12048 Hz, 0.4 % fast (about 2 dB of FT8)
  int64_t base = 0;
  uint64_t k = 0;
  int lastFs = 0;
  uint32_t sinceYield = 0;
  uint32_t webInterp = 0;          // Web audio: missed ticks to fill in
  int webLast = 2048;
  float dc = 2048, dev = 0;
  uint32_t devCount = 0;

  for(;;)
  {
    if(!want || !adc)
    {
      sampling = false;
      webTimerRun(false, 0);
      vTaskDelay(pdMS_TO_TICKS(50));
      continue;
    }
    int16_t *ring = ft8Ring;
    bool web = !ring && webAudio && webRing;
    bool timed = timedSampling;
    int fs = ring ? FT8_FS : web ? WEB_FS : CW_FS;
    if(!sampling || fs != lastFs)
    {
      sampling = true;
      n = 0;
      lastFs = fs;
      base = next = esp_timer_get_time();
      k = 0;
    }
    webTimerRun(timed, fs);

    // Web audio: sleep until the timer says so. Decoders (Wi-Fi off): busy-wait
    // to the next sample time, steady timing matters more than the idle core
    if(timed)
    {
      // Timer ticks not yet sampled; a few behind are caught up, more are dropped
      static uint32_t backlog = 0;
      if(!backlog)
      {
        uint32_t c = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(5));
        if(!c) continue;
        if(c > 32) { samplesLate += c - 32; c = 32; }
        if(c > 1) samplesCaughtUp += c - 1;
        // Web audio: one fresh reading, the missed ones filled in between
        // (a burst of late readings sounds like a buzz). Decoders: catch up.
        if(web && c > 1) { webInterp = c - 1; c = 1; }
        backlog = c;
      }
      backlog--;
      // Ahead of the exact rate (12 kHz on an 83 us timer): skip this tick
      if((int64_t)((k + 1) * 1000000ULL / (uint64_t)fs) > esp_timer_get_time() - base + 40) continue;
    }
    else while(esp_timer_get_time() < next);
    k++;
    next = base + (int64_t)(k * 1000000ULL / (uint64_t)fs);

    int v = 0;
    int64_t t0 = timed ? esp_timer_get_time() : 0;
    if(adc_oneshot_read(adc, CW_ADC_CH, &v) != ESP_OK)
    {
      adcErrors++;
      v = (int)dc;
    }
    if(timed)
    {
      uint32_t us = (uint32_t)(esp_timer_get_time() - t0);
      if(us > readUsMax) readUsMax = us;
      readUsAvg = (readUsAvg * 255 + us * 16) / 256;      // x16, smoothed
      // Always leave the idle task a moment, even when behind: without it the
      // task watchdog restarted the radio (R4 tests 1 and 2)
      // Every 100 ms: enough for the watchdog. Every 10 ms (test 3 to 6)
      // left a 1 ms hole each time, 10 % of the sound filled in.
      if(++sinceYield >= (uint32_t)fs / 10)
      {
        sinceYield = 0;
        vTaskDelay(1);
      }
    }

    // Audio level for the input check on the page
    dc += (v - dc) * 0.001f;
    dev += fabsf(v - dc);
    if(++devCount >= (uint32_t)fs / 2)
    {
      audioLevel = (uint16_t)(dev / devCount);
      dev = 0;
      devCount = 0;
    }

    if(ring)
    {
      ring[ft8Written % ft8RingSize] = (int16_t)v;
      ft8Written = ft8Written + 1;
    }
    else if(web)
    {
      for(uint32_t i = 1; i <= webInterp; i++)
      {
        webRing[webWritten % WEB_RING] = (int16_t)(webLast + (v - webLast) * (int)i / (int)(webInterp + 1));
        webWritten = webWritten + 1;
      }
      webInterp = 0;
      webLast = v;
      webRing[webWritten % WEB_RING] = (int16_t)v;
      webWritten = webWritten + 1;
    }
    else buf[n++] = (int16_t)v;

    if(n == CW_BLOCK)
    {
      xSemaphoreTake(lock, portMAX_DELAY);
      decoder.push(buf, n);
      xSemaphoreGive(lock);
      n = 0;
    }

    // Let the idle task run now and then, or the watchdog bites
    if(!timed && ++sinceYield >= (uint32_t)fs / 2)
    {
      sinceYield = 0;
      vTaskDelay(1);
      // FT8 counts samples against the clock: catch up rather than skip
      if(!ring && !web) { base = next = esp_timer_get_time(); k = 0; }
    }
    // Fell far behind (interrupts): start the timing again
    if(!timed && esp_timer_get_time() - next > 10000) { base = next = esp_timer_get_time(); k = 0; timingRestarts++; }
  }
}

void cwAudioInit()
{
  lock = xSemaphoreCreateMutex();
  decoder.begin(CW_FS, cwPitch());
  xTaskCreatePinnedToCore(sampler, "cwaudio", 4096, nullptr, 2, &samplerTask, 0);
}

static bool adcOpen()
{
  if(adc) return(true);
  adc_oneshot_unit_init_cfg_t unit = {};
  unit.unit_id = CW_ADC_UNIT;
  if(adc_oneshot_new_unit(&unit, &adc) != ESP_OK) { adc = nullptr; return(false); }
  adc_oneshot_chan_cfg_t ch = {};
  ch.atten = ADC_ATTEN_DB_12;
  ch.bitwidth = ADC_BITWIDTH_DEFAULT;
  if(adc_oneshot_config_channel(adc, CW_ADC_CH, &ch) != ESP_OK)
  {
    adc_oneshot_del_unit(adc);
    adc = nullptr;
    return(false);
  }
  return(true);
}

static bool wifiOn() { return(WiFi.getMode() != WIFI_MODE_NULL); }

// The CW decoder page, or the FT8 page, which uses the same sampler
static bool pageOpen() { return(currentCmd == CMD_CWDEC || currentCmd == CMD_FT8 || currentCmd == CMD_DIGI); }

// FT8 page (Ft8.cpp): hand the sampler a ring, or nullptr to give it back
void audioFt8Ring(int16_t *ring, uint32_t size)
{
  if(ring) { ft8RingSize = size; ft8Written = 0; }
  ft8Ring = ring;
  // Let a sample in flight land before the caller frees the old ring
  if(!ring) { int64_t t = esp_timer_get_time(); while(esp_timer_get_time() - t < 300); }
}

uint32_t audioFt8Written() { return(ft8Written); }

void audioWebStream(bool on)
{
  if(on && !webRing)
    webRing = (int16_t *)heap_caps_malloc(WEB_RING * sizeof(int16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  webAudio = on && webRing;
}

bool audioWebStreaming() { return(webAudio && sampling); }

// Copy the samples written since *pos (at most max); the oldest are skipped
// when the reader fell more than a ring behind
void audioTimedStats(uint32_t *avgUs, uint32_t *maxUs, uint32_t *late, uint32_t *caughtUp, uint32_t *overrun)
{
  *avgUs = readUsAvg / 16;
  *maxUs = readUsMax;
  *late = samplesLate;
  *caughtUp = samplesCaughtUp;
  *overrun = webOverrun;
  readUsMax = 0;
}

uint32_t audioWebBacklog(uint32_t pos) { return(webRing ? webWritten - pos : 0); }

uint32_t audioWebRead(int16_t *dst, uint32_t max, uint32_t *pos)
{
  if(!webRing) return(0);
  uint32_t w = webWritten;
  if(w - *pos > WEB_RING - 256) { webOverrun += (w - 2048) - *pos; *pos = w - 2048; }
  uint32_t n = 0;
  while(*pos != w && n < max) dst[n++] = webRing[(*pos)++ % WEB_RING];
  return(n);
}
uint16_t audioInputLevel() { return(audioLevel); }
uint32_t audioTimingRestarts() { return(timingRestarts); }
uint32_t audioAdcErrors() { return(adcErrors); }

bool cwStripShown()
{
  return(cwStripIdx && currentMode == CW && !sleepOn());
}

void cwPageOpen()
{
  currentCmd = CMD_CWDEC;
}

void cwPageClear()
{
  xSemaphoreTake(lock, portMAX_DELAY);
  decoder.clearText();
  xSemaphoreGive(lock);
}

bool cwAudioTick()
{
  bool page = pageOpen() && !sleepOn();

  // CW page: at least a steady receiver level for the decoder, whatever the
  // volume setting (the knob tunes on this page). The speaker stays on, so the
  // signal can still be heard; the user's volume comes back on leaving.
  // R5: the same level as the FT8 page (50, was 45), and never below the
  // user's own volume: a user report on R4 said the sound dropped sharply on
  // opening the page and nothing decoded, while FT8 at 50 worked.
  static bool cwLevel = false;
  bool cwPage = currentCmd == CMD_CWDEC;
  if(cwPage && !cwLevel) { rx.setVolume(volume > CW_PAGE_VOLUME ? volume : CW_PAGE_VOLUME); cwLevel = true; }
  else if(!cwPage && cwLevel) { rx.setVolume(volume); cwLevel = false; }

  // The page owns Wi-Fi while it is open and gives it back afterwards
  // (not while a web request runs: Wi-Fi off under it crashes the radio)
  // Not while Web Control is open: then the page runs with Wi-Fi on, so it
  // can be watched and used from the browser (R4 test)
  if(page && wifiOn() && !wifiPaused && !netJobsBusy() && !webControlActive())
  {
    netStop();
    wifiPaused = true;
  }
  else if(!page && wifiPaused)
  {
    wifiPaused = false;
    netInit(wifiModeIdx);
  }

  timedSampling = wifiOn();
  bool need = (page && (!wifiOn() || webControlActive())) || (cwStripShown() && !wifiOn()) || (webAudio && !page);
  if(need && !want)
  {
    // Connect the pad to the ADC again (it is detached while idle)
    if(adcOpen())
    {
      adc_oneshot_chan_cfg_t ch = {};
      ch.atten = ADC_ATTEN_DB_12;
      ch.bitwidth = ADC_BITWIDTH_DEFAULT;
      adc_oneshot_config_channel(adc, CW_ADC_CH, &ch);
      want = true;
    }
  }
  else if(!need && want)
  {
    want = false;
    padDetachPending = true;
  }
  // Once the task has stopped, detach the pad: an ADC left on GPIO11 couples
  // a faint buzz into the speaker line (found by the epiHATR FT8 fork)
  if(padDetachPending && !sampling)
  {
    padDetachPending = false;
    gpio_reset_pin((gpio_num_t)11);
    pinMode(11, INPUT);
  }

  // Follow the CW pitch of the current filter
  if(want && (int)decoder.getTone() != cwPitch())
  {
    xSemaphoreTake(lock, portMAX_DELAY);
    decoder.setTone(cwPitch());
    xSemaphoreGive(lock);
  }

  // Redraw when the text, the key or the speed changed (polled at 10 Hz)
  if(!sampling || millis() - lastPoll < 100) return(false);
  lastPoll = millis();
  xSemaphoreTake(lock, portMAX_DELAY);
  bool changed = strcmp(shownText, decoder.getText()) || shownKey != decoder.isKeyDown() || shownWpm != decoder.getWpm();
  strcpy(shownText, decoder.getText());
  shownKey = decoder.isKeyDown();
  shownWpm = decoder.getWpm();
  xSemaphoreGive(lock);
  return(changed || page);
}

//
// Drawing
//

#define CW_RGB(c)  ((uint16_t)((((c) >> 8) & 0xF800) | (((c) >> 5) & 0x07E0) | (((c) >> 3) & 0x001F)))
#define CW_NAVY    CW_RGB(0x073A4B)
#define CW_DEEP    CW_RGB(0x041F29)
#define CW_TRACK   CW_RGB(0x0E4A5E)
#define CW_GREEN   CW_RGB(0x06D7A0)
#define CW_BLUE    CW_RGB(0x108AB1)
#define CW_YELLOW  CW_RGB(0xFFD167)
#define CW_MUTED   CW_RGB(0x8FB3C0)
#define CW_TEXT    CW_RGB(0xC7DCE3)

static void levelBar(int x, int y, int w, int h)
{
  uint8_t lvl;
  bool key;
  xSemaphoreTake(lock, portMAX_DELAY);
  lvl = decoder.getLevel();
  key = decoder.isKeyDown();
  xSemaphoreGive(lock);

  spr.fillRoundRect(x, y, w, h, h / 2, CW_TRACK);
  int f = w * lvl / 100;
  if(sampling && f > h) spr.fillRoundRect(x, y, f, h, h / 2, key ? CW_GREEN : CW_BLUE);
  spr.drawFastVLine(x + w * 55 / 100, y - 2, h + 4, CW_YELLOW);
}

// The last lines of text that fit the width, newest at the bottom
static int wrapLines(const char *text, const lgfx::IFont *font, int width, const char *lines[], int len[], int maxLines)
{
  int total = strlen(text);
  int count = 0;
  int end = total;
  while(end > 0 && count < maxLines)
  {
    // Longest tail of text[0..end) that fits, broken at a space when possible
    int start = end;
    char tmp[CwDecoder::TEXT_MAX + 1];
    while(start > 0)
    {
      int l = end - start + 1;
      memcpy(tmp, text + start - 1, l);
      tmp[l] = '\0';
      if(spr.textWidth(tmp, font) > width) break;
      start--;
    }
    if(start > 0)
    {
      int sp = start;
      while(sp < end && text[sp] != ' ') sp++;
      if(sp < end) start = sp + 1;
    }
    for(int i = count ; i > 0 ; i--) { lines[i] = lines[i - 1]; len[i] = len[i - 1]; }
    lines[0] = text + start;
    len[0] = end - start;
    count++;
    end = start;
    while(end > 0 && text[end - 1] == ' ') end--;
  }
  return(count);
}

static void drawLines(int x, int y, int width, int lineH, int maxLines, uint16_t old, uint16_t last)
{
  const lgfx::IFont *font = uiFont(false);
  const char *lines[6];
  int len[6];
  int n = wrapLines(shownText, font, width, lines, len, maxLines);
  spr.setTextDatum(TL_DATUM);
  for(int i = 0 ; i < n ; i++)
  {
    char buf[CwDecoder::TEXT_MAX + 2];
    memcpy(buf, lines[i], len[i]);
    buf[len[i]] = '\0';
    if(i == n - 1) strcat(buf, "_");
    spr.setTextColor(i == n - 1 ? last : old);
    spr.drawString(buf, x, y + (maxLines - n + i) * lineH, font);
  }
}

static const char *waitReason()
{
  if(wifiOn() && !(pageOpen() && webControlActive()))
    return(pageOpen() ? "Pausing Wi-Fi..." : "Wi-Fi on: open CW Decoder");
  if(!adc) return("Audio input not available");
  return(nullptr);
}

// Strip on the normal screen, in place of the tuning scale
void drawCwStrip()
{
  // Speed, level and key on the right, above the text
  spr.fillRoundRect(88, 103, 231, 26, 8, CW_NAVY);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(0xFFFF);
  spr.drawString("CW", 96, 116, uiFont(true));
  char wpm[12];
  snprintf(wpm, sizeof(wpm), "%d WPM", shownWpm);
  spr.setTextColor(CW_YELLOW);
  spr.drawString(sampling ? wpm : "-- WPM", 124, 116, uiFont(true));
  levelBar(196, 112, 96, 8);
  spr.fillCircle(306, 116, 5, sampling && shownKey ? CW_GREEN : CW_TRACK);

  // Two lines of text across the bottom
  spr.fillRoundRect(0, 131, 320, 39, 8, CW_NAVY);
  const char *why = waitReason();
  if(why)
  {
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(CW_MUTED);
    spr.drawString(why, 160, 150, uiFont(true));
  }
  else drawLines(8, 133, 304, 18, 2, CW_TEXT, 0xFFFF);
}

// Full-screen page
void drawCwPage()
{
  spr.fillSprite(CW_DEEP);
  spr.fillRect(0, 0, 320, 30, CW_NAVY);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(0xFFFF);
  spr.drawString("CW decoder", 10, 15, uiFont(false));
  spr.setTextColor(CW_MUTED);
  char buf[32];
  snprintf(buf, sizeof(buf), "%s  %s", bandModeDesc[currentMode], getCurrentBandwidth()->desc);
  spr.drawString(buf, 112, 15, uiFont(true));

  // Frequency in kHz, as on the main screen
  snprintf(buf, sizeof(buf), "%lu.%03lu", (unsigned long)(currentFrequency / 1000), (unsigned long)(currentFrequency % 1000));
  spr.setTextDatum(MR_DATUM);
  spr.setTextColor(CW_YELLOW);
  spr.drawString(buf, 310, 15, &lgfx::fonts::FreeSansBold12pt7b);

  // Speed, tone, level and key
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(0xFFFF);
  snprintf(buf, sizeof(buf), "%d WPM", shownWpm);
  spr.drawString(sampling ? buf : "-- WPM", 10, 44, uiFont(false));
  spr.setTextColor(CW_MUTED);
  snprintf(buf, sizeof(buf), "Tone %d Hz", cwPitch());
  spr.drawString(buf, 86, 44, uiFont(true));
  levelBar(190, 39, 90, 10);
  spr.fillCircle(298, 44, 6, sampling && shownKey ? CW_GREEN : CW_TRACK);
  spr.drawFastHLine(10, 58, 300, CW_TRACK);

  const char *why = waitReason();
  if(why)
  {
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(CW_MUTED);
    spr.drawString(why, 160, 108, uiFont(false));
  }
  else if(sampling && audioLevel < 3)
  {
    // Input check: almost nothing on GPIO11
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(CW_YELLOW);
    spr.drawString("No audio on the decoder input", 160, 100, uiFont(false));
    spr.setTextColor(CW_MUTED);
    spr.drawString("Volume up, unplug headphones", 160, 122, uiFont(true));
  }
  else drawLines(10, 64, 300, 22, 4, CW_TEXT, 0xFFFF);

  // Footer: raw input level for the hardware check, and the controls
  spr.setTextDatum(BL_DATUM);
  spr.setTextColor(CW_MUTED);
  snprintf(buf, sizeof(buf), "In %u", (unsigned)audioLevel);
  spr.drawString(buf, 6, 168, uiFont(true));
  spr.setTextDatum(BR_DATUM);
  spr.drawString("Turn: tune  Click: clear  Press: exit", 314, 168, uiFont(true));
  spr.pushSprite(0, 0);
}
