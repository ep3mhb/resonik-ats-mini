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

#define BV_POINTS  160               // Points per sweep (the sweep has up to 200)
#define BV_ROWS    40                // Sweeps kept
#define BV_RGB(c)  ((uint16_t)((((c) >> 8) & 0xF800) | (((c) >> 5) & 0x07E0) | (((c) >> 3) & 0x001F)))

static uint8_t *rows = nullptr;      // BV_ROWS x BV_POINTS, RSSI dBuV
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
  rowCount = rowHead = 0;
  cursor = BV_POINTS / 2;
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
  // The sweep has 200 points; the view shows its middle 160
  int off = (200 - BV_POINTS) / 2;
  return(sweepStart + sweepStep * (cursor + off));
}

void bandViewClick(bool shortPress)
{
  uint32_t f = sweepStep ? cursorFreq() : 0;
  currentCmd = CMD_NONE;   // bandViewTick stops the sweep and restores the sound
  if(!shortPress && f) { bandViewTick(); resonikTuneHz(f); }
}

bool bandViewTick()
{
  bool want = currentCmd == CMD_BANDVIEW && !sleepOn();
  if(!want)
  {
    if(active) { active = sweeping = false; scanAsyncStop(); return(true); }
    return(false);
  }
  if(!active) { active = true; sweeping = false; }
  if(!sweeping)
  {
    sweeping = scanAsyncStart(centreFreq, viewStep());
    if(!sweeping) { currentCmd = CMD_NONE; return(true); }
  }
  // A few steps per loop: the knob and the screen stay responsive
  for(int i = 0; i < 4; i++)
  {
    if(!scanAsyncTick())
    {
      static uint8_t full[200];
      int n = scanLastSweep(&sweepStart, &sweepStep, full, 200);
      uint8_t *row = rows + rowHead * BV_POINTS;
      int off = (200 - BV_POINTS) / 2;
      for(int x = 0; x < BV_POINTS; x++) row[x] = x + off < n ? full[x + off] : 0;
      rowHead = (rowHead + 1) % BV_ROWS;
      if(rowCount < BV_ROWS) rowCount++;
      sweeping = false;
      return(true);
    }
    delay(1);
  }
  return(false);
}

static uint16_t colour(int v, int lo, int hi)
{
  // Blue (weak) through green to yellow (strong)
  int t = hi > lo ? (v - lo) * 255 / (hi - lo) : 0;
  if(t < 0) t = 0;
  if(t > 255) t = 255;
  int r = t < 128 ? 0 : (t - 128) * 2, g = t < 128 ? t * 2 : 255, b = t < 128 ? 160 - t : 0;
  if(b < 0) b = 0;
  return(spr.color565(r, g > 255 ? 255 : g, b));
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

  // Waterfall, newest on top, each sweep 3 px; colours from the whole view
  int lo = 255, hi = 0;
  for(int r = 0; r < rowCount; r++) for(int x = 0; x < BV_POINTS; x++)
  {
    int v = rows[r * BV_POINTS + x];
    if(v < lo) lo = v;
    if(v > hi) hi = v;
  }
  for(int r = 0; r < rowCount; r++)
  {
    const uint8_t *row = rows + ((rowHead - 1 - r + BV_ROWS) % BV_ROWS) * BV_POINTS;
    for(int x = 0; x < BV_POINTS; x++) spr.fillRect(x * 2, 28 + r * 3, 2, 3, colour(row[x], lo, hi));
  }
  spr.drawFastVLine(cursor * 2, 28, BV_ROWS * 3, BV_RGB(0xF04770));
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
