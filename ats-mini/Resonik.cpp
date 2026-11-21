//
// RESONIK Phase 2: beacon tracker, memory scanner, signal logger,
// propagation data.
//
#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"
#include "Memories.h"
#include "Storage.h"
#include "Resonik.h"
#include "Beacons.h"

#include <time.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>

//
// Tune any frequency and mode, switching band when needed. Prefers the
// current band, then the narrowest band that contains the frequency.
//
bool tuneToFrequency(uint32_t hz, uint8_t mode)
{
  Memory mem = {};
  mem.freq = hz;
  mem.mode = mode;

  mem.band = bandIdx;
  if(!isMemoryInBand(&bands[bandIdx], &mem))
  {
    int best = -1;
    uint32_t bestSpan = UINT32_MAX;
    for(int i = 0 ; i < getTotalBands() ; i++)
    {
      if(!isMemoryInBand(&bands[i], &mem)) continue;
      uint32_t span = bands[i].maximumFreq - bands[i].minimumFreq;
      if(span < bestSpan) { best = i; bestSpan = span; }
    }
    if(best < 0) return(false);
    mem.band = best;
  }

  if(mem.band == bandIdx && mode == currentMode)
  {
    if(!updateFrequency(hz, false)) return(false);
    clearStationInfo();
    identifyFrequency(currentFrequency);
    prefsRequestSave(SAVE_CUR_BAND);
    return(true);
  }
  return(tuneToMemory(&mem));
}

//
// NCDXF/IARU beacon tracker
//
int8_t beaconFollowIdx = -1;
static uint32_t beaconFollowFreq = 0;
static int beaconLastSlot = -1;
static uint32_t beaconHoldUntil = 0;   // R5: no band change before this time

// R5: where the radio was when following began. Stop on the page, and Off or
// Names only in the menu, take it back there: starting a follow now moves the
// radio at once, so stopping it has to be seen and heard too (Mohammad,
// 21 Nov: "stop not working" - the radio stayed on 14.100 MHz).
static bool beaconHomeSet = false;
static uint8_t beaconHomeBand, beaconHomeMode;
static uint32_t beaconHomeFreq;
static uint32_t beaconReturnAt = 0;    // 0 = no return waiting

// Following ends where the radio is: tuned away, a scan started
static void beaconDrop()
{
  beaconFollowIdx = -1;
  beaconHomeSet = false;
  beaconReturnAt = 0;
}

void beaconEndHere() { beaconDrop(); }   // For pages that take the receiver over

uint8_t beaconNames = 1;

// R4: "Off" now also hides the beacon names (GitHub: Off did not stop them);
// "Names only" shows the beacon due in each slot without following one
const char *const beaconMenuLabels[BEACON_MENU_COUNT] =
{
  "Off", "Names only", "4U1UN", "VE8AT", "W6WX", "KH6RS", "ZL6B", "VK6RBP", "JA2IGY", "RR9O", "VR2B",
  "4S7B", "ZS6DN", "5Z4B", "4X6TU", "OH2B", "CS3B", "LU4AA", "OA4B", "YV5B",
};

const char *beaconLabel(uint32_t freq)
{
  static char label[40];
  int band = ibpBandAt(freq);
  if(band < 0 || !clockAvailable() || (!beaconNames && beaconFollowIdx < 0)) return(nullptr);

  uint32_t utc = (uint32_t)time(NULL);
  const IbpBeacon &b = ibpBeacons[ibpBeaconOn(band, utc)];
  snprintf(label, sizeof(label), "Beacon %s %s %ds", b.call, b.place, ibpSecondsLeft(utc));
  return(label);
}

int beaconMenuIdx() { return(beaconFollowIdx >= 0 ? beaconFollowIdx + 2 : beaconNames ? 1 : 0); }

void beaconMenuSet(int idx, bool now)
{
  if(idx <= 1)
  {
    beaconNames = idx == 1;
    beaconFollow(-1, now ? 0 : 1200);
    prefsRequestSave(SAVE_SETTINGS);
  }
  // Turning through the list on the radio passes every name: the band
  // changes only for the one the knob rests on. The page's Start acts at once.
  else beaconFollow(idx - 2, now ? 0 : 1200);
  // Take the name off (or put it on) at once
  clearStationInfo();
  identifyFrequency(currentFrequency);
}

// R5: what ends or refuses a follow is said on the radio and on the Web
// Control page (Mohammad, 21 Nov: Start on the page, and nothing was seen)
static void beaconSay(const char *line1, const char *line2 = nullptr)
{
  statusShow(line1, line2);
  webControlNote(line1, line2);
}

void beaconFollow(int8_t idx, uint16_t holdMs)
{
  beaconFollowIdx = idx < 0 || idx > 17 ? -1 : idx;
  // R5: where the radio is when following begins. Tuning away from there,
  // or changing band, stops the follow also while it waits for the beacon's
  // turn (up to 130 s of every 180). It used to start from 0, which the
  // tuned-away check below ignores: the radio then left FM, or any other
  // band the user had gone to, when the beacon came on (Mohammad, 21 Nov).
  beaconFollowFreq = beaconFollowIdx >= 0 ? currentFrequency : 0;
  beaconLastSlot = -1;
  beaconHoldUntil = millis() + holdMs;
  if(beaconFollowIdx >= 0 && !clockAvailable())
  {
    beaconSay("Beacons need the clock set", "Wi-Fi Sync, RDS or Date/Time");
    beaconFollowIdx = -1;
    return;
  }

  if(beaconFollowIdx >= 0)
  {
    // The place to go back to; kept when one beacon is changed for another
    if(!beaconHomeSet)
    {
      beaconHomeSet = true;
      beaconHomeBand = bandIdx;
      beaconHomeMode = currentMode;
      beaconHomeFreq = currentFrequency;
    }
    beaconReturnAt = 0;
  }
  // Stopped: back to that place, after the same pause as before a band change
  else if(beaconHomeSet) beaconReturnAt = (millis() + holdMs) | 1;
}

// Called from the main loop. Returns true when the screen should be redrawn.
bool beaconTick()
{
  // R5: following was stopped: back to where the radio was before it
  if(beaconReturnAt && beaconFollowIdx < 0 && (int32_t)(millis() - beaconReturnAt) >= 0)
  {
    bool go = beaconHomeSet;
    beaconDrop();
    if(go)
    {
      Memory home = {};
      home.freq = beaconHomeFreq;
      home.mode = beaconHomeMode;
      home.band = beaconHomeBand;
      bool moved = bandIdx != beaconHomeBand || currentMode != beaconHomeMode || currentFrequency != beaconHomeFreq;
      if(moved) tuneToMemory(&home);
      clearStationInfo();
      identifyFrequency(currentFrequency);
      statusShow("Beacon follow off");
      webControlNote("Beacon follow off", moved ? "the radio is back where it was" : nullptr);
      return(true);
    }
  }

  bool onBeacon = ibpBandAt(currentFrequency) >= 0 && currentMode != FM && (beaconNames || beaconFollowIdx >= 0);
  if(beaconFollowIdx < 0 && !onBeacon) return(false);
  if(!clockAvailable()) return(false);

  uint32_t utc = (uint32_t)time(NULL);
  int slot = (int)((utc % 180) / 10);

  // Refresh the beacon name once a second while on a beacon frequency
  static uint32_t lastSecond = 0;
  bool changed = false;
  if(onBeacon && utc != lastSecond)
  {
    lastSecond = utc;
    changed = identifyFrequency(currentFrequency, true);
  }

  if(beaconFollowIdx < 0) return(changed);

  // Tuned away by hand, or another band or FM chosen: stop following
  if(currentFrequency != beaconFollowFreq)
  {
    beaconSay("Beacon follow off");
    beaconDrop();
    return(true);
  }

  if((int32_t)(millis() - beaconHoldUntil) < 0) return(changed);
  if(slot == beaconLastSlot) return(changed);
  beaconLastSlot = slot;

  // R5: while its beacon is silent (130 s of every 180) the radio waits on
  // 14.100 MHz, where the next run begins. Starting a follow is then seen
  // and heard at once, and the first of its five slots is not cut short by
  // the band change from 28.200 MHz. It used to stay where it was.
  int band = ibpBandOf(beaconFollowIdx, utc);
  uint32_t hz = ibpFreqs[band < 0 ? 0 : band];
  const IbpBeacon &b = ibpBeacons[beaconFollowIdx];
  if((currentFrequency != hz || currentMode != CW) && !tuneToFrequency(hz, CW))
  {
    beaconSay("No band for", "the beacon frequency");
    beaconDrop();
    return(true);
  }
  beaconFollowFreq = currentFrequency;

  if(band < 0)
  {
    char line[40];
    snprintf(line, sizeof(line), "%s back in %d s", b.call, ibpSecondsToNext(beaconFollowIdx, utc));
    statusShow("Following beacon", line, 3000);
  }
  return(true);
}

//
// Memory scanner: steps through the stored memories and holds on any with
// a signal; scanning resumes 4 seconds after the signal goes. Any encoder
// action stops it.
//
enum class MemScan : uint8_t { Off, Tune, Listen, Hold };
static MemScan memScanState = MemScan::Off;
static uint32_t memScanTimer, memScanLastHit;
static int memScanSlot = -1;

#define MSCAN_SETTLE_MS  700
#define MSCAN_RESUME_MS 4000

bool memScanActive() { return(memScanState != MemScan::Off); }

void memScanStart()
{
  int filled = 0;
  for(int i = 0 ; i < getTotalMemories() ; i++) if(getMemory(i).freq) filled++;
  if(filled < 2)
  {
    statusShow("Memory scan", "Store two or more memories first");
    return;
  }
  memScanSlot = memoryIdx;
  memScanState = MemScan::Tune;
  memScanTimer = millis();
  beaconDrop();
}

void memScanStop(const char *why)
{
  if(memScanState == MemScan::Off) return;
  memScanState = MemScan::Off;
  statusShow(why ? why : "Memory scan stopped");
}

static bool memScanHit()
{
  rx.getCurrentReceivedSignalQuality();
  uint8_t r = rx.getCurrentRSSI();
  uint8_t s = rx.getCurrentSNR();
  uint8_t sq = currentSquelch[currentMode];
  // The mode's squelch setting decides when it is on; otherwise SNR >= 6 dB
  if(sq & 0x7f) return(((sq & 0x80) ? s : r) >= (sq & 0x7f));
  return(s >= 6);
}

bool memScanTick()
{
  if(memScanState == MemScan::Off) return(false);
  uint32_t now = millis();

  switch(memScanState)
  {
    case MemScan::Tune:
    {
      if(now < memScanTimer) return(false);
      for(int n = 0 ; n < getTotalMemories() ; n++)
      {
        memScanSlot = (memScanSlot + 1) % getTotalMemories();
        Memory m = getMemory(memScanSlot);
        if(!m.freq) continue;
        if(!tuneToMemory(&m)) continue;
        memoryIdx = memScanSlot;
        char line[32];
        snprintf(line, sizeof(line), "M%02d %.10s", memScanSlot + 1, m.name);
        statusShow("Memory scan", line, 0);
        memScanState = MemScan::Listen;
        memScanTimer = now + MSCAN_SETTLE_MS;
        return(true);
      }
      memScanStop("No usable memories");
      return(true);
    }

    case MemScan::Listen:
      if(now < memScanTimer) return(false);
      if(memScanHit())
      {
        char line[32];
        Memory m = getMemory(memScanSlot);
        snprintf(line, sizeof(line), "Hold M%02d %.10s", memScanSlot + 1, m.name);
        statusShow("Memory scan", line, 0);
        memScanState = MemScan::Hold;
        memScanLastHit = now;
        memScanTimer = now + 500;
        return(true);
      }
      memScanState = MemScan::Tune;
      memScanTimer = now;
      return(false);

    case MemScan::Hold:
      if(now < memScanTimer) return(false);
      memScanTimer = now + 500;
      if(memScanHit()) memScanLastHit = now;
      else if(now - memScanLastHit > MSCAN_RESUME_MS)
      {
        memScanState = MemScan::Tune;
        memScanTimer = now;
      }
      return(false);

    default:
      return(false);
  }
}

//
// Band scanner (R3): steps through the band at the tuning step and holds on
// any signal (the same test as the memory scanner: the mode's squelch, or SNR
// of 6 dB or more), wrapping at the band edge. In SSB and CW it moves on 4 s
// after the signal goes. Broadcast stations (AM, FM) never go quiet, so there
// it moves on after 5 s, and in FM it skips the next 200 kHz so a strong
// station does not stop it again on its own sidebands. Turning the knob sets
// the direction (right up, left down) and moves on at once; a click stops it.
//
enum class BandScan : uint8_t { Off, Listen, Hold };
static BandScan bandScanState = BandScan::Off;
static uint32_t bandScanTimer, bandScanLastHit, bandScanHoldStart;
static int8_t bandScanDir = 1;
static uint8_t bandScanSkip = 0;          // Steps to pass without stopping

#define BSCAN_SETTLE_FM   150
#define BSCAN_SETTLE_AM   120
#define BSCAN_HOLD_BC    5000            // AM and FM: time on each station
#define BSCAN_SKIP_FM       2            // FM: steps passed after a station

bool bandScanActive() { return(bandScanState != BandScan::Off); }

// Step between frequencies: the tuning step, at least 1 kHz in SSB, 500 Hz
// in CW and 100 kHz in FM, so a pass over a band takes seconds, not minutes
static uint32_t bandScanStep()
{
  uint32_t step = getCurrentStep()->step;
  if(currentMode == FM && step < 100000) step = 100000;
  else if(currentMode == CW && step < 500) step = 500;
  else if(isSSB() && currentMode != CW && step < 1000) step = 1000;
  return(step);
}

static void bandScanShow(const char *what)
{
  char line[40];
  const char *dir = bandScanDir > 0 ? "Up" : "Down";
  if(!what) what = dir;
  if(currentMode == FM) snprintf(line, sizeof(line), "%s %lu.%02lu MHz", what,
                                 (unsigned long)(currentFrequency / 1000000), (unsigned long)(currentFrequency / 10000 % 100));
  else snprintf(line, sizeof(line), "%s %lu.%lu kHz", what,
                (unsigned long)(currentFrequency / 1000), (unsigned long)(currentFrequency % 1000 / 100));
  statusShow("Scanner  (turn: up/down, click: stop)", line, 0);
}

static void bandScanNext(uint32_t now)
{
  bandScanState = BandScan::Listen;
  updateFrequency((int64_t)currentFrequency + bandScanDir * (int64_t)bandScanStep(), true);
  bandScanTimer = now + (currentMode == FM ? BSCAN_SETTLE_FM : BSCAN_SETTLE_AM);
  bandScanShow(nullptr);
}

void bandScanStart()
{
  memScanStop();
  beaconDrop();
  bandScanDir = 1;
  bandScanSkip = 0;
  bandScanState = BandScan::Listen;
  bandScanTimer = millis() + (currentMode == FM ? BSCAN_SETTLE_FM : BSCAN_SETTLE_AM);
  bandScanShow(nullptr);
}

// Knob while scanning: set the direction and go on at once
void bandScanTurn(int16_t enc)
{
  if(bandScanState == BandScan::Off || !enc) return;
  bandScanDir = enc > 0 ? 1 : -1;
  // Leaving a station: do not stop on its sidebands in FM
  bandScanSkip = (bandScanState == BandScan::Hold && currentMode == FM) ? BSCAN_SKIP_FM : 0;
  bandScanNext(millis());
}

void bandScanStop(const char *why)
{
  if(bandScanState == BandScan::Off) return;
  bandScanState = BandScan::Off;
  statusShow(why ? why : "Scanner stopped");
  prefsRequestSave(SAVE_CUR_BAND);
}

bool bandScanTick()
{
  if(bandScanState == BandScan::Off) return(false);
  uint32_t now = millis();
  if((int32_t)(now - bandScanTimer) < 0) return(false);

  switch(bandScanState)
  {
    case BandScan::Listen:
      if(bandScanSkip) bandScanSkip--;
      else if(memScanHit())
      {
        bandScanState = BandScan::Hold;
        bandScanLastHit = bandScanHoldStart = now;
        bandScanTimer = now + 500;
        bandScanShow("Hold");
        return(true);
      }
      bandScanNext(now);
      return(true);

    case BandScan::Hold:
    {
      bandScanTimer = now + 500;
      bool broadcast = currentMode == FM || currentMode == AM;
      if(memScanHit()) bandScanLastHit = now;
      if(broadcast ? now - bandScanHoldStart >= BSCAN_HOLD_BC : now - bandScanLastHit > MSCAN_RESUME_MS)
      {
        bandScanSkip = currentMode == FM ? BSCAN_SKIP_FM : 0;
        bandScanNext(now);
        return(true);
      }
      return(false);
    }

    default:
      return(false);
  }
}

//
// Signal logger: one CSV line per interval to the flash file system,
// downloadable from the web page (/log.csv). Stops at LOG_MAX_BYTES.
//
uint8_t loggerIdx = 0;
const char *const loggerDesc[5] = { "Off", "10 s", "1 min", "5 min", "Clear log" };
static const uint32_t loggerIntervals[4] = { 0, 10000, 60000, 300000 };
#define LOG_MAX_BYTES (1024UL * 1024UL)

size_t loggerSize()
{
  fs::File f = LittleFS.open(LOG_PATH, "r");
  if(!f) return(0);
  size_t s = f.size();
  f.close();
  return(s);
}

void loggerClear()
{
  LittleFS.remove(LOG_PATH);
}

bool loggerTick()
{
  static uint32_t last = 0;
  if(!loggerIdx || loggerIdx >= LOG_CLEAR) return(false);
  uint32_t now = millis();
  if(last && now - last < loggerIntervals[loggerIdx]) return(false);
  last = now;

  bool isNew = !LittleFS.exists(LOG_PATH);
  if(!isNew && loggerSize() >= LOG_MAX_BYTES)
  {
    loggerIdx = 0;
    prefsRequestSave(SAVE_SETTINGS);
    statusShow("Signal log full", "Download it, then clear it");
    return(true);
  }

  fs::File f = LittleFS.open(LOG_PATH, "a");
  if(!f) return(false);
  if(isNew) f.print("utc,frequency_hz,mode,band,rssi_dbuv,snr_db,station\r\n");

  char when[24] = "";
  if(clockAvailable())
  {
    time_t t = time(NULL);
    struct tm tm;
    gmtime_r(&t, &tm);
    strftime(when, sizeof(when), "%Y-%m-%dT%H:%M:%SZ", &tm);
  }
  else snprintf(when, sizeof(when), "+%lus", (unsigned long)(now / 1000));

  // Station name without commas or quotes, so the CSV stays simple
  char station[48];
  const char *name = getStationName();
  if((uint8_t)*name == 0xFF) name++;
  size_t j = 0;
  for(const char *p = name ; *p && j < sizeof(station) - 1 ; p++)
    station[j++] = (*p == ',' || *p == '"' || *p < ' ') ? ' ' : *p;
  station[j] = '\0';

  f.printf("%s,%lu,%s,%s,%u,%u,%s\r\n", when, (unsigned long)currentFrequency,
           bandModeDesc[currentMode], getCurrentBand()->bandName, rssi, snr, station);
  f.close();
  return(false);
}

//
// Propagation data (hamqsl.com, by N0NBH)
//
extern const uint8_t caBundleStart[] asm("_binary_x509_crt_bundle_start");
extern const uint8_t caBundleEnd[] asm("_binary_x509_crt_bundle_end");

static PropData propData = {};
static uint32_t propFetched = 0;   // millis() of the last successful fetch

const PropData *propagationData() { return(&propData); }

bool propagationFetch()
{
  if(getWiFiStatus() < 2) return(false);
  // At most every 15 minutes; the feed itself updates every hour or so
  if(propData.valid && millis() - propFetched < 15UL * 60UL * 1000UL) return(true);

  NetworkClientSecure client;
  client.setCACertBundle(caBundleStart, caBundleEnd - caBundleStart);
  HTTPClient http;
  http.setTimeout(6000);
  http.setConnectTimeout(6000);
  if(!http.begin(client, PROP_URL)) return(false);
  http.setUserAgent(getUserAgent());
  int code = http.GET();
  bool ok = false;
  if(code == HTTP_CODE_OK)
  {
    String body = http.getString();
    PropData d;
    if(propParse(body.c_str(), &d))
    {
      propData = d;
      propFetched = millis();
      ok = true;
    }
  }
  http.end();
  return(ok);
}

static uint16_t propColor(PropCond c)
{
  switch(c)
  {
    case PROP_GOOD: return(0x07E0);
    case PROP_FAIR: return(0xFFE0);
    case PROP_POOR: return(0xF800);
    default:        return(TH.text_muted);
  }
}

static const char *propText(PropCond c)
{
  switch(c)
  {
    case PROP_GOOD: return("Good");
    case PROP_FAIR: return("Fair");
    case PROP_POOR: return("Poor");
    default:        return("-");
  }
}

void drawPropagation()
{
  const PropData &d = propData;
  char text[48];

  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(TH.text);
  spr.drawString("Propagation", 4, 4, FONT_SMALL);
  spr.setTextColor(TH.text_muted);
  spr.setTextDatum(TR_DATUM);
  spr.drawString(d.valid ? d.updated : "", 316, 4, FONT_SMALL);
  spr.drawLine(0, 22, 319, 22, TH.text_muted);

  if(!d.valid)
  {
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(TH.text);
    spr.drawString("No data yet.", 160, 70, FONT_SMALL);
    spr.drawString("Needs the internet: Wi-Fi", 160, 92, FONT_SMALL);
    spr.drawString("set to Connect or Sync Only.", 160, 110, FONT_SMALL);
    spr.pushSprite(0, 0);
    return;
  }

  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(TH.text);
  snprintf(text, sizeof(text), "SFI %d   SSN %d   A %d   K %d", d.sfi, d.ssn, d.aIndex, d.kIndex);
  spr.drawString(text, 4, 28, FONT_SMALL);
  spr.setTextColor(TH.text_muted);
  snprintf(text, sizeof(text), "X-ray %s  Geomag %s", d.xray, d.geomag);
  spr.drawString(text, 4, 46, FONT_SMALL);

  // Band table: day and night conditions
  spr.setTextColor(TH.text_muted);
  spr.drawString("Band", 20, 70, FONT_SMALL);
  spr.drawString("Day", 150, 70, FONT_SMALL);
  spr.drawString("Night", 230, 70, FONT_SMALL);
  for(int i = 0 ; i < 4 ; i++)
  {
    int y = 88 + i * 18;
    spr.setTextColor(TH.text);
    spr.drawString(propBandNames[i], 20, y, FONT_SMALL);
    spr.setTextColor(propColor(d.day[i]));
    spr.drawString(propText(d.day[i]), 150, y, FONT_SMALL);
    spr.setTextColor(propColor(d.night[i]));
    spr.drawString(propText(d.night[i]), 230, y, FONT_SMALL);
  }

  spr.setTextColor(TH.text_muted);
  spr.setTextDatum(BR_DATUM);
  spr.drawString("hamqsl.com (N0NBH)", 316, 168, FONT_SMALL);
  spr.pushSprite(0, 0);
}

//
// Tune anywhere, changing band if needed (number pad, Band View, the page)
//
bool resonikTuneHz(uint32_t hz)
{
  if(updateFrequency(hz, false))
  {
    clearStationInfo();
    identifyFrequency(currentFrequency);
    prefsRequestSave(SAVE_CUR_BAND);
    return(true);
  }
  // Another band: FM broadcast range in FM, otherwise keep the mode (AM if coming from FM)
  uint8_t mode = (hz >= 64000000 && hz <= 108000000) ? FM : (currentMode == FM ? AM : currentMode);
  return(tuneToFrequency(hz, mode));
}
