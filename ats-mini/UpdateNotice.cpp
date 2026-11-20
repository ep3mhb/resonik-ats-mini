//
// RESONIK R5: new version notice (see UpdateNotice.h).
//
// The check itself is the one Menu -> System -> Update FW makes (a HEAD
// request for GitHub's latest-release redirect, Ota.cpp), run on the network
// job task so a slow network never stops the knob or the screen. It needs the
// clock (TLS) and a connection; nothing is sent but that one request.
//
#include "Common.h"
#include "Themes.h"
#include "Menu.h"
#include "Draw.h"
#include "Storage.h"
#include "Utils.h"
#include "Ota.h"
#include "OtaVersion.h"
#include "UpdateNotice.h"
#include <atomic>

#define UPD_FIRST_CHECK   (3UL * 60 * 1000)          // After the first connection
#define UPD_RECHECK       (24UL * 60 * 60 * 1000)    // Then once a day
#define UPD_RETRY         (30UL * 60 * 1000)         // After a failed check

uint8_t updNoticeIdx = 1;
uint32_t updSkipVersion = 0;
const char *const updNoticeDesc[2] = { "Off", "On" };

static std::atomic<uint32_t> latest{0};      // Newer release found, 0 none
static std::atomic<bool> pending{false};     // Open the page when the radio is free
static uint32_t firstSeen = 0;               // First time the network was up
static uint32_t lastCheck = 0;               // Last check (millis), 0 never
static bool lastOk = true;
static uint32_t shownVersion = 0;            // Page already opened for this release this time
static int choice = 0;
static uint32_t openedAt = 0;
#define UPD_PAGE_TIMEOUT  60000UL                  // Unanswered: as Later

void updNoticeVersionText(char *buf, int size, uint32_t v)
{
  snprintf(buf, size, "v%u.%02u-r%u", (unsigned)(v / 10000), (unsigned)((v / 100) % 100), (unsigned)(v % 100));
}

bool updNoticeDue(uint32_t now)
{
  if(!updNoticeIdx) return(false);
  if(!firstSeen) { firstSeen = now ? now : 1; return(false); }
  if(!lastCheck) return(now - firstSeen >= UPD_FIRST_CHECK);
  return(now - lastCheck >= (lastOk ? UPD_RECHECK : UPD_RETRY));
}

void updNoticeChecked(bool ok, uint32_t version)
{
  // Runs on the network job task: only atomics and plain stores here
  lastCheck = millis();
  if(!lastCheck) lastCheck = 1;
  lastOk = ok;
  if(!ok) return;
  if(!otaIsNewer(version, VER_FULL, VER_PRERELEASE)) { latest = 0; return; }
  latest = version;
  // "Later" means the next check asks again; a skipped release never
  if(version != updSkipVersion) pending = true;
}

bool updNoticeWaiting() { return(updNoticeIdx && latest.load() != 0); }
uint32_t updNoticeVersion() { return(latest.load()); }

bool updNoticeTick()
{
  if(currentCmd == CMD_UPDNOTICE && millis() - openedAt > UPD_PAGE_TIMEOUT)
  {
    currentCmd = CMD_NONE;
    return(true);
  }
  if(!pending.load() || !updNoticeIdx) return(false);
  // Only over the normal screen, never over a page, menu or the OTA progress
  if(currentCmd != CMD_NONE || sleepOn()) return(false);
  uint32_t v = latest.load();
  pending = false;
  if(!v || v == updSkipVersion) return(false);
  shownVersion = v;
  choice = 0;
  openedAt = millis();
  currentCmd = CMD_UPDNOTICE;
  return(true);
}

void updNoticeTurn(int enc)
{
  choice = ((choice + enc) % 3 + 3) % 3;
  openedAt = millis();
}

void updNoticeClick(bool shortPress)
{
  // A short press, like the timeout, is "Later"
  if(!shortPress)
  {
    if(choice == 0) otaRequestLatest(true);
    else if(choice == 2) { updSkipVersion = shownVersion; prefsRequestSave(SAVE_SETTINGS); }
  }
  currentCmd = CMD_NONE;
}

void drawUpdNotice()
{
  static const char *const opts[3] = { "Update now", "Later", "Skip this version" };
  char line[48], ver[16];
  updNoticeVersionText(ver, sizeof(ver), shownVersion);

  spr.fillSprite(TH.bg);
  spr.fillRoundRect(4, 3, 312, 24, 6, TH.menu_bg);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(TH.menu_hdr);
  spr.drawString("New firmware", 12, 15, uiFont(false));

  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TH.text);
  snprintf(line, sizeof(line), "RESONIK %s is available", ver);
  spr.drawString(line, 160, 44, uiFont(false));
  spr.setTextColor(TH.text_muted);
  spr.drawString("Settings and memories are kept", 160, 64, uiFont(true));

  for(int i = 0; i < 3; i++)
  {
    int y = 82 + i * 26;
    bool hl = i == choice;
    if(hl) spr.fillRoundRect(60, y, 200, 22, 6, TH.menu_hl_bg);
    else spr.drawRoundRect(60, y, 200, 22, 6, TH.menu_border);
    spr.setTextColor(hl ? TH.menu_hl_text : TH.menu_item);
    spr.drawString(opts[i], 160, y + 11, uiFont(true));
  }
  spr.setTextDatum(BC_DATUM);
  spr.setTextColor(TH.text_muted);
  spr.drawString("Turn: choose  Click: OK  Press: later", 160, 169, uiFont(true));
}
