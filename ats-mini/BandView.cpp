//
// RESONIK R5: Band View (Menu -> Tuning -> Band View): the band around the
// frequency swept again and again and drawn as a waterfall, newest sweep on
// top, weak blue to strong yellow. The sound is off while it sweeps. Turn to
// move the cursor, click to tune there and leave, a short press to leave.
//
// Auto-Store (Menu -> Memory -> Auto-Store): one sweep around the frequency,
// then the strongest stations go into free memory slots.
//
#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"
#include "Memories.h"
#include "Storage.h"
#include "Resonik.h"
#include "BandView.h"
#include <esp_heap_caps.h>

#define BV_POINTS  160               // Points per sweep, 2 px each
#define BV_ROWS    26                // Sweeps kept in the waterfall
#define BV_RGB(c)  ((uint16_t)((((c) >> 8) & 0xF800) | (((c) >> 5) & 0x07E0) | (((c) >> 3) & 0x001F)))

// Screen: the spectrum of the sweep on top (bars, filled in as the sweep
// goes), the sweeps before it as a waterfall under it. Until 21 Nov only
// the waterfall, one 3 px line per finished sweep: nothing moved for the
// 10 to 20 s a sweep took (Mohammad: very slow).
#define BV_SPEC_Y  28
#define BV_SPEC_H  40
#define BV_FALL_Y  71                // 26 lines of 3 px end at 148, over the foot line

// Readings are kept as dBuV + 1: 0 is "not measured" (past the band edge)
static uint8_t *rows = nullptr;      // BV_ROWS x BV_POINTS
static uint8_t live[BV_POINTS];      // The sweep in progress; beyond liveCount, the one before
static int liveCount = 0;
static int rowCount = 0, rowHead = 0;
static uint32_t sweepStart = 0, sweepStep = 0;
static int cursor = BV_POINTS / 2;
static bool active = false, sweeping = false;
static uint32_t centreFreq = 0;

static uint32_t viewStep()
{
  if(currentMode == FM) return(100000);
  if(currentMode == AM) return(getCurrentBand()->bandType == MW_BAND_TYPE ? 9000 : 5000);
  return(1000);
}

void bandViewOpen()
{
  if(!rows) rows = (uint8_t *)heap_caps_calloc(BV_ROWS * BV_POINTS, 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if(!rows) { statusShow("Band View", "Not enough memory"); return; }
  // The sweep has the receiver to itself
  beaconEndHere();
  memScanStop();
  bandScanStop();
  rowCount = rowHead = 0;
  liveCount = 0;
  memset(live, 0, sizeof(live));
  cursor = BV_POINTS / 2;
  sweepStep = 0;
  centreFreq = currentFrequency;
  currentCmd = CMD_BANDVIEW;
}

void bandViewTurn(int enc)
{
  cursor += enc;
  if(cursor < 0) cursor = 0;
  if(cursor >= BV_POINTS) cursor = BV_POINTS - 1;
}

static uint32_t cursorFreq()
{
  return(sweepStart + sweepStep * cursor);
}

void bandViewClick(bool shortPress)
{
  uint32_t f = sweepStep ? cursorFreq() : 0;
  currentCmd = CMD_NONE;   // bandViewTick stops the sweep and restores the sound
  if(!shortPress && f) { bandViewTick(); resonikTuneHz(f); }
}

bool bandViewTick()
{
  static uint32_t lastDraw = 0;
  bool want = currentCmd == CMD_BANDVIEW && !sleepOn();
  if(!want)
  {
    if(active) { active = sweeping = false; scanAsyncStop(); return(true); }
    return(false);
  }
  if(!active) { active = true; sweeping = false; }
  if(!sweeping)
  {
    sweeping = scanAsyncStart(centreFreq, viewStep(), BV_POINTS);
    if(!sweeping) { currentCmd = CMD_NONE; return(true); }
    liveCount = 0;
  }
  // Up to 15 ms of each pass of the main loop: a tune is complete within a
  // few questions, and the knob and the screen still answer at once
  bool done = false;
  uint32_t t0 = millis();
  do
  {
    if(!scanAsyncTick()) { done = true; break; }
    delay(1);
  }
  while(millis() - t0 < 15);

  // The points measured since the last pass replace those of the sweep before
  static uint8_t pts[BV_POINTS];
  int n = scanLastSweep(&sweepStart, &sweepStep, pts, BV_POINTS);
  bool grew = n > liveCount;
  for(int i = liveCount; i < n; i++) live[i] = pts[i] < 255 ? pts[i] + 1 : 255;
  liveCount = n;

  if(done)
  {
    // Past the band edge nothing was measured
    for(int i = n; i < BV_POINTS; i++) live[i] = 0;
    memcpy(rows + rowHead * BV_POINTS, live, BV_POINTS);
    rowHead = (rowHead + 1) % BV_ROWS;
    if(rowCount < BV_ROWS) rowCount++;
    sweeping = false;
    lastDraw = millis();
    return(true);
  }
  // The bars grow five times a second
  if(grew && millis() - lastDraw >= 200) { lastDraw = millis(); return(true); }
  return(false);
}

// Blue (weak) through green to yellow (strong); dim for the sweep before
static uint16_t colour(int v, int lo, int hi, bool dim = false)
{
  int t = hi > lo ? (v - lo) * 255 / (hi - lo) : 0;
  if(t < 0) t = 0;
  if(t > 255) t = 255;
  int r = t < 128 ? 0 : (t - 128) * 2, g = t < 128 ? t * 2 : 255, b = t < 128 ? 160 - t : 0;
  if(g > 255) g = 255;
  if(b < 0) b = 0;
  if(dim) { r = r * 2 / 5; g = g * 2 / 5; b = b * 2 / 5 + 24; }
  return(spr.color565(r, g, b));
}

void drawBandView()
{
  char line[40];
  spr.fillSprite(BV_RGB(0x041F29));
  spr.fillRoundRect(4, 3, 312, 22, 6, BV_RGB(0x073A4B));
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(BV_RGB(0xFFD167));
  spr.drawString("Band View", 10, 14, uiFont(false));
  spr.setTextDatum(MR_DATUM);
  spr.setTextColor(BV_RGB(0xEEF5F7));
  if(sweepStep)
  {
    uint32_t f = cursorFreq();
    if(currentMode == FM) snprintf(line, sizeof(line), "%.1f MHz", f / 1e6);
    else snprintf(line, sizeof(line), "%.0f kHz", f / 1e3);
  }
  else snprintf(line, sizeof(line), "Sweeping...");
  spr.drawString(line, 310, 14, uiFont(true));

  // One scale for the bars and the waterfall: weakest to strongest reading
  int lo = 255, hi = 0;
  for(int x = 0; x < BV_POINTS; x++) if(live[x]) { if(live[x] < lo) lo = live[x]; if(live[x] > hi) hi = live[x]; }
  for(int r = 0; r < rowCount; r++) for(int x = 0; x < BV_POINTS; x++)
  {
    int v = rows[r * BV_POINTS + x];
    if(!v) continue;
    if(v < lo) lo = v;
    if(v > hi) hi = v;
  }
  if(hi < lo + 10) hi = lo + 10;     // Noise alone does not fill the height

  // Spectrum: this sweep bright up to where it has come, the one before dim
  spr.drawFastHLine(0, BV_SPEC_Y + BV_SPEC_H, 320, BV_RGB(0x0E4A5E));
  for(int x = 0; x < BV_POINTS; x++)
  {
    int v = live[x];
    if(!v) continue;
    int h = 2 + (v - lo) * (BV_SPEC_H - 2) / (hi - lo);
    if(h > BV_SPEC_H) h = BV_SPEC_H;
    spr.fillRect(x * 2, BV_SPEC_Y + BV_SPEC_H - h, 2, h, colour(v, lo, hi, x >= liveCount));
  }
  // Where the sweep is
  if(sweeping && liveCount < BV_POINTS) spr.fillRect(liveCount * 2, BV_SPEC_Y + BV_SPEC_H + 1, 4, 2, BV_RGB(0xFFD167));

  // Waterfall, newest on top, each sweep 3 px
  for(int r = 0; r < rowCount; r++)
  {
    const uint8_t *row = rows + ((rowHead - 1 - r + BV_ROWS) % BV_ROWS) * BV_POINTS;
    for(int x = 0; x < BV_POINTS; x++) if(row[x]) spr.fillRect(x * 2, BV_FALL_Y + r * 3, 2, 3, colour(row[x], lo, hi));
  }
  spr.drawFastVLine(cursor * 2, BV_SPEC_Y, BV_FALL_Y + BV_ROWS * 3 - BV_SPEC_Y, BV_RGB(0xF04770));
  spr.setTextDatum(BC_DATUM);
  spr.setTextColor(BV_RGB(0x8FB3C0));
  spr.drawString("Turn: cursor  Click: tune there  Press: leave", 160, 169, uiFont(true));
}

//
// Auto-Store
//
void autoStoreRun()
{
  drawMessage("Auto-Store: scanning...");
  scanRun(currentFrequency, viewStep());
  uint32_t found[20];
  int n = scanPeaks(found, 20);
  int stored = 0, first = -1, last = -1;
  for(int i = 0; i < n; i++)
  {
    // Not twice: skip stations already in memory (within one step)
    bool known = false;
    for(int s = 0; s < getTotalMemories() && !known; s++)
    {
      Memory m = getMemory(s);
      if(m.freq && (m.freq > found[i] ? m.freq - found[i] : found[i] - m.freq) < viewStep()) known = true;
    }
    if(known) continue;
    int slot = -1;
    for(int s = 0; s < getTotalMemories(); s++) if(!getMemory(s).freq) { slot = s; break; }
    if(slot < 0) break;
    Memory m = {};
    m.freq = found[i];
    m.band = bandIdx;
    m.mode = currentMode;
    strncpy(m.name, "Auto", sizeof(m.name));
    setMemory(slot, m);
    if(first < 0) first = slot;
    last = slot;
    stored++;
  }
  char line[40];
  if(stored) { snprintf(line, sizeof(line), "%d stations, slots %d-%d", stored, first + 1, last + 1); prefsRequestSave(SAVE_MEMORIES); }
  else snprintf(line, sizeof(line), n ? "Already in memory" : "No strong stations found");
  statusShow("Auto-Store", line, 4000);
}
