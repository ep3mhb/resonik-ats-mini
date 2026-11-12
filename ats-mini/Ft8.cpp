// RESONIK R3: FT8 decoder page.
//
// The V4B board wires the speaker line to GPIO11. While the page is open the
// CW decoder's sampler (CwAudio.cpp, core 0) reads it at 12 kHz into a ring
// in PSRAM; a worker task here feeds Ft8Rx.h, cuts the audio at the UTC
// 15 second slot edges and decodes each slot while the next one is coming in.
// The main loop only draws. Turning the knob past 10 m goes on to the FT4
// frequencies, with 7.5 second slots.
//
// Without a clock (R3) the page still decodes: it runs on the radio's own
// timer and looks for the band's slot edges itself. While searching it moves
// its windows by 3 s (FT8) or 1.2 s (FT4) after each empty slot, so within a
// minute or so a window lines up with the stations. The first decode tells
// it where the real edge is, and from then on it stays locked to the
// signals, refining the edge a little with every slot that decodes. Like the CW page, the page pauses Wi-Fi (ADC2 is
// shared with the Wi-Fi radio) and gives it back on leaving.
//
// On opening, the page saves the radio set-up, tunes the FT8 dial frequency
// of the band in use (or 40 m) in USB 3.0k, mutes the speaker amplifier (the
// ADC can be heard on it and FT8 tones are not for listening), sets the line
// level the ADC needs and runs the CPU at 240 MHz for the decoder. Leaving
// puts all of it back. The knob steps through the FT8 bands.
//
// The approach follows the epiHATR ats-mini fork, which first decoded FT8 on
// this radio with ft8_lib (Karlis Goba) and KISS FFT (Mark Borgerding).

#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"
#include "CwAudio.h"
#include "Ft8.h"
#include "Ft8Rx.h"

#include <sys/time.h>
#include <esp_heap_caps.h>
#include <freertos/semphr.h>

#define RING_SAMPLES  (Ft8Rx::FS * 8)   // 8 s of audio: room for a slow decode
#define LINE_VOLUME   50                // SI4732 volume while decoding (speaker is off)
#define LOG_MAX       30                // Decoded messages kept
#define WF_W          320               // Waterfall on screen: 200..2900 Hz
#define WF_H          36                // Rows, one per FT8 symbol (0.16 s)

static Ft8Rx ft8;
static int16_t *ring = nullptr;
static SemaphoreHandle_t lock = nullptr;
static TaskHandle_t worker = nullptr;

static bool pageActive = false;           // Main loop: the page is set up
static volatile bool running = false;     // The worker should process audio
static volatile uint32_t generation = 0;  // Bumped on every start and band change
static uint32_t cpuSaved = 0;
static int channel = 2;                   // 40 m until a band says otherwise
static volatile bool wantFt4 = false;     // Protocol of the channel; the worker switches

// Timing without a clock: our timer plus an offset found from the signals
static Ft8Sync slotSync;

// Milliseconds on the time line the slots are cut from
static int64_t slotClockMs()
{
  if(clockAvailable())
  {
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    return((int64_t)tv.tv_sec * 1000 + tv.tv_usec / 1000);
  }
  return((int64_t)millis() + slotSync.offsetMs);
}

// After a slot without a clock: line up with the stations heard, or move on
static void syncAfterSlot(const Ft8Rx::Msg *found, int n)
{
  if(clockAvailable()) { slotSync.state = Ft8Sync::UTC; return; }
  if(slotSync.state == Ft8Sync::UTC) slotSync.state = Ft8Sync::SEARCH;
  if(slotSync.afterSlot<Ft8Rx::Msg, Ft8Rx::edgeCorrectionMs>(found, n, ft8.ft4()))
    generation++;                         // Start again on the moved edge
}

// Shared with the worker (under lock)
typedef struct
{
  Ft8Rx::Msg m;
  uint32_t slot;        // Slot number (UTC seconds / 15) it was heard in
} LogEntry;

static LogEntry logBuf[LOG_MAX];
static int logCount = 0;
static uint8_t wf[WF_H][WF_W];
static int wfHead = 0;                    // Next row to write
static int wfCount = 0;
static volatile bool collecting = false;  // Inside a slot that will be decoded
static volatile int lastDecoded = -1;     // Messages in the last slot, -1 before the first
static volatile uint32_t lastDecodeMs = 0;
static volatile uint32_t changes = 0;     // Bumped when anything on the page changed

//
// Worker
//

static void addRow()
{
  const uint8_t *row = ft8.lastRow();
  if(!row) return;
  int bins = ft8.bins();

  // Shown against the noise of the row (its median), so the picture does
  // not depend on the input level: noise dark, a signal 10 dB up bright
  uint16_t hist[256] = { 0 };
  for(int b = 0 ; b < bins ; b++) hist[row[b]]++;
  int median = 0;
  for(int acc = 0 ; median < 255 && (acc += hist[median]) * 2 < bins ; median++);

  uint8_t line[WF_W];
  for(int x = 0 ; x < WF_W ; x++)
  {
    int b0 = x * bins / WF_W, b1 = (x + 1) * bins / WF_W;
    int v = 0;
    for(int b = b0 ; b < b1 || b == b0 ; b++) if(row[b] > v) v = row[b];
    // Waterfall units are half dB
    v = 56 + (v - median) * 3;
    line[x] = v < 0 ? 0 : v > 255 ? 255 : v;
  }
  xSemaphoreTake(lock, portMAX_DELAY);
  memcpy(wf[wfHead], line, WF_W);
  wfHead = (wfHead + 1) % WF_H;
  if(wfCount < WF_H) wfCount++;
  changes++;
  xSemaphoreGive(lock);
}

// Feed the ring from *pos up to end
static void feed(uint32_t *pos, uint32_t end)
{
  if((int32_t)(end - *pos) <= 0) { if((int32_t)(end - *pos) < 0) *pos = end; return; }
  // Fell too far behind: skip ahead (the slot will not decode, the next will)
  if(end - *pos > RING_SAMPLES - Ft8Rx::FS) *pos = end - (RING_SAMPLES - Ft8Rx::FS);
  while(*pos != end)
  {
    uint32_t idx = *pos % RING_SAMPLES;
    uint32_t n = end - *pos;
    if(n > RING_SAMPLES - idx) n = RING_SAMPLES - idx;
    if(n > 480) n = 480;
    if(ft8.push(ring + idx, n)) addRow();
    *pos += n;
  }
}

static void decodeSlot(uint32_t slot)
{
  static Ft8Rx::Msg found[LOG_MAX];
  uint32_t t = millis();
  int n = ft8.decode(found, LOG_MAX);
  t = millis() - t;
  syncAfterSlot(found, n);

  // Strongest first
  for(int i = 1 ; i < n ; i++)
    for(int j = i ; j > 0 && found[j].snr > found[j - 1].snr ; j--)
    {
      Ft8Rx::Msg tmp = found[j]; found[j] = found[j - 1]; found[j - 1] = tmp;
    }

  xSemaphoreTake(lock, portMAX_DELAY);
  // Newest slot on top: move the older entries down
  int keep = logCount + n > LOG_MAX ? LOG_MAX - n : logCount;
  if(keep < 0) keep = 0;
  memmove(logBuf + n, logBuf, keep * sizeof(LogEntry));
  for(int i = 0 ; i < n && i < LOG_MAX ; i++) { logBuf[i].m = found[i]; logBuf[i].slot = slot; }
  logCount = keep + n > LOG_MAX ? LOG_MAX : keep + n;
  lastDecoded = n;
  lastDecodeMs = t;
  changes++;
  xSemaphoreGive(lock);
}

static void work(void *)
{
  uint32_t gen = 0;
  uint32_t pos = 0;
  int64_t slot = -1;

  for(;;)
  {
    if(!running)
    {
      collecting = false;
      ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(200));
      gen = generation - 1;
      continue;
    }

    uint32_t written = audioFt8Written();

    if(gen != generation && ft8.ft4() != wantFt4 && !ft8.begin(wantFt4))
    {
      // Out of memory switching protocol: try again shortly
      vTaskDelay(pdMS_TO_TICKS(200));
      continue;
    }
    const int64_t slotMs = ft8.slotMs();
    int64_t ms = slotClockMs();
    int64_t now = ms / slotMs;
    uint32_t msInSlot = (uint32_t)(ms % slotMs);

    if(gen != generation)
    {
      // Started, or the band changed: wait for the next slot edge
      gen = generation;
      pos = written;
      slot = now;
      ft8.reset();
      collecting = msInSlot < 500;
    }
    else if(now != slot)
    {
      // Slot edge: the sample that matches it, by the clock
      uint32_t back = msInSlot * (Ft8Rx::FS / 1000);
      uint32_t edge = back < written ? written - back : 0;
      if((int32_t)(edge - pos) < 0) edge = pos;
      feed(&pos, edge);
      bool decode = collecting;
      // Next slot starts at the edge
      uint32_t heard = (uint32_t)slot;
      slot = now;
      collecting = true;
      if(decode) decodeSlot(heard);
      ft8.reset();
      // Audio of the new slot that came in while decoding
      written = audioFt8Written();
    }

    feed(&pos, written);
    vTaskDelay(pdMS_TO_TICKS(40));
  }
}

//
// Page
//

// Some boards take the decoder input before the speaker amplifier, others
// after it (the audio tap of the V4B). With the amplifier off the second kind
// gets nothing, so the page starts with it off and switches it on if the
// input stays silent; that holds until the radio is switched off.
static bool ampNeeded = false;
static uint32_t openedAt = 0;

static void audioForFt8()
{
  // Speaker amplifier off unless the input needs it, line level up for the ADC
  if(PIN_AMP_EN >= 0) digitalWrite(PIN_AMP_EN, ampNeeded ? HIGH : LOW);
  if(audioClamp) digitalWrite(AUDIO_MUTE, LOW);
  rx.setAudioMute(false);
  rx.setVolume(LINE_VOLUME);
}

static void audioRestore()
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

static bool start()
{
  if(!lock) lock = xSemaphoreCreateMutex();
  if(!ring) ring = (int16_t *)heap_caps_malloc(RING_SAMPLES * sizeof(int16_t), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  int ch = ft8ChannelOfCurrent();
  if(ch >= 0) channel = ch;
  wantFt4 = ft8ChannelIsFt4(channel);
  if(!ring || !ft8.begin(wantFt4))
  {
    statusShow("FT8", "Not enough memory");
    return(false);
  }
  if(!worker) xTaskCreatePinnedToCore(work, "ft8", 12288, nullptr, 1, &worker, 1);

  ft8SaveRadio();
  ft8SelectChannel(channel);
  audioForFt8();

  cpuSaved = getCpuFrequencyMhz();
  if(cpuSaved < 240) setCpuFrequencyMhz(240);

  xSemaphoreTake(lock, portMAX_DELAY);
  wfHead = wfCount = 0;
  lastDecoded = -1;
  xSemaphoreGive(lock);
  slotSync.state = clockAvailable() ? Ft8Sync::UTC : Ft8Sync::SEARCH;

  audioFt8Ring(ring, RING_SAMPLES);
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
  audioRestore();
}

void ft8PageOpen()
{
  currentCmd = CMD_FT8;
}

void ft8Turn(int16_t enc)
{
  if(!pageActive || !enc) return;
  int n = ft8ChannelCount();
  channel = (channel + (enc > 0 ? 1 : -1) + n) % n;
  bool wasFt4 = wantFt4;
  wantFt4 = ft8ChannelIsFt4(channel);
  ft8SelectChannel(channel);
  audioForFt8();
  // Without a clock the edges found hold on every band: FT8 edges (every
  // 15 s) are FT4 edges too, but FT4 edges (every 7.5 s) may be half an FT8
  // slot off, so going from FT4 to FT8 looks for them again
  if(slotSync.state == Ft8Sync::LOCKED && wasFt4 && !wantFt4) slotSync.state = Ft8Sync::SEARCH;
  // The list and the waterfall belong to the band that was left
  xSemaphoreTake(lock, portMAX_DELAY);
  wfHead = wfCount = 0;
  logCount = 0;
  lastDecoded = -1;
  xSemaphoreGive(lock);
  generation++;
}

void ft8Clear()
{
  if(!lock) return;
  xSemaphoreTake(lock, portMAX_DELAY);
  logCount = 0;
  changes++;
  xSemaphoreGive(lock);
}

bool ft8Tick()
{
  bool want = currentCmd == CMD_FT8 && !sleepOn();
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

  // Silent input with the amplifier off: this board taps after it
  if(!ampNeeded && PIN_AMP_EN >= 0 && millis() - openedAt > 1500 && audioInputLevel() < 2)
  {
    ampNeeded = true;
    audioForFt8();
  }

  // Redraw on news, and every half second for the clock and slot bar
  static uint32_t seen = 0, lastDraw = 0;
  if(changes != seen || millis() - lastDraw >= 500)
  {
    seen = changes;
    lastDraw = millis();
    return(true);
  }
  return(false);
}

//
// Drawing
//

#define FT_RGB(c)  ((uint16_t)((((c) >> 8) & 0xF800) | (((c) >> 5) & 0x07E0) | (((c) >> 3) & 0x001F)))
#define FT_NAVY    FT_RGB(0x073A4B)
#define FT_DEEP    FT_RGB(0x041F29)
#define FT_TRACK   FT_RGB(0x0E4A5E)
#define FT_GREEN   FT_RGB(0x06D7A0)
#define FT_YELLOW  FT_RGB(0xFFD167)
#define FT_PINK    FT_RGB(0xF04770)
#define FT_MUTED   FT_RGB(0x8FB3C0)
#define FT_TEXT    FT_RGB(0xC7DCE3)

// Waterfall colours: deep navy through blue, green and yellow to pink,
// the RESONIK palette
static uint16_t wfColourCalc(uint8_t v)
{
  static const uint8_t stops[][4] =
  {
    { 0,   0x04, 0x1F, 0x29 },
    { 64,  0x07, 0x3A, 0x4B },
    { 112, 0x10, 0x8A, 0xB1 },
    { 144, 0x06, 0xD7, 0xA0 },
    { 180, 0xFF, 0xD1, 0x67 },
    { 220, 0xF0, 0x47, 0x70 },
    { 255, 0xFF, 0xFF, 0xFF },
  };
  int i = 0;
  while(i < 5 && v > stops[i + 1][0]) i++;
  const uint8_t *a = stops[i], *b = stops[i + 1];
  int span = b[0] - a[0];
  int t = span ? (v - a[0]) * 256 / span : 0;
  if(t < 0) t = 0;
  if(t > 256) t = 256;
  uint8_t r = a[1] + ((b[1] - a[1]) * t >> 8);
  uint8_t g = a[2] + ((b[2] - a[2]) * t >> 8);
  uint8_t bl = a[3] + ((b[3] - a[3]) * t >> 8);
  return(spr.color565(r, g, bl));
}

static uint16_t wfColour(uint8_t v)
{
  static uint16_t lut[256];
  static bool built = false;
  if(!built)
  {
    for(int i = 0 ; i < 256 ; i++) lut[i] = wfColourCalc(i);
    built = true;
  }
  return(lut[v]);
}

static int hzToX(int hz)
{
  return((hz - Ft8Rx::F_MIN) * WF_W / (Ft8Rx::F_MAX - Ft8Rx::F_MIN));
}

void drawFt8Page()
{
  char buf[40];
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  const bool ft4 = wantFt4;
  const int slotMs = ft4 ? 7500 : 15000;
  int64_t ms = slotClockMs();
  int msIn = (int)(ms % slotMs);
  uint32_t slotNow = (uint32_t)(ms / slotMs);

  spr.fillSprite(FT_DEEP);

  // Header: band, dial frequency, UTC
  spr.fillRect(0, 0, 320, 28, FT_NAVY);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(0xFFFF);
  spr.setTextColor(ft4 ? FT_YELLOW : 0xFFFF);
  spr.drawString(ft4 ? "FT4" : "FT8", 10, 14, uiFont(false));
  spr.setTextColor(FT_MUTED);
  spr.drawString(ft8ChannelName(channel), 48, 14, uiFont(true));
  snprintf(buf, sizeof(buf), "%lu.%03lu", (unsigned long)(currentFrequency / 1000000), (unsigned long)(currentFrequency / 1000 % 1000));
  spr.setTextColor(FT_YELLOW);
  if(ft4) spr.setTextColor(0xFFFF);
  spr.drawString(buf, 88, 15, &lgfx::fonts::FreeSansBold12pt7b);
  spr.setTextDatum(MR_DATUM);
  if(clockAvailable())
  {
    struct tm tm;
    time_t t = tv.tv_sec;
    gmtime_r(&t, &tm);
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d UTC", tm.tm_hour, tm.tm_min, tm.tm_sec);
    spr.setTextColor(0xFFFF);
  }
  else
  {
    // No clock: say whether the edges are found from the signals yet
    bool locked = slotSync.state == Ft8Sync::LOCKED;
    snprintf(buf, sizeof(buf), locked ? "Synced to band" : "No clock: syncing");
    spr.setTextColor(locked ? FT_GREEN : FT_YELLOW);
  }
  spr.drawString(buf, 312, 14, uiFont(true));

  // Slot progress: green while a slot is being collected
  spr.fillRect(0, 28, 320, 3, FT_TRACK);
  spr.fillRect(0, 28, msIn * 320 / slotMs, 3, collecting ? FT_GREEN : FT_MUTED);

  // Waterfall, newest row at the bottom
  const int wy = 32;
  xSemaphoreTake(lock, portMAX_DELAY);
  for(int r = 0 ; r < wfCount ; r++)
  {
    const uint8_t *row = wf[(wfHead - 1 - r + WF_H) % WF_H];
    int y = wy + WF_H - 1 - r;
    for(int x = 0 ; x < WF_W ; x++) spr.drawPixel(x, y, wfColour(row[x]));
  }
  // Frequency marks every 500 Hz
  for(int hz = 500 ; hz < Ft8Rx::F_MAX ; hz += 500)
    spr.drawFastVLine(hzToX(hz), wy, hz % 1000 ? 2 : 4, FT_MUTED);

  // Messages: dB, audio frequency, text; newest slot first
  const int ly = wy + WF_H + 2, lh = 16, lines = 5;
  const lgfx::IFont *font = uiFont(true);
  int shown = logCount < lines ? logCount : lines;
  for(int i = 0 ; i < shown ; i++)
  {
    const LogEntry &e = logBuf[i];
    int y = ly + i * lh + lh / 2;
    bool fresh = e.slot + 1 >= slotNow;
    bool cq = !strncmp(e.m.text, "CQ ", 3);
    spr.setTextDatum(MR_DATUM);
    spr.setTextColor(fresh ? FT_TEXT : FT_TRACK);
    snprintf(buf, sizeof(buf), "%+d", e.m.snr);
    spr.drawString(buf, 34, y, font);
    snprintf(buf, sizeof(buf), "%u", e.m.freq);
    spr.drawString(buf, 76, y, font);
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(!fresh ? FT_MUTED : cq ? FT_GREEN : 0xFFFF);
    spr.drawString(e.m.text, 86, y, font);
    if(cq && fresh) spr.fillCircle(hzToX(e.m.freq + 22), wy + 2, 2, FT_GREEN);
  }
  int decoded = lastDecoded;
  xSemaphoreGive(lock);

  if(!shown)
  {
    const char *l1, *l2 = nullptr;
    snprintf(buf, sizeof(buf), "First slot in %d s", (slotMs - msIn + 999) / 1000);
    if(audioInputLevel() < 2) { l1 = "No audio on the decoder input"; l2 = "Needs the V4B board (speaker line on GPIO11)"; }
    else if(!clockAvailable() && slotSync.state != Ft8Sync::LOCKED) { l1 = "Finding the slot timing"; l2 = "No clock: lining up with the stations heard"; }
    else if(decoded < 0) { l1 = "Listening"; l2 = collecting ? "Decoding at the end of this slot" : buf; }
    else { l1 = "Nothing decoded yet"; l2 = "Check the antenna, or try another band"; }
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(0xFFFF);
    spr.drawString(l1, 160, ly + 26, uiFont(false));
    spr.setTextColor(FT_MUTED);
    if(l2) spr.drawString(l2, 160, ly + 50, font);
  }

  // Footer: input level, last slot, controls
  spr.drawFastHLine(0, 154, 320, FT_TRACK);
  spr.setTextDatum(BL_DATUM);
  spr.setTextColor(FT_MUTED);
  // "Spk" when the speaker had to stay on for the input to get audio
  if(decoded >= 0) snprintf(buf, sizeof(buf), "In %u%s  Last %d", audioInputLevel(), ampNeeded ? " Spk" : "", decoded);
  else snprintf(buf, sizeof(buf), "In %u%s", audioInputLevel(), ampNeeded ? " Spk" : "");
  spr.drawString(buf, 6, 169, font);
  spr.setTextDatum(BR_DATUM);
  spr.drawString("Turn: band  Press: exit", 314, 169, font);
  spr.pushSprite(0, 0);
}
