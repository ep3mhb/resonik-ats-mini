//
// RESONIK R5: number pad to type a frequency (Menu -> Tuning -> Freq Entry).
// Turn to move, click to press a key, a short press leaves. kHz or MHz
// tunes there, switching band (and leaving FM) when the frequency is
// outside the current one, as CAT does.
//
#include "Common.h"
#include "Themes.h"
#include "Menu.h"
#include "Draw.h"
#include "Utils.h"
#include "Resonik.h"
#include "FreqPad.h"

static const char *const keys[16] =
{
  "1", "2", "3", "Del",
  "4", "5", "6", "Clear",
  "7", "8", "9", "kHz",
  ".", "0", "MHz", "Back",
};
#define K_DEL    3
#define K_CLEAR  7
#define K_KHZ   11
#define K_DOT   12
#define K_MHZ   14
#define K_BACK  15

static char entry[12];
static int entryLen = 0;
static int sel = 0;
static const char *note = nullptr;

void freqPadOpen()
{
  entryLen = 0;
  entry[0] = '\0';
  sel = 0;
  note = nullptr;
  currentCmd = CMD_FREQPAD;
}

void freqPadTurn(int enc)
{
  sel = ((sel + enc) % 16 + 16) % 16;
  note = nullptr;
}

static bool padApply(double mult)
{
  if(!entryLen) { note = "Type a number\nfirst"; return(false); }
  double v = atof(entry) * mult;
  if(v < 150000.0 || v > 108000000.0 || (v > 30000000.0 && v < 64000000.0))
  {
    note = "Out of range\n0.15-30 MHz\n64-108 MHz";
    return(false);
  }
  if(!resonikTuneHz((uint32_t)(v + 0.5)))
  {
    note = "Cannot tune\nthere";
    return(false);
  }
  return(true);
}

void freqPadClick(bool shortPress)
{
  if(shortPress) { currentCmd = CMD_NONE; return; }
  switch(sel)
  {
    case K_DEL:   if(entryLen) entry[--entryLen] = '\0'; break;
    case K_CLEAR: entryLen = 0; entry[0] = '\0'; break;
    case K_BACK:  currentCmd = CMD_NONE; return;
    case K_KHZ:   if(padApply(1000.0)) currentCmd = CMD_NONE; return;
    case K_MHZ:   if(padApply(1000000.0)) currentCmd = CMD_NONE; return;
    case K_DOT:
      if(strchr(entry, '.') || entryLen >= (int)sizeof(entry) - 1) break;
      entry[entryLen++] = '.';
      entry[entryLen] = '\0';
      break;
    default:
      if(entryLen >= (int)sizeof(entry) - 1) break;
      entry[entryLen++] = keys[sel][0];
      entry[entryLen] = '\0';
      break;
  }
  note = nullptr;
}

// Lines of a text one under the other. The column left of the keys is
// 118 px wide: every line here is under that (measured with the font).
// One long line ran over the first two columns of keys (Mohammad, 21 Nov).
static void padLines(const char *text, int x, int y, int step)
{
  char line[24];
  while(*text)
  {
    int n = 0;
    while(*text && *text != '\n' && n < (int)sizeof(line) - 1) line[n++] = *text++;
    line[n] = '\0';
    if(*text == '\n') text++;
    spr.drawString(line, x, y, uiFont(true));
    y += step;
  }
}

void drawFreqPad()
{
  spr.fillSprite(TH.bg);
  spr.fillRoundRect(4, 3, 312, 24, 6, TH.menu_bg);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(TH.menu_hdr);
  spr.drawString("Frequency", 12, 15, uiFont(false));

  // The number typed so far, or a message
  spr.drawRoundRect(12, 31, 116, 26, 6, TH.menu_border);
  spr.setTextDatum(MR_DATUM);
  spr.setTextColor(TH.freq_text);
  spr.drawString(entryLen ? entry : "", 122, 44, uiFont(false));
  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(note ? TH.text_warn : TH.text_muted);
  padLines(note ? note : "Type a number,\nthen kHz or MHz", 12, 61, 17);

  // Keys: four columns on the right, four rows
  for(int i = 0; i < 16; i++)
  {
    int x = 136 + (i % 4) * 45, y = 31 + (i / 4) * 34;
    bool hl = i == sel;
    if(hl) spr.fillRoundRect(x, y, 42, 30, 6, TH.menu_hl_bg);
    else spr.drawRoundRect(x, y, 42, 30, 6, TH.menu_border);
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(hl ? TH.menu_hl_text : TH.menu_item);
    spr.drawString(keys[i], x + 21, y + 15, uiFont(true));
  }

  spr.setTextDatum(TL_DATUM);
  spr.setTextColor(TH.text_muted);
  padLines("Turn: move\nClick: press\nPress: leave", 12, 116, 17);
}
