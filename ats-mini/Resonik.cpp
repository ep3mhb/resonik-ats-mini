//
// RESONIK Phase 2: beacon tracker, memory scanner, signal logger,
// propagation data and Kenwood CAT.
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
#include "CatKenwood.h"

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

const char *const beaconMenuLabels[19] =
{
  "Off", "4U1UN", "VE8AT", "W6WX", "KH6RS", "ZL6B", "VK6RBP", "JA2IGY", "RR9O", "VR2B",
  "4S7B", "ZS6DN", "5Z4B", "4X6TU", "OH2B", "CS3B", "LU4AA", "OA4B", "YV5B",
};

const char *beaconLabel(uint32_t freq)
{
  static char label[40];
  int band = ibpBandAt(freq);
  if(band < 0 || !clockAvailable()) return(nullptr);

  uint32_t utc = (uint32_t)time(NULL);
  const IbpBeacon &b = ibpBeacons[ibpBeaconOn(band, utc)];
  snprintf(label, sizeof(label), "Beacon %s %s %ds", b.call, b.place, ibpSecondsLeft(utc));
  return(label);
}

void beaconFollow(int8_t idx)
{
  beaconFollowIdx = idx < 0 || idx > 17 ? -1 : idx;
  beaconFollowFreq = 0;
  beaconLastSlot = -1;
  if(beaconFollowIdx >= 0 && !clockAvailable())
  {
    statusShow("Beacons need the clock set", "Wi-Fi Sync, RDS or Date/Time");
    beaconFollowIdx = -1;
  }
}

// Called from the main loop. Returns true when the screen should be redrawn.
bool beaconTick()
{
  bool onBeacon = ibpBandAt(currentFrequency) >= 0 && currentMode != FM;
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

  // Tuned away by hand: stop following
  if(beaconFollowFreq && currentFrequency != beaconFollowFreq)
  {
    statusShow("Beacon follow off");
    beaconFollowIdx = -1;
    return(true);
  }

  if(slot == beaconLastSlot) return(changed);
  beaconLastSlot = slot;

  int band = ibpBandOf(beaconFollowIdx, utc);
  const IbpBeacon &b = ibpBeacons[beaconFollowIdx];
  if(band < 0)
  {
    char line[40];
    snprintf(line, sizeof(line), "%s back in %d s", b.call, ibpSecondsToNext(beaconFollowIdx, utc));
    statusShow("Following beacon", line, 3000);
    return(true);
  }

  if(tuneToFrequency(ibpFreqs[band], CW))
    beaconFollowFreq = currentFrequency;
  else
  {
    statusShow("No band for", "the beacon frequency");
    beaconFollowIdx = -1;
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
  beaconFollowIdx = -1;
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
        snprintf(line, sizeof(line), "M%02d %s", memScanSlot + 1, m.name[0] ? m.name : "");
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
        snprintf(line, sizeof(line), "Hold M%02d %s", memScanSlot + 1, m.name);
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
// Band scanner (R3): steps up through the band at the tuning step and holds
// on any signal (the same test as the memory scanner: the mode's squelch, or
// SNR of 6 dB or more); it moves on 4 seconds after the signal goes and wraps
// at the band edge. Any encoder action or click stops it on the frequency.
//
enum class BandScan : uint8_t { Off, Listen, Hold };
static BandScan bandScanState = BandScan::Off;
static uint32_t bandScanTimer, bandScanLastHit;

#define BSCAN_SETTLE_FM   150
#define BSCAN_SETTLE_AM   120

bool bandScanActive() { return(bandScanState != BandScan::Off); }

// Step between frequencies: the tuning step, at least 1 kHz in SSB and
// 500 Hz in CW so a pass over a ham band takes seconds, not minutes
static uint32_t bandScanStep()
{
  uint32_t step = getCurrentStep()->step;
  if(currentMode == CW && step < 500) step = 500;
  else if(isSSB() && currentMode != CW && step < 1000) step = 1000;
  return(step);
}

static void bandScanShow(const char *what)
{
  char line[32];
  if(currentMode == FM) snprintf(line, sizeof(line), "%s %lu.%02lu MHz", what,
                                 (unsigned long)(currentFrequency / 1000000), (unsigned long)(currentFrequency / 10000 % 100));
  else snprintf(line, sizeof(line), "%s %lu.%lu kHz", what,
                (unsigned long)(currentFrequency / 1000), (unsigned long)(currentFrequency % 1000 / 100));
  statusShow("Scanner", line, 0);
}

void bandScanStart()
{
  memScanStop();
  beaconFollowIdx = -1;
  bandScanState = BandScan::Listen;
  bandScanTimer = millis() + (currentMode == FM ? BSCAN_SETTLE_FM : BSCAN_SETTLE_AM);
  bandScanShow("Up");
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
      if(memScanHit())
      {
        bandScanState = BandScan::Hold;
        bandScanLastHit = now;
        bandScanTimer = now + 500;
        bandScanShow("Hold");
        return(true);
      }
      // Next frequency up, wrapping at the top of the band
      updateFrequency((int64_t)currentFrequency + bandScanStep(), true);
      bandScanTimer = now + (currentMode == FM ? BSCAN_SETTLE_FM : BSCAN_SETTLE_AM);
      return(true);

    case BandScan::Hold:
      bandScanTimer = now + 500;
      if(memScanHit()) bandScanLastHit = now;
      else if(now - bandScanLastHit > MSCAN_RESUME_MS)
      {
        bandScanState = BandScan::Listen;
        bandScanShow("Up");
        updateFrequency((int64_t)currentFrequency + bandScanStep(), true);
        bandScanTimer = now + (currentMode == FM ? BSCAN_SETTLE_FM : BSCAN_SETTLE_AM);
        return(true);
      }
      return(false);

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
// Kenwood TS-480 CAT on the USB port (Settings > USB Port > CAT)
//
static uint32_t catGetFreq() { return(currentFrequency); }

static bool catSetFreq(uint32_t hz)
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

static uint8_t catGetMode()
{
  switch(currentMode)
  {
    case LSB: return(KW_LSB);
    case USB: return(KW_USB);
    case CW:  return(KW_CW);
    case FM:  return(KW_FM);
    default:  return(KW_AM);
  }
}

static bool catSetMode(uint8_t kw)
{
  uint8_t mode;
  switch(kw)
  {
    case KW_LSB: mode = LSB; break;
    case KW_USB: case KW_FSK: case KW_FSKR: mode = USB; break;   // Data modes on USB
    case KW_CW:  case KW_CWR: mode = CW; break;
    case KW_FM:  mode = FM; break;
    case KW_AM:  mode = AM; break;
    default: return(false);
  }
  if(mode == currentMode) return(true);
  if((mode == FM) != (currentMode == FM)) return(false);   // FM only on the FM band
  return(tuneToFrequency(currentFrequency, mode));
}

static uint8_t catGetSmeter() { return(catSmeterFromRssi(rssi)); }
static uint8_t catGetVolume() { return((uint8_t)(volume * 255 / 63)); }
static void catSetVolume(uint8_t v) { doVolume((int16_t)(v * 63 / 255) - (int16_t)volume); }

static const CatRadio catRadio = { catGetFreq, catSetFreq, catGetMode, catSetMode, catGetSmeter, catGetVolume, catSetVolume };
static CatKenwood cat;

bool catSerialLoop()
{
  bool changed = false;
  char out[48];
  // Handle what has arrived, a bounded amount per loop
  for(int i = 0 ; i < 64 && Serial.available() ; i++)
  {
    int c = Serial.read();
    if(c < 0) break;
    if(cat.feed((char)c, catRadio, out))
    {
      if(out[0]) Serial.print(out);
      changed = true;
    }
  }
  return(changed);
}
