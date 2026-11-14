//
// RESONIK R4 (test): Web Control. The radio's screen live in the browser,
// with buttons for the knob, click, press, volume, band, mode, step and
// filter, a frequency box, and (V4B boards, experimental) the audio.
//
// One WebSocket at /ws carries everything:
//   browser -> radio   text: ad hoc remote commands, the same as over USB
//                      ("R", "r", "e", "E", "V", "v", "B", "b", "M", ...,
//                      "F7074000\n"), run by remoteLoop() on the main task;
//                      "!audio1" / "!audio0" switch the audio stream,
//                      "!seek+" / "!seek-" seek, "!scope" sweeps the band
//   radio -> browser   binary 0x01 y n + n rows of RGB565 (high byte first):
//                      the rows of the screen that changed since last sent;
//                      binary 0x02 0x00 + 16-bit samples at 8 kHz: audio;
//                      text {...}: status once a second, {"p":..} the
//                      panel (frequency, mode, S-meter, RDS) when it changes,
//                      {"scope":..} the result of a band sweep
//
#include "Common.h"
#include "Utils.h"
#include "Menu.h"
#include "Storage.h"
#include "Remote.h"
#include "CwAudio.h"
#include "WebControl.h"
#include "PageControl.h"
#include "Memories.h"
#include "Beacons.h"
#include "BatteryTime.h"
#include <ESPAsyncWebServer.h>
#include <mutex>
#include <atomic>

static AsyncWebSocket ws("/ws");

// Command bytes from the socket task to the main loop
static std::mutex inMutex;
static uint8_t inBuf[512];
static uint16_t inHead = 0, inTail = 0;
static std::atomic<bool> sessionReset{false};
static std::atomic<bool> fullFrame{true};
static std::atomic<int8_t> audioWanted{-1};      // -1 no change, 0 off, 1 on
static std::atomic<uint32_t> tuneWanted{0};       // Frequency in Hz from the page
static std::atomic<int8_t> muteWanted{-1};        // Speaker while listening: steps quieter (0 normal)
static std::atomic<int8_t> seekWanted{0};        // +1 up, -1 down
static std::atomic<bool> scopeWanted{false};
static std::atomic<bool> screenWanted{false};     // The radio's screen: only when the page shows it
static std::atomic<bool> defWanted{true};         // Settings list (on connect and on request)
static std::atomic<bool> memWanted{true};         // Memory list

// Settings and memory actions from the page, run on the main task
struct WebSet { uint16_t cmd; int32_t value; };
static WebSet setQueue[16];
static uint8_t setHead = 0, setTail = 0;            // Under inMutex
static std::atomic<bool> webBusy{false};          // A seek or sweep is running
static std::atomic<bool> webAbort{false};         // ...and the page sent something
static RemoteState remoteState;

// Any button on the page stops a running seek or sweep, as the knob does
bool webConsumeAbortPending() { return(webAbort.exchange(false)); }

// Remote output (status lines, errors) is not needed here
class NullStream : public Stream
{
public:
  int available() override { return(0); }
  int read() override { return(-1); }
  int peek() override { return(-1); }
  size_t write(uint8_t) override { return(1); }
  size_t write(const uint8_t *, size_t n) override { return(n); }
  void flush() override {}
};
static NullStream nullStream;

bool doSeek(int16_t enc, int16_t enca);    // ats-mini.ino

static InputEvent webReadInput(uint8_t &byte)
{
  if(sessionReset.exchange(false)) return(InputEvent::SessionReset);
  std::lock_guard<std::mutex> lock(inMutex);
  if(inHead == inTail) return(InputEvent::NoData);
  byte = inBuf[inTail];
  inTail = (inTail + 1) % sizeof(inBuf);
  return(InputEvent::Byte);
}

static bool webInputPending()
{
  std::lock_guard<std::mutex> lock(inMutex);
  return(inHead != inTail);
}

static void webOnEvent(AsyncWebSocket *, AsyncWebSocketClient *client, AwsEventType type,
                       void *arg, uint8_t *data, size_t len)
{
  if(type == WS_EVT_CONNECT)
  {
    sessionReset = true;
    fullFrame = true;
    defWanted = true;
    memWanted = true;
    client->setCloseClientOnQueueFull(false);
  }
  else if(type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if(!info->final || info->index || info->len != len || info->opcode != WS_TEXT) return;
    bool busy = webBusy;
    if(busy && !(len >= 6 && !memcmp(data, "!audio", 6)) && !(len >= 5 && !memcmp(data, "!mute", 5)))
    {
      webAbort = true;
      return;                                      // Only stops it
    }
    if(len >= 5 && !memcmp(data, "!seek", 5))
    {
      seekWanted = (len > 5 && data[5] == '-') ? -1 : 1;
      return;
    }
    if(len >= 6 && !memcmp(data, "!scope", 6))
    {
      scopeWanted = true;
      return;
    }
    if(len >= 7 && !memcmp(data, "!screen", 7))
    {
      screenWanted = len > 7 && data[7] == '1';
      if(screenWanted) fullFrame = true;
      return;
    }
    // "!set <cmd> <value>", "!mem t|d <slot>", "!mem s"
    if(len >= 5 && (!memcmp(data, "!set ", 5) || !memcmp(data, "!mem ", 5)))
    {
      char text[40];
      size_t n = len < sizeof(text) - 1 ? len : sizeof(text) - 1;
      memcpy(text, data, n);
      text[n] = 0;
      WebSet w;
      if(text[1] == 's')
      {
        char *end;
        w.cmd = strtoul(text + 5, &end, 10);
        w.value = strtol(end, nullptr, 10);
      }
      else
      {
        w.cmd = 0xFF00 | (uint8_t)text[5];          // Memory action letter
        w.value = len > 7 ? strtol(text + 7, nullptr, 10) : -1;
      }
      std::lock_guard<std::mutex> lock(inMutex);
      uint8_t next = (setHead + 1) % 16;
      if(next != setTail) { setQueue[setHead] = w; setHead = next; }
      return;
    }
    if(len >= 4 && !memcmp(data, "!def", 4)) { defWanted = true; memWanted = true; return; }
    if(len >= 5 && !memcmp(data, "!mute", 5))
    {
      // "!mute0" normal, "!mute4" 4 steps quieter, "!mute8" ... ("!mute1" = 8, as before)
      int8_t v = (len > 5 && data[5] >= '0' && data[5] <= '9') ? data[5] - '0' : 0;
      muteWanted = v == 1 ? 8 : v;
      return;
    }
    if(len >= 6 && !memcmp(data, "!audio", 6))
    {
      audioWanted = (len > 6 && data[6] == '1') ? 1 : 0;
      return;
    }
    // Frequency: tuned on the main task, changing band if needed
    if(len > 1 && data[0] == 'F')
    {
      uint32_t hz = 0;
      for(size_t i = 1; i < len && data[i] >= '0' && data[i] <= '9'; i++) hz = hz * 10 + (data[i] - '0');
      if(hz) tuneWanted = hz;
      return;
    }
    std::lock_guard<std::mutex> lock(inMutex);
    for(size_t i = 0; i < len; i++)
    {
      uint16_t next = (inHead + 1) % sizeof(inBuf);
      if(next == inTail) break;                    // Full: drop the rest
      inBuf[inHead] = data[i];
      inHead = next;
    }
  }
}

void webControlInit(AsyncWebServer &server, bool (*authenticated)(AsyncWebServerRequest *))
{
  ws.onEvent(webOnEvent);
  ws.handleHandshake([authenticated](AsyncWebServerRequest *request) { return(authenticated(request)); });
  server.addHandler(&ws);
  server.on("/control", HTTP_GET, [authenticated] (AsyncWebServerRequest *request) {
    if(!authenticated(request)) return request->requestAuthentication();
    request->send(200, "text/html", webControlPage);
  });
}

//
// Screen: send the rows that changed, a few kB at a time
//
#define ROW_BYTES   (DISPLAY_WIDTH * 2)
#define CHUNK_ROWS  24
#define WEB_SCREEN_AUDIO_SLACK 1600        // 0.2 s of audio waiting: screen holds back
static uint8_t *lastFrame = nullptr;                // What the browser has
static uint8_t chunk[3 + CHUNK_ROWS * ROW_BYTES];

// Run-length coding of a band of rows: (count 1-255, high byte, low byte)
// triples. Menus and the main screen are mostly flat colour, so this is
// usually 10 to 30 times smaller than raw; the waterfall is sent raw.
static size_t rleEncode(const uint8_t *src, size_t bytes, uint8_t *dst, size_t room)
{
  size_t o = 0;
  for(size_t i = 0; i < bytes; )
  {
    uint8_t hi = src[i], lo = src[i + 1];
    size_t run = 1;
    while(run < 255 && i + run * 2 < bytes && src[i + run * 2] == hi && src[i + run * 2 + 1] == lo) run++;
    if(o + 3 > room) return(0);
    dst[o++] = run;
    dst[o++] = hi;
    dst[o++] = lo;
    i += run * 2;
  }
  return(o);
}

static void webSendScreen(size_t budget)
{
  // Small bands while listening, so the audio never waits behind a big one
  int maxRows = budget == SIZE_MAX ? CHUNK_ROWS : 6;
  bool first = true;
  const uint8_t *fb = (const uint8_t *)spr.getBuffer();
  if(!fb) return;
  if(!lastFrame)
  {
    lastFrame = (uint8_t *)heap_caps_malloc(DISPLAY_HEIGHT * ROW_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if(!lastFrame) return;
    fullFrame = true;
  }
  // A new browser needs the whole screen: forget what was sent, so every row
  // differs and goes out
  if(fullFrame.exchange(false)) memset(lastFrame, 0xA5, DISPLAY_HEIGHT * ROW_BYTES);
  // Everything that changed, as long as the connection keeps up
  for(int y = 0; y < DISPLAY_HEIGHT; )
  {
    if(!memcmp(fb + y * ROW_BYTES, lastFrame + y * ROW_BYTES, ROW_BYTES)) { y++; continue; }
    int n = 1;
    while(y + n < DISPLAY_HEIGHT && n < maxRows &&
          memcmp(fb + (y + n) * ROW_BYTES, lastFrame + (y + n) * ROW_BYTES, ROW_BYTES)) n++;
    if(!ws.availableForWriteAll()) return;
    size_t raw = n * ROW_BYTES;
    size_t packed = rleEncode(fb + y * ROW_BYTES, raw, chunk + 3, raw - 1);
    chunk[0] = packed ? 0x03 : 0x01;
    chunk[1] = y;
    chunk[2] = n;
    if(!packed) memcpy(chunk + 3, fb + y * ROW_BYTES, raw);
    size_t bytes = 3 + (packed ? packed : raw);
    if(bytes > budget && !first) return;     // The rest next time
    budget = bytes > budget ? 0 : budget - bytes;
    first = false;
    ws.binaryAll(chunk, bytes);
    memcpy(lastFrame + y * ROW_BYTES, fb + y * ROW_BYTES, raw);
    y += n;
  }
}

//
// Audio: DC removed, about 100 ms per message
//
static uint32_t audioPos = 0;
static uint32_t audioSendWaits = 0;      // Times the connection was too busy
static float audioDc = 2048;

static void webSendAudio()
{
  static uint8_t msg[2 + 2 * 4096];
  int16_t *s = (int16_t *)(msg + 2);
  // Connection busy: leave the samples in the ring for the next try (they
  // used to be read and thrown away here, without being counted)
  if(!ws.availableForWriteAll()) { audioSendWaits++; return; }
  uint32_t n = audioWebRead(s, 4096, &audioPos);
  if(!n) return;
  // 8-bit mu-law (as the telephone network): half the data of 16-bit, which
  // matters when Wi-Fi is busy (test 5: the connection was full 165 times)
  uint8_t *out = msg + 2;
  for(uint32_t i = 0; i < n; i++)
  {
    audioDc += (s[i] - audioDc) * 0.002f;
    int32_t v = (int32_t)((s[i] - audioDc) * 16.0f);
    v = v > 32635 ? 32635 : v < -32635 ? -32635 : v;
    uint8_t sign = v < 0 ? 0x80 : 0;
    if(v < 0) v = -v;
    v += 0x84;
    uint8_t exp = 7;
    for(int32_t mask = 0x4000; !(v & mask) && exp; mask >>= 1) exp--;
    uint8_t mant = (v >> (exp + 3)) & 0x0F;
    out[i] = ~(sign | (exp << 4) | mant);
  }
  msg[0] = 0x04;
  msg[1] = 0x00;
  ws.binaryAll(msg, 2 + n);
}

// Audio at full processor speed (as the FT8 page): each sample is a short
// ADC read, and with Wi-Fi on there is little time to spare at 80 MHz
static uint32_t cpuSaved = 0;

static uint8_t speakerSteps = 0;      // How much quieter the radio plays while listening

// R4 test 14: while listening the receiver plays at a steady line level
// (AUDIO_LINE_VOLUME), not the user's volume, so the web sound does not
// depend on it. "Quieter" and "Quietest" go 4 or 8 steps below it.
static uint8_t quietVolume()
{
  int v = (int)AUDIO_LINE_VOLUME - speakerSteps;
  return(v < 10 ? 10 : v);
}
static void speakerRestore();

static void webAudioSet(bool on)
{
  audioWebStream(on);
  if(!on) speakerRestore();
  if(on && !cpuSaved)
  {
    cpuSaved = getCpuFrequencyMhz();
    if(cpuSaved < 240) setCpuFrequencyMhz(240);
  }
  else if(!on && cpuSaved)
  {
    if(cpuSaved < 240) setCpuFrequencyMhz(cpuSaved);
    cpuSaved = 0;
  }
}

//
// Silence the speaker while listening in the browser. Boards that take the
// decoder audio before the speaker amplifier: the amplifier is switched off.
// The V4B takes it after the amplifier, so switching it off would also
// silence the browser: there the radio's volume is lowered 8 steps instead
// and the page makes up the level.
//
enum class Spk : uint8_t { Normal, TryAmpOff, AmpOff, Quiet };
static Spk spk = Spk::Normal;
static uint32_t spkSince = 0;
static bool ampOffSilences = false;   // Learned: this board (V4B) loses the audio with the amplifier off
static const char *spkName[] = { "on", "testing", "off", "quiet" };

static bool lineOn = false;           // The receiver is at the listening level
static uint8_t lineApplied = 0, lineUserVol = 0;

static void speakerRestore()
{
  if(spk != Spk::Normal)
  {
    if(PIN_AMP_EN >= 0 && !muteOn(MUTE_MAIN, 2)) digitalWrite(PIN_AMP_EN, HIGH);
    spk = Spk::Normal;
  }
  if(lineOn) { rx.setVolume(volume); lineOn = false; }
}

// Set the receiver level; again after Vol +/- (which sets the user's volume)
static void applyLevel(uint8_t v)
{
  if(lineOn && v == lineApplied && volume == lineUserVol) return;
  rx.setVolume(v);
  lineApplied = v;
  lineUserVol = volume;
  lineOn = true;
}

static void speakerTick(bool wanted)
{
  if(!audioWebStreaming()) { speakerRestore(); return; }
  if(!wanted)
  {
    // Normal: speaker on, receiver at the listening level
    if(spk != Spk::Normal)
    {
      if(PIN_AMP_EN >= 0 && !muteOn(MUTE_MAIN, 2)) digitalWrite(PIN_AMP_EN, HIGH);
      spk = Spk::Normal;
    }
    applyLevel(AUDIO_LINE_VOLUME);
    return;
  }
  uint32_t now = millis();
  switch(spk)
  {
    case Spk::Normal:
      if(PIN_AMP_EN >= 0 && !ampOffSilences)
      {
        digitalWrite(PIN_AMP_EN, LOW);
        spk = Spk::TryAmpOff;
        spkSince = now;
        applyLevel(AUDIO_LINE_VOLUME);
      }
      else spk = Spk::Quiet;
      break;
    case Spk::TryAmpOff:
      applyLevel(AUDIO_LINE_VOLUME);
      // The level is measured over 0.5 s: give it 2 s after the switch
      if(now - spkSince < 2000) break;
      spkSince = now;
      if(audioInputLevel() >= 3) { spk = Spk::AmpOff; break; }   // Audio still there
      // no audio with the amplifier off (V4B)
      [[fallthrough]];
    case Spk::AmpOff:
      // Speaker off and the input still has audio: full listening level.
      // Keep checking: if the input goes silent, the amplifier is needed
      if(spk == Spk::AmpOff)
      {
        applyLevel(AUDIO_LINE_VOLUME);
        if(audioInputLevel() >= 3) { spkSince = now; break; }
        if(now - spkSince < 1500) break;
      }
      digitalWrite(PIN_AMP_EN, HIGH);
      // Straight to Quiet next time (no 2 s of silence at every start)
      ampOffSilences = true;
      spk = Spk::Quiet;
      break;
    case Spk::Quiet:
      applyLevel(quietVolume());
      break;
    default:
      break;
  }
}

//
// Seek in SSB and CW: the chip only seeks in AM and FM (the radio's own
// Seek moves one step there). Steps 1 kHz at a time with the sound muted
// and stops at the first point clearly above the noise measured at the
// start, or at the band edge. Any button on the page or the knob stops it.
//
static void webSeekProgress(uint32_t hz)
{
  char j[40];
  snprintf(j, sizeof(j), "{\"seekf\":%lu}", (unsigned long)hz);
  ws.textAll(j);
}

static void webSeekSsb(int dir)
{
  static uint8_t seen[200];
  int n = 0;
  uint8_t thr = 0, why = 0;
  uint32_t start = (currentFrequency / 1000) * 1000;
  webBusy = true;
  webAbort = false;
  uint32_t found = ssbSeek(dir, webSeekProgress, seen, &n, &thr, &why);
  clearStationInfo();
  identifyFrequency(currentFrequency);
  webBusy = false;
  webAbort = false;

  char j[120];
  if(found) snprintf(j, sizeof(j), "{\"note\":\"Seek: signal at %.2f kHz\"}", found / 1000.0);
  else if(why == 1) snprintf(j, sizeof(j), "{\"note\":\"Seek stopped\"}");
  else snprintf(j, sizeof(j), "{\"note\":\"Seek: nothing above the noise %s, back to the start\"}",
                why == 3 ? "up to the band edge" : "in 200 kHz");
  ws.textAll(j);

  // What it measured, lowest frequency first, with the threshold
  if(n > 200) n = 200;
  if(n)
  {
    String sc;
    sc.reserve(60 + n * 6);
    uint32_t f0 = dir > 0 ? start + 1000 : start - 1000UL * n;
    sc += "{\"scope\":1,\"seek\":1,\"f0\":"; sc += (unsigned long)f0;
    sc += ",\"st\":1000,\"fm\":0,\"thr\":"; sc += thr;
    sc += ",\"r\":[";
    for(int i = 0; i < n; i++) { if(i) sc += ','; sc += seen[dir > 0 ? i : n - 1 - i]; }
    sc += "],\"s\":[";
    for(int i = 0; i < n; i++) { if(i) sc += ','; sc += '0'; }
    sc += "]}";
    ws.textAll(sc);
  }
}

//
// Band scope: signal strength at 41 points around the frequency, swept
// with the audio muted (about 2 to 3 seconds), then back to where it was.
// Same method as the radio's own Scan, but short, and the numbers go to
// the page, which draws the graph; a click there tunes.
//
#define SCOPE_POINTS 41

static void webScope()
{
  static uint8_t r[SCOPE_POINTS], q[SCOPE_POINTS];
  const Band *band = getCurrentBand();
  uint32_t step = currentMode == FM ? 100000 : isSSB() ? 1000 : 5000;
  if(currentMode == AM && band->bandType == MW_BAND_TYPE && getCurrentStep()->step == 9000) step = 9000;
  uint32_t cur = currentFrequency;
  int64_t start = ((int64_t)(cur / step) - SCOPE_POINTS / 2) * step;
  int64_t span = (int64_t)step * (SCOPE_POINTS - 1);
  if(start + span > band->maximumFreq) start = (int64_t)band->maximumFreq - span;
  if(start < band->minimumFreq) start = band->minimumFreq;

  webBusy = true;
  webAbort = false;
  seekStop = false;
  muteOn(MUTE_TEMP, true);
  if(isSSB()) rx.setSSBBfo(0);
  int n = 0;
  for(; n < SCOPE_POINTS; n++)
  {
    uint32_t f = start + (uint32_t)n * step;
    if(f > band->maximumFreq || consumeAbortPending()) break;
    rx.setFrequencyConfirmed(freqFromHz(f, currentMode));
    rx.getCurrentReceivedSignalQuality();
    r[n] = rx.getCurrentRSSI();
    q[n] = rx.getCurrentSNR();
  }
  if(isSSB()) rx.setFrequencyConfirmed(getReceiverFrequency(cur));
  updateFrequency(cur, false);
  clearStationInfo();
  identifyFrequency(currentFrequency);
  muteOn(MUTE_TEMP, false);
  webBusy = false;
  webAbort = false;

  String j;
  j.reserve(60 + n * 8);
  j += "{\"scope\":1,\"f0\":"; j += (unsigned long)start;
  j += ",\"st\":"; j += step;
  j += ",\"fm\":"; j += currentMode == FM ? 1 : 0;
  j += ",\"r\":[";
  for(int i = 0; i < n; i++) { if(i) j += ','; j += r[i]; }
  j += "],\"s\":[";
  for(int i = 0; i < n; i++) { if(i) j += ','; j += q[i]; }
  j += "]}";
  ws.textAll(j);
}

// Text for JSON: quotes, backslashes and control characters escaped,
// bytes outside ASCII dropped (RDS uses its own character set)
static void jsonText(String &j, const char *key, const char *v)
{
  j += ",\""; j += key; j += "\":\"";
  for(; v && *v; v++)
  {
    uint8_t c = *v;
    if(c == '"' || c == '\\') { j += '\\'; j += (char)c; }
    else if(c >= 0x20 && c < 0x7F) j += (char)c;
    else if(c != 0xFF) j += ' ';
  }
  j += '"';
}

// The panel the page draws itself: sent when something on it changes,
// at most four times a second
static void webSendPanel(bool force)
{
  static String last;
  const Band *band = getCurrentBand();
  String j;
  j.reserve(200);
  j += "{\"p\":1,\"f\":"; j += (unsigned long)currentFrequency;
  j += ",\"m\":"; j += currentMode;
  jsonText(j, "band", band->bandName);
  jsonText(j, "mode", bandModeDesc[currentMode]);
  jsonText(j, "step", getCurrentStep()->desc);
  jsonText(j, "bw", getCurrentBandwidth()->desc);
  j += ",\"rssi\":"; j += rssi;
  j += ",\"snr\":"; j += snr;
  j += ",\"vol\":"; j += volume;
  j += ",\"bat\":"; j += String(batteryLastVolts(), 2);
  // R4 test 12: battery %, time left, memory slot, FM stereo, band limits, clock
  float bv = batteryLastVolts();
  j += ",\"batp\":"; j += (int)(batterySocFromVolts(bv) + 0.5f);
  j += ",\"chg\":"; j += bv > 4.3 ? 1 : 0;
  j += ",\"bmin\":"; j += batteryMinutesLeft();
  int slot = 0;
  for(int i = 0; i < getTotalMemories(); i++)
  {
    Memory m = getMemory(i);
    if(m.freq && m.freq == currentFrequency && m.mode == currentMode) { slot = i + 1; break; }
  }
  j += ",\"mem\":"; j += slot;
  j += ",\"st\":"; j += (currentMode == FM && fmStereoIdx != FM_STEREO_MONO && rx.getCurrentPilot()) ? 1 : 0;
  j += ",\"lo\":"; j += (unsigned long)band->minimumFreq;
  j += ",\"hi\":"; j += (unsigned long)band->maximumFreq;
  jsonText(j, "ps", getStationName());
  jsonText(j, "rt", getRadioText());
  j += '}';
  if(!force && j == last) return;
  last = j;
  ws.textAll(j);
}

//
// Beacons: while the radio sits on one of the five NCDXF/IARU frequencies,
// the strongest signal of each 10 second slot is reported to the page, which
// colours that beacon on its map (heard strongly, weakly, or not at all)
//
static void webBeaconTick()
{
  static int curBand = -1;
  static uint32_t curSlot = 0;
  static bool whole = false;              // On this frequency since the slot began
  static uint8_t maxR = 0, maxQ = 0;
  int band = (clockAvailable() && currentMode != FM) ? ibpBandAt(currentFrequency) : -1;
  uint32_t utc = band >= 0 ? (uint32_t)time(NULL) : 0;
  uint32_t slot = utc / 10;
  if(band != curBand || slot != curSlot)
  {
    bool next = band >= 0 && band == curBand && slot == curSlot + 1;
    if(next && whole)
    {
      char j[96];
      snprintf(j, sizeof(j), "{\"bcn\":1,\"b\":%d,\"i\":%d,\"r\":%u,\"q\":%u,\"t\":%lu}",
               curBand, ibpBeaconOn(curBand, curSlot * 10), maxR, maxQ, (unsigned long)(curSlot * 10));
      ws.textAll(j);
    }
    whole = next;                          // A slot joined part way is not reported
    curBand = band;
    curSlot = slot;
    maxR = maxQ = 0;
    return;
  }
  // Skip the first second of the slot (the previous beacon may still be on)
  if(band < 0 || utc % 10 == 0) return;
  if(rssi > maxR) maxR = rssi;
  if(snr > maxQ) maxQ = snr;
}

bool webControlActive() { return(ws.count() > 0); }

bool webControlTick()
{
  static uint32_t lastScreen = 0, lastAudio = 0, lastStatus = 0, lastCleanup = 0, lastPanel = 0;
  static uint32_t lastVals = 0;
  static uint8_t lastCount = 0;
  uint32_t now = millis();

  if(now - lastCleanup > 1000)
  {
    lastCleanup = now;
    ws.cleanupClients(3);
  }
  if(!ws.count())
  {
    webAudioSet(false);
    audioWanted = -1;
    return(false);
  }

  // Commands, at most a few per loop pass
  bool redraw = false;
  for(int i = 0; i < 8 && (i == 0 || webInputPending()); i++)
    redraw |= remoteLoop(&nullStream, &remoteState, webReadInput);

  // Tune: the current band if it covers the frequency, otherwise the
  // narrowest band that does (so 7074 from FM goes to 40M, not ALL)
  uint32_t hz = tuneWanted.exchange(0);
  if(hz)
  {
    const Band *cur = getCurrentBand();
    if(hz >= cur->minimumFreq && hz <= cur->maximumFreq) updateFrequency(hz, false);
    else
    {
      int best = -1;
      for(int i = 0; i < (int)getTotalBands(); i++)
        if(hz >= bands[i].minimumFreq && hz <= bands[i].maximumFreq &&
           (best < 0 || bands[i].maximumFreq - bands[i].minimumFreq < bands[best].maximumFreq - bands[best].minimumFreq))
          best = i;
      if(best >= 0)
      {
        bands[best].currentFreq = hz;
        selectBand(best, false);
      }
    }
    // Band plan Auto picks the mode for the segment, as when tuning by hand
    bandPlanForget();
    if(isSSB() || currentMode == AM) bandPlanAutoSideband();
    clearStationInfo();
    identifyFrequency(currentFrequency);
    prefsRequestSave(SAVE_CUR_BAND);
    redraw = true;
  }

  // Seek, as with the knob in Seek mode (SSB and CW: one step)
  int8_t sk = seekWanted.exchange(0);
  if(sk)
  {
    webBusy = true;
    webAbort = false;
    if(isSSB()) webSeekSsb(sk);
    else doSeek(sk, sk);
    webBusy = false;
    webAbort = false;
    prefsRequestSave(SAVE_CUR_BAND);
    redraw = true;
  }
  if(scopeWanted.exchange(false))
  {
    webScope();
    redraw = true;
  }

  // Settings and memories from the page
  for(int i = 0; i < 4; i++)
  {
    WebSet w;
    {
      std::lock_guard<std::mutex> lock(inMutex);
      if(setHead == setTail) break;
      w = setQueue[setTail];
      setTail = (setTail + 1) % 16;
    }
    if((w.cmd & 0xFF00) == 0xFF00)
    {
      redraw |= webMemoryAction((char)(w.cmd & 0xFF), w.value);
      memWanted = true;
    }
    else redraw |= webMenuSet(w.cmd, w.value);
    lastVals = 0;                                  // Show the result at once
  }

  int8_t a = audioWanted.exchange(-1);
  if(a >= 0) webAudioSet(a == 1);
  int8_t m = muteWanted.exchange(-1);
  if(m >= 0) speakerSteps = m;
  speakerTick(speakerSteps > 0);

  // Audio first: it cannot wait, the screen can. Up to 0.5 s per message,
  // so a busy moment is caught up at once instead of overrunning the ring.
  bool audioOn = audioWebStreaming();
  if(audioOn && now - lastAudio >= 60)
  {
    lastAudio = now;
    webSendAudio();
  }
  // While listening the screen gets what is left: at most about 20 kB/s,
  // and nothing while the audio is behind. Tuning redraws much of the
  // screen, and in test 7 that crowded out the sound (gaps of seconds).
  if(screenWanted && now - lastScreen >= (audioOn ? 150u : 50u))
  {
    lastScreen = now;
    if(!audioOn) webSendScreen(SIZE_MAX);
    else if(audioWebBacklog(audioPos) < WEB_SCREEN_AUDIO_SLACK) webSendScreen(3000);
  }
  // Settings: the list once, then the values when they change. Not while
  // listening and the audio is behind (the list is a few kB).
  if(ws.availableForWriteAll() && (!audioOn || audioWebBacklog(audioPos) < WEB_SCREEN_AUDIO_SLACK))
  {
    if(defWanted.exchange(false))
    {
      String j;
      j.reserve(6000);
      webMenuDef(j);
      ws.textAll(String("{\"def\":") + j + "}");
      lastVals = 0;
    }
    else if(memWanted.exchange(false))
    {
      String j;
      j.reserve(2000);
      webMemoriesJson(j);
      ws.textAll(j);
    }
    else if(!lastVals || now - lastVals >= 300)
    {
      static String last;
      bool force = !lastVals;
      lastVals = now;
      String j;
      j.reserve(900);
      webMenuVals(j);
      if(force || j != last) { last = j; ws.textAll(j); }
    }
  }
  webBeaconTick();
  if(now - lastPanel >= 250)
  {
    lastPanel = now;
    // A new browser gets the panel at once
    bool force = ws.count() != lastCount;
    lastCount = ws.count();
    if(ws.availableForWriteAll()) webSendPanel(force);
  }
  if(now - lastStatus >= 1000)
  {
    lastStatus = now;
    char json[300];
    uint32_t avgUs, maxUs, late, caughtUp, overrun;
    audioTimedStats(&avgUs, &maxUs, &late, &caughtUp, &overrun);
    snprintf(json, sizeof(json),
      "{\"audio\":%d,\"adcErrors\":%lu,\"restarts\":%lu,\"level\":%u,\"clients\":%u,\"freq\":%lu,"
      "\"readUs\":%lu,\"readMax\":%lu,\"late\":%lu,\"cpu\":%lu,\"spk\":\"%s\",\"caughtUp\":%lu,\"overrun\":%lu,\"waits\":%lu}",
      audioWebStreaming() ? 1 : 0, (unsigned long)audioAdcErrors(), (unsigned long)audioTimingRestarts(),
      audioInputLevel(), (unsigned)ws.count(), (unsigned long)currentFrequency,
      (unsigned long)avgUs, (unsigned long)maxUs, (unsigned long)late, (unsigned long)getCpuFrequencyMhz(), spkName[(int)spk],
      (unsigned long)caughtUp, (unsigned long)overrun, (unsigned long)audioSendWaits);
    ws.textAll(json);
  }
  return(redraw);
}
