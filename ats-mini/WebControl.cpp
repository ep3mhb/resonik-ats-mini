//
// RESONIK R4 (test): Web Control. The radio's screen live in the browser,
// with buttons for the knob, click, press, volume, band, mode, step and
// filter, a frequency box, and (V4B boards, experimental) the audio.
//
// One WebSocket at /ws carries everything:
//   browser -> radio   text: ad hoc remote commands, the same as over USB
//                      ("R", "r", "e", "E", "V", "v", "B", "b", "M", ...,
//                      "F7074000\n"), run by remoteLoop() on the main task;
//                      "!audio1" / "!audio0" switch the audio stream
//   radio -> browser   binary 0x01 y n + n rows of RGB565 (high byte first):
//                      the rows of the screen that changed since last sent;
//                      binary 0x02 0x00 + 16-bit samples at 8 kHz: audio;
//                      text {...}: status once a second
//
#include "Common.h"
#include "Menu.h"
#include "Storage.h"
#include "Remote.h"
#include "CwAudio.h"
#include "WebControl.h"
#include "PageControl.h"
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
static RemoteState remoteState;

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
    client->setCloseClientOnQueueFull(false);
  }
  else if(type == WS_EVT_DATA)
  {
    AwsFrameInfo *info = (AwsFrameInfo *)arg;
    if(!info->final || info->index || info->len != len || info->opcode != WS_TEXT) return;
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
#define CHUNK_ROWS  12
static uint8_t *lastFrame = nullptr;                // What the browser has
static uint8_t chunk[3 + CHUNK_ROWS * ROW_BYTES];

static void webSendScreen()
{
  const uint8_t *fb = (const uint8_t *)spr.getBuffer();
  if(!fb) return;
  if(!lastFrame)
  {
    lastFrame = (uint8_t *)heap_caps_malloc(DISPLAY_HEIGHT * ROW_BYTES, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    if(!lastFrame) return;
    fullFrame = true;
  }
  // A new browser needs the whole screen: forget what was sent, so every row
  // differs and goes out over the next few ticks
  if(fullFrame.exchange(false)) memset(lastFrame, 0xA5, DISPLAY_HEIGHT * ROW_BYTES);
  int sent = 0;
  for(int y = 0; y < DISPLAY_HEIGHT && sent < 3; )
  {
    if(!memcmp(fb + y * ROW_BYTES, lastFrame + y * ROW_BYTES, ROW_BYTES)) { y++; continue; }
    int n = 1;
    while(y + n < DISPLAY_HEIGHT && n < CHUNK_ROWS &&
          memcmp(fb + (y + n) * ROW_BYTES, lastFrame + (y + n) * ROW_BYTES, ROW_BYTES)) n++;
    if(!ws.availableForWriteAll()) return;
    chunk[0] = 0x01;
    chunk[1] = y;
    chunk[2] = n;
    memcpy(chunk + 3, fb + y * ROW_BYTES, n * ROW_BYTES);
    ws.binaryAll(chunk, 3 + n * ROW_BYTES);
    memcpy(lastFrame + y * ROW_BYTES, fb + y * ROW_BYTES, n * ROW_BYTES);
    sent++;
    y += n;
  }
}

//
// Audio: DC removed, about 100 ms per message
//
static uint32_t audioPos = 0;
static float audioDc = 2048;

static void webSendAudio()
{
  static uint8_t msg[2 + 2 * 1024];
  int16_t *s = (int16_t *)(msg + 2);
  uint32_t n = audioWebRead(s, 1024, &audioPos);
  if(!n || !ws.availableForWriteAll()) return;
  for(uint32_t i = 0; i < n; i++)
  {
    audioDc += (s[i] - audioDc) * 0.002f;
    int32_t v = (int32_t)((s[i] - audioDc) * 16.0f);
    s[i] = v > 32767 ? 32767 : v < -32768 ? -32768 : v;
  }
  msg[0] = 0x02;
  msg[1] = 0x00;
  ws.binaryAll(msg, 2 + n * 2);
}

bool webControlTick()
{
  static uint32_t lastScreen = 0, lastAudio = 0, lastStatus = 0, lastCleanup = 0;
  uint32_t now = millis();

  if(now - lastCleanup > 1000)
  {
    lastCleanup = now;
    ws.cleanupClients(3);
  }
  if(!ws.count())
  {
    if(audioWebStreaming() || audioWanted == 1) audioWebStream(false);
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

  int8_t a = audioWanted.exchange(-1);
  if(a >= 0) audioWebStream(a == 1);

  if(now - lastScreen >= 120)
  {
    lastScreen = now;
    webSendScreen();
  }
  if(audioWebStreaming() && now - lastAudio >= 100)
  {
    lastAudio = now;
    webSendAudio();
  }
  if(now - lastStatus >= 1000)
  {
    lastStatus = now;
    char json[160];
    snprintf(json, sizeof(json),
      "{\"audio\":%d,\"adcErrors\":%lu,\"restarts\":%lu,\"level\":%u,\"clients\":%u,\"freq\":%lu}",
      audioWebStreaming() ? 1 : 0, (unsigned long)audioAdcErrors(), (unsigned long)audioTimingRestarts(),
      audioInputLevel(), (unsigned)ws.count(), (unsigned long)currentFrequency);
    ws.textAll(json);
  }
  return(redraw);
}
