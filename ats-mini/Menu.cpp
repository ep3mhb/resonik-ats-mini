#include "math.h"
#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Draw.h"
#include "EIBI.h"
#include "Ota.h"
#include "BleMode.h"
#include "Menu.h"
#include "Memories.h"
#include "BandPlan.h"
#include "Resonik.h"
#include "TileMenuData.h"
#include "CwAudio.h"
#include "Ft8.h"
#include "Patches.h"
#include "Storage.h"

#include <time.h>

//
// Bands Menu
//
// TO CONFIGURE YOUR OWN BAND PLAN:
// Add new bands by inserting new lines in the table below. Remove
// bands by deleting lines. Change bands by editing lines below.
//
// NOTE:
// You have to RESET PREFERENCES after adding or removing lines in this
// table. Turn your receiver on with the encoder push button pressed
// at first time to RESET the preferences.
//

int bandIdx = 0;

// All band frequencies are in Hz.
// Band limits are expanded to align with the nearest tuning scale mark
// Do not forget to update the bands table in the manual.md
Band bands[] =
{
  {"VHF",  FM_BAND_TYPE, FM,  64000000, 108000000, 103900000, 2, 0, 0, 0},
  // All band. LW, MW and SW (from 150kHz to 30MHz)
  {"ALL",  SW_BAND_TYPE, AM,    150000,  30000000,  15000000, 1, 4, 0, 0},
  {"11M",  SW_BAND_TYPE, AM,  25600000,  26100000,  25850000, 1, 4, 0, 0},
  {"13M",  SW_BAND_TYPE, AM,  21500000,  21900000,  21650000, 1, 4, 0, 0},
  {"15M",  SW_BAND_TYPE, AM,  18900000,  19100000,  18950000, 1, 4, 0, 0},
  {"16M",  SW_BAND_TYPE, AM,  17400000,  18100000,  17650000, 1, 4, 0, 0},
  {"19M",  SW_BAND_TYPE, AM,  15100000,  15900000,  15450000, 1, 4, 0, 0},
  {"22M",  SW_BAND_TYPE, AM,  13500000,  13900000,  13650000, 1, 4, 0, 0},
  {"25M",  SW_BAND_TYPE, AM,  11000000,  13000000,  11850000, 1, 4, 0, 0},
  {"31M",  SW_BAND_TYPE, AM,   9000000,  11000000,   9650000, 1, 4, 0, 0},
  {"41M",  SW_BAND_TYPE, AM,   7000000,   9000000,   7300000, 1, 4, 0, 0},
  {"49M",  SW_BAND_TYPE, AM,   5000000,   7000000,   6000000, 1, 4, 0, 0},
  {"60M",  SW_BAND_TYPE, AM,   4000000,   5100000,   4950000, 1, 4, 0, 0},
  {"75M",  SW_BAND_TYPE, AM,   3500000,   4000000,   3950000, 1, 4, 0, 0},
  {"90M",  SW_BAND_TYPE, AM,   3000000,   3500000,   3300000, 1, 4, 0, 0},
//  {"25M",  SW_BAND_TYPE, AM,  11600000,  12100000,  11850000, 1, 4, 0},
//  {"31M",  SW_BAND_TYPE, AM,   9400000,   9900000,   9650000, 1, 4, 0},
//  {"41M",  SW_BAND_TYPE, AM,   7200000,   7500000,   7300000, 1, 4, 0},
//  {"49M",  SW_BAND_TYPE, AM,   5900000,   6200000,   6000000, 1, 4, 0},
//  {"60M",  SW_BAND_TYPE, AM,   4700000,   5100000,   4950000, 1, 4, 0},
//  {"75M",  SW_BAND_TYPE, AM,   3900000,   4000000,   3950000, 1, 4, 0},
//  {"90M",  SW_BAND_TYPE, AM,   3200000,   3400000,   3300000, 1, 4, 0},
  {"MW3",  MW_BAND_TYPE, AM,   1700000,   3500000,   2500000, 1, 4, 0, 0},
  {"MW2",  MW_BAND_TYPE, AM,    495000,   1701000,    783000, 2, 4, 0, 0},
  {"MW1",  MW_BAND_TYPE, AM,    150000,   1800000,    810000, 3, 4, 0, 0},
  {"160M", MW_BAND_TYPE, LSB,   1800000,   2000000,   1900000, 5, 4, 0, 0},
  {"80M",  SW_BAND_TYPE, LSB,   3500000,   4000000,   3800000, 5, 4, 0, 0},
  {"40M",  SW_BAND_TYPE, LSB,   7000000,   7300000,   7150000, 5, 4, 0, 0},
  {"30M",  SW_BAND_TYPE, LSB,  10000000,  10200000,  10125000, 5, 4, 0, 0},
  {"20M",  SW_BAND_TYPE, USB,  14000000,  14400000,  14100000, 5, 4, 0, 0},
  {"17M",  SW_BAND_TYPE, USB,  18000000,  18200000,  18115000, 5, 4, 0, 0},
  {"15M",  SW_BAND_TYPE, USB,  21000000,  21500000,  21225000, 5, 4, 0, 0},
  {"12M",  SW_BAND_TYPE, USB,  24800000,  25000000,  24940000, 5, 4, 0, 0},
  {"10M",  SW_BAND_TYPE, USB,  28000000,  29700000,  28500000, 5, 4, 0, 0},
  // https://www.hfunderground.com/wiki/CB
  // Also see MIN_CB_FREQUENCY and MAX_CB_FREQUENCY
  {"CB",   SW_BAND_TYPE, AM,  25000000,  28000000,  27135000, 0, 4, 0, 0},
};

int getTotalBands() { return(ITEM_COUNT(bands)); }
Band *getCurrentBand() { return(&bands[bandIdx]); }

//
// Main Menu
//

#define MENU_MODE         0
#define MENU_BAND         1
#define MENU_VOLUME       2
#define MENU_VFO          3
#define MENU_STEP         4
#define MENU_SEEK         5
#define MENU_SCAN         6
#define MENU_MEMORY       7
#define MENU_SQUELCH      8
#define MENU_BW           9
#define MENU_AGC_ATT     10
#define MENU_AVC         11
#define MENU_SOFTMUTE    12
#define MENU_BEACONS     13  // RESONIK
#define MENU_MEMSCAN     14  // RESONIK
#define MENU_SCANNER     15  // RESONIK R3
#define MENU_SETTINGS    16

int8_t menuIdx = MENU_VOLUME;

static const char *menu[] =
{
  "Mode",
  "Band",
  "Volume",
  "VFO A/B",
  "Step",
  "Seek",
  "Scan",
  "Memory",
  "Squelch",
  "Bandwidth",
  "AGC/ATTN",
  "AVC",
  "SoftMute",
  "Beacons",
  "Mem Scan",
  "Scanner",
  "Settings",
};

//
// Settings Menu
//

#define MENU_BRIGHTNESS   0
#define MENU_DATETIME     1
#define MENU_UTCOFFSET    2
#define MENU_SLEEP        3
#define MENU_SLEEPMODE    4
#define MENU_OFFTIMER     5
#define MENU_THEME        6
#define MENU_UI           7
#define MENU_ZOOM         8
#define MENU_SCROLL       9
#define MENU_RDS          10
#define MENU_FM_STEREO    11
#define MENU_FM_REGION    12
#define MENU_CALIBRATION  13
#define MENU_DSP_PATCHES  14
#define MENU_LOADEIBI     15
#define MENU_UPDATEFW     16
#define MENU_USBMODE      17
#define MENU_TCPMODE      18
#define MENU_BLEMODE      19
#define MENU_WIFIMODE     20
#define MENU_BANDPLAN     21  // RESONIK
#define MENU_AUTOATT      22  // RESONIK
#define MENU_MENUSTYLE    23  // RESONIK
#define MENU_LOGGER       24  // RESONIK
#define MENU_PROPAG       25  // RESONIK
#define MENU_KNOB         26  // RESONIK
#define MENU_CWDEC        27  // RESONIK R3
#define MENU_CWSTRIP      28  // RESONIK R3
#define MENU_BPAREA       29  // RESONIK R3
#define MENU_FT8          30  // RESONIK R3
#define MENU_ABOUT        31


static uint8_t updateFwIdx = 0;
static const char *const updateFwActions[] = {"Check", "Update"};

int8_t settingsIdx = MENU_BRIGHTNESS;

static const char *settings[] =
{
  "Brightness",
  "Date/Time",
  "Time Zone",
  "Sleep Timer",
  "Sleep Mode",
  "Off Timer",
  "Theme",
  "UI Layout",
  "Zoom Menu",
  "Scroll Dir.",
  "RDS",
  "FM Stereo",
  "FM Region",
  "Calibration",
  "DSP Patches",
  "Load EiBi",
  "Update FW",
  "USB Port",
  "TCP Port",
  "Bluetooth",
  "Wi-Fi",
  "Band Plan",
  "Auto Att.",
  "Menu Style",
  "Logger",
  "Propagation",
  "Knob",
  "CW Decoder",
  "CW Strip",
  "Plan Area",
  "FT8",
  "About",
};

//
// Tile menu (RESONIK): six categories, each a page of tiles behind a Back
// tile. Items point into menu[] or, with TILE_SET, into settings[], so the
// labels and actions are shared with the classic menu. Everyday items come
// first, the rest A to Z.
//
#define TILE_SET  0x80

typedef struct
{
  TileIcon icon;
  uint8_t  id;
} TileEntry;

typedef struct
{
  const char *label;
  TileIcon icon;
  const TileEntry *items;
  uint8_t count;
} TileCategory;

static const TileEntry tileTuning[] =
{
  { TI_BAND, MENU_BAND }, { TI_MODE, MENU_MODE }, { TI_BANDWIDTH, MENU_BW },
  { TI_REGION, TILE_SET | MENU_FM_REGION }, { TI_RDS, TILE_SET | MENU_RDS },
  { TI_SCAN, MENU_SCAN }, { TI_SCANNER, MENU_SCANNER }, { TI_SEEK, MENU_SEEK }, { TI_STEP, MENU_STEP }, { TI_VFO, MENU_VFO },
};
static const TileEntry tileAudio[] =
{
  { TI_VOLUME, MENU_VOLUME }, { TI_AGC, MENU_AGC_ATT }, { TI_ATT, TILE_SET | MENU_AUTOATT },
  { TI_AVC, MENU_AVC }, { TI_STEREO, TILE_SET | MENU_FM_STEREO },
  { TI_SOFTMUTE, MENU_SOFTMUTE }, { TI_SQUELCH, MENU_SQUELCH },
};
static const TileEntry tileMemory[] =
{
  { TI_MEMORY, MENU_MEMORY }, { TI_LOG, TILE_SET | MENU_LOGGER }, { TI_MEMSCAN, MENU_MEMSCAN },
};
static const TileEntry tileHam[] =
{
  { TI_BANDPLAN, TILE_SET | MENU_BANDPLAN }, { TI_BEACON, MENU_BEACONS },
  { TI_FORK, TILE_SET | MENU_CALIBRATION }, { TI_CWDEC, TILE_SET | MENU_CWDEC },
  { TI_CWSTRIP, TILE_SET | MENU_CWSTRIP }, { TI_FT8, TILE_SET | MENU_FT8 },
  { TI_DOWNLOAD, TILE_SET | MENU_LOADEIBI },
  { TI_GLOBE, TILE_SET | MENU_BPAREA }, { TI_PROP, TILE_SET | MENU_PROPAG },
};
static const TileEntry tileDisplay[] =
{
  { TI_BRIGHT, TILE_SET | MENU_BRIGHTNESS }, { TI_TILES, TILE_SET | MENU_MENUSTYLE },
  { TI_SCROLL, TILE_SET | MENU_SCROLL }, { TI_MOON, TILE_SET | MENU_SLEEPMODE },
  { TI_SLEEPTIMER, TILE_SET | MENU_SLEEP }, { TI_PALETTE, TILE_SET | MENU_THEME },
  { TI_LAYOUT, TILE_SET | MENU_UI }, { TI_ZOOM, TILE_SET | MENU_ZOOM },
};
static const TileEntry tileSystem[] =
{
  { TI_INFO, TILE_SET | MENU_ABOUT }, { TI_BT, TILE_SET | MENU_BLEMODE },
  { TI_CLOCK, TILE_SET | MENU_DATETIME }, { TI_CHIP, TILE_SET | MENU_DSP_PATCHES },
  { TI_KNOB, TILE_SET | MENU_KNOB },
  { TI_POWER, TILE_SET | MENU_OFFTIMER }, { TI_NETWORK, TILE_SET | MENU_TCPMODE },
  { TI_GLOBE, TILE_SET | MENU_UTCOFFSET }, { TI_FIRMWARE, TILE_SET | MENU_UPDATEFW },
  { TI_USB, TILE_SET | MENU_USBMODE }, { TI_WIFI, TILE_SET | MENU_WIFIMODE },
};

static const TileCategory tileCategories[] =
{
  { "Tuning",    TI_DIAL,     tileTuning,  ITEM_COUNT(tileTuning)  },
  { "Audio",     TI_VOLUME,   tileAudio,   ITEM_COUNT(tileAudio)   },
  { "Memory",    TI_MEMORY,   tileMemory,  ITEM_COUNT(tileMemory)  },
  { "Ham & SWL", TI_BEACON,   tileHam,     ITEM_COUNT(tileHam)     },
  { "Display",   TI_BRIGHT,   tileDisplay, ITEM_COUNT(tileDisplay) },
  { "System",    TI_SETTINGS, tileSystem,  ITEM_COUNT(tileSystem)  },
};

// Every menu item except Settings, and every settings item, has a tile
static_assert(ITEM_COUNT(tileTuning) + ITEM_COUNT(tileAudio) + ITEM_COUNT(tileMemory) +
              ITEM_COUNT(tileHam) + ITEM_COUNT(tileDisplay) + ITEM_COUNT(tileSystem) ==
              ITEM_COUNT(menu) - 1 + ITEM_COUNT(settings), "a tile for every menu item");

// Where the tile menu is: tileCat -1 is the top level. Inside a category,
// tileSlot counts the tiles on screen: each page holds Back and five items.
static int8_t tileCat = -1;
static int8_t tileSlot = 0;
static uint16_t tileReturnCmd = 0;   // Command opened from a tile; when it ends, the tiles come back
static int8_t tileValueItem = 0;     // Its item in the category, for the value screen header

static int tileSlotCount(int cat)
{
  if(cat < 0) return(ITEM_COUNT(tileCategories));
  int n = tileCategories[cat].count;
  return((n - 1) / 5 * 6 + 1 + (n - 1) % 5 + 1);
}

static const char *tileLabel(uint8_t id)
{
  return(id & TILE_SET ? settings[id & ~TILE_SET] : menu[id]);
}

// Opening the menu comes back to the page used last, so Band then Mode
// (say) is one click each; Back leads to the six categories
void tileMenuOpen()
{
  tileReturnCmd = 0;
}

// After a click: a setting opened from the tiles that has just closed goes
// back to its category page, unless the press was a short one
void tileAfterClick(uint16_t prevCmd, bool shortPress)
{
  if(!tileReturnCmd) return;
  if(currentCmd == tileReturnCmd) return;           // Still open
  if(prevCmd == tileReturnCmd && currentCmd == CMD_NONE && (!shortPress || prevCmd == CMD_DATETIME || prevCmd == CMD_UPDATEFW) && menuStyleIdx == MENU_STYLE_TILES)
    currentCmd = CMD_MENU;
  tileReturnCmd = 0;
}

void tileReturnCancel()
{
  tileReturnCmd = 0;
}

//
// RESONIK settings: band plan, automatic attenuation, menu style
//
uint8_t bandPlanIdx = BANDPLAN_AUTO_SSB;
const char *const bandPlanDesc[] = { "Off", "Scale", "Auto" };
uint8_t bandPlanArea = BP_AREA_R1;
const char *const bandPlanAreaDesc[] = { "Region 1", "UAE (A6)", "Iran (EP)" };
int getTotalBandPlanModes() { return(ITEM_COUNT(bandPlanDesc)); }

uint8_t autoAttIdx = 1;
const char *const autoAttDesc[] = { "Off", "On" };

uint8_t menuStyleIdx = MENU_STYLE_TILES;   // Tiles by default (RESONIK)
const char *const menuStyleDesc[] = { "Classic", "Tiles" };
static const char *const knobDesc[] = { "20 clicks", "30 clicks" };

//
// FM Region Menu
//
const FMRegion fmRegions[] = {
  // 50uS de-emphasis
  { 0x1, "EU/JP/AU" },
  // 75uS de-emphasis
  { 0x2, "US" },
};

int getTotalFMRegions() { return(ITEM_COUNT(fmRegions)); }

//
// Mode Menu
//

const char *bandModeDesc[] = { "FM", "LSB", "USB", "AM", "CW" };

int getTotalModes() { return(ITEM_COUNT(bandModeDesc)); }

//
// FM Stereo Menu
//

uint8_t fmStereoIdx = FM_STEREO_AUTO;
const char *const fmStereoDesc[] = { "Auto", "Mono" };

int getTotalFMStereoModes() { return(ITEM_COUNT(fmStereoDesc)); }

bool audioClamp = true;

uint8_t dspPatchesIdx = DSP_PATCHES_DEFAULT;

//
// Memory Menu
//

uint8_t memoryIdx = 0;
Memory newMemory;

//
// RDS Menu
//

uint8_t rdsModeIdx = 0;
const RDSMode rdsMode[] =
{
  { RDS_PS, "PS"},
  { RDS_PS | RDS_CT, "PS+CT" },
  { RDS_PS | RDS_PI, "PS+PI" },
  { RDS_PS | RDS_PI | RDS_CT, "PS+PI+CT" },
  { RDS_PS | RDS_PI | RDS_RT | RDS_PT, "ALL-CT (EU)" },
  { RDS_PS | RDS_PI | RDS_RT | RDS_PT | RDS_RBDS, "ALL-CT (US)" },
  { RDS_PS | RDS_PI | RDS_RT | RDS_PT | RDS_CT, "ALL (EU)" },
  { RDS_PS | RDS_PI | RDS_RT | RDS_PT | RDS_CT | RDS_RBDS, "ALL (US)" },
  { RDS_OFF, "OFF"},
};

uint8_t getRDSMode() { return(rdsMode[rdsModeIdx].mode); }
int getTotalRDSModes() { return(ITEM_COUNT(rdsMode)); }

//
// Sleep Mode Menu
//

uint8_t sleepModeIdx = SLEEP_LOCKED;
const char *const sleepModeDesc[] =
{ "Locked", "Unlocked", "CPU Sleep" };

int getTotalSleepModes() { return(ITEM_COUNT(sleepModeDesc)); }

//
// Time Zone Menu (RESONIK): plain UTC offsets plus named zones with automatic
// summer time. Index 0 is UTC, the default (upstream defaulted to UTC-5).
// https://en.wikipedia.org/wiki/List_of_UTC_offsets
//
uint8_t utcOffsetIdx = 0;
const TimeZone timeZones[] =
{
  {    0, TZ_NONE, "UTC", "UTC", nullptr, "UTC" },  // Default: index 0
  {  -720, TZ_NONE, "UTC-12", "UTC-12", nullptr, nullptr },
  {  -660, TZ_NONE, "UTC-11", "UTC-11", nullptr, nullptr },
  {  -600, TZ_NONE, "UTC-10", "UTC-10", nullptr, nullptr },
  {  -600, TZ_NONE, "Honolulu", "HST", nullptr, "Pacific/Honolulu" },
  {  -570, TZ_NONE, "UTC-9:30", "UTC-9:30", nullptr, nullptr },
  {  -540, TZ_NONE, "UTC-9", "UTC-9", nullptr, nullptr },
  {  -540, TZ_US, "Anchorage", "AKST", "AKDT", "America/Anchorage" },
  {  -480, TZ_NONE, "UTC-8", "UTC-8", nullptr, nullptr },
  {  -480, TZ_US, "Los Angeles", "PST", "PDT", "America/Los_Angeles" },
  {  -420, TZ_NONE, "UTC-7", "UTC-7", nullptr, nullptr },
  {  -420, TZ_US, "Denver", "MST", "MDT", "America/Denver" },
  {  -420, TZ_NONE, "Phoenix", "MST", nullptr, "America/Phoenix" },
  {  -360, TZ_NONE, "UTC-6", "UTC-6", nullptr, nullptr },
  {  -360, TZ_US, "Chicago", "CST", "CDT", "America/Chicago" },
  {  -360, TZ_NONE, "Mexico City", "CST", nullptr, "America/Mexico_City" },
  {  -300, TZ_NONE, "UTC-5", "UTC-5", nullptr, nullptr },
  {  -300, TZ_US, "New York", "EST", "EDT", "America/New_York" },
  {  -240, TZ_NONE, "UTC-4", "UTC-4", nullptr, nullptr },
  {  -210, TZ_NONE, "UTC-3:30", "UTC-3:30", nullptr, nullptr },
  {  -180, TZ_NONE, "UTC-3", "UTC-3", nullptr, nullptr },
  {  -180, TZ_NONE, "Sao Paulo", "BRT", nullptr, "America/Sao_Paulo" },
  {  -180, TZ_NONE, "Argentina", "ART", nullptr, "America/Argentina/Buenos_Aires" },
  {  -150, TZ_NONE, "UTC-2:30", "UTC-2:30", nullptr, nullptr },
  {  -120, TZ_NONE, "UTC-2", "UTC-2", nullptr, nullptr },
  {   -60, TZ_NONE, "UTC-1", "UTC-1", nullptr, nullptr },
  {     0, TZ_NONE, "UTC+0", "UTC+0", nullptr, nullptr },
  {     0, TZ_EU, "London", "GMT", "BST", "Europe/London" },
  {     0, TZ_EU, "Lisbon", "WET", "WEST", "Europe/Lisbon" },
  {    60, TZ_NONE, "UTC+1", "UTC+1", nullptr, nullptr },
  {    60, TZ_EU, "Berlin/Paris", "CET", "CEST", "Europe/Berlin" },
  {    60, TZ_NONE, "Lagos", "WAT", nullptr, "Africa/Lagos" },
  {   120, TZ_NONE, "UTC+2", "UTC+2", nullptr, nullptr },
  {   120, TZ_EU, "Athens", "EET", "EEST", "Europe/Athens" },
  {   120, TZ_NONE, "Cape Town", "SAST", nullptr, "Africa/Johannesburg" },
  {   180, TZ_NONE, "UTC+3", "UTC+3", nullptr, nullptr },
  {   180, TZ_NONE, "Moscow", "MSK", nullptr, "Europe/Moscow" },
  {   180, TZ_NONE, "Istanbul", "TRT", nullptr, "Europe/Istanbul" },
  {   180, TZ_NONE, "Riyadh", "AST", nullptr, "Asia/Riyadh" },
  {   210, TZ_NONE, "UTC+3:30", "UTC+3:30", nullptr, nullptr },
  {   210, TZ_NONE, "Tehran", "IRST", nullptr, "Asia/Tehran" },
  {   240, TZ_NONE, "UTC+4", "UTC+4", nullptr, nullptr },
  {   240, TZ_NONE, "Dubai", "GST", nullptr, "Asia/Dubai" },
  {   240, TZ_NONE, "Muscat", "GST", nullptr, "Asia/Muscat" },
  {   270, TZ_NONE, "UTC+4:30", "UTC+4:30", nullptr, nullptr },
  {   270, TZ_NONE, "Kabul", "AFT", nullptr, "Asia/Kabul" },
  {   300, TZ_NONE, "UTC+5", "UTC+5", nullptr, nullptr },
  {   300, TZ_NONE, "Karachi", "PKT", nullptr, "Asia/Karachi" },
  {   330, TZ_NONE, "UTC+5:30", "UTC+5:30", nullptr, nullptr },
  {   330, TZ_NONE, "Delhi", "IST", nullptr, "Asia/Kolkata" },
  {   345, TZ_NONE, "UTC+5:45", "UTC+5:45", nullptr, nullptr },
  {   345, TZ_NONE, "Kathmandu", "NPT", nullptr, "Asia/Kathmandu" },
  {   360, TZ_NONE, "UTC+6", "UTC+6", nullptr, nullptr },
  {   360, TZ_NONE, "Dhaka", "BST", nullptr, "Asia/Dhaka" },
  {   390, TZ_NONE, "UTC+6:30", "UTC+6:30", nullptr, nullptr },
  {   420, TZ_NONE, "UTC+7", "UTC+7", nullptr, nullptr },
  {   420, TZ_NONE, "Bangkok", "ICT", nullptr, "Asia/Bangkok" },
  {   480, TZ_NONE, "UTC+8", "UTC+8", nullptr, nullptr },
  {   480, TZ_NONE, "Singapore", "SGT", nullptr, "Asia/Singapore" },
  {   480, TZ_NONE, "Beijing", "CST", nullptr, "Asia/Shanghai" },
  {   480, TZ_NONE, "Perth", "AWST", nullptr, "Australia/Perth" },
  {   525, TZ_NONE, "UTC+8:45", "UTC+8:45", nullptr, nullptr },
  {   540, TZ_NONE, "UTC+9", "UTC+9", nullptr, nullptr },
  {   540, TZ_NONE, "Tokyo", "JST", nullptr, "Asia/Tokyo" },
  {   570, TZ_NONE, "UTC+9:30", "UTC+9:30", nullptr, nullptr },
  {   570, TZ_AU, "Adelaide", "ACST", "ACDT", "Australia/Adelaide" },
  {   600, TZ_NONE, "UTC+10", "UTC+10", nullptr, nullptr },
  {   600, TZ_AU, "Sydney", "AEST", "AEDT", "Australia/Sydney" },
  {   600, TZ_NONE, "Brisbane", "AEST", nullptr, "Australia/Brisbane" },
  {   630, TZ_NONE, "UTC+10:30", "UTC+10:30", nullptr, nullptr },
  {   660, TZ_NONE, "UTC+11", "UTC+11", nullptr, nullptr },
  {   720, TZ_NONE, "UTC+12", "UTC+12", nullptr, nullptr },
  {   720, TZ_NZ, "Auckland", "NZST", "NZDT", "Pacific/Auckland" },
  {   765, TZ_NONE, "UTC+12:45", "UTC+12:45", nullptr, nullptr },
  {   780, TZ_NONE, "UTC+13", "UTC+13", nullptr, nullptr },
  {   825, TZ_NONE, "UTC+13:45", "UTC+13:45", nullptr, nullptr },
  {   840, TZ_NONE, "UTC+14", "UTC+14", nullptr, nullptr },
};

// Offsets of the upstream "UTCOffset" list (15-minute units), to migrate saved settings
static const int8_t legacyUtcOffsets[] =
{
  -48, -44, -40, -38, -36, -32, -28, -24, -20, -16, -14, -12, -10, -8, -4, 0, 4, 8, 12, 14, 16, 18, 20, 22, 23, 24, 26, 28, 32, 35, 36, 38, 40, 42, 44, 48, 51, 52, 55, 56
};

uint8_t timeZoneFromLegacyIndex(uint8_t idx)
{
  if(idx >= ITEM_COUNT(legacyUtcOffsets)) return(0);
  int16_t minutes = legacyUtcOffsets[idx] * 15;
  for(int i = 1 ; i < (int)ITEM_COUNT(timeZones) ; i++)
    if(!timeZones[i].iana && timeZones[i].stdMinutes == minutes) return(i);
  return(0);
}

const TimeZone *getCurrentTimeZone()
{
  if(utcOffsetIdx >= ITEM_COUNT(timeZones)) utcOffsetIdx = 0;
  return(&timeZones[utcOffsetIdx]);
}

// Current offset in 15-minute units, including summer time
int8_t getCurrentUTCOffset() { return(tzOffsetMinutes(getCurrentTimeZone(), time(NULL)) / 15); }
const char *getCurrentTimeZoneAbbr() { return(tzAbbr(getCurrentTimeZone(), time(NULL))); }
int getTotalUTCOffsets() { return(ITEM_COUNT(timeZones)); }

//
// Date/Time Menu
//
enum DateTimeField
{
  DATETIME_YEAR,
  DATETIME_MONTH,
  DATETIME_DAY,
  DATETIME_HOUR,
  DATETIME_MINUTE,
  DATETIME_SECOND,
  DATETIME_FIELD_COUNT,
};

static const uint16_t dateTimeMaxYear = 2105;
static uint8_t dateTimeField;
// Until a value is changed, the Date/Time screen follows the running clock
static bool dateTimeEdited;
static uint16_t dateTimeYear;
static uint8_t dateTimeMonth;
static uint8_t dateTimeDay;
static uint8_t dateTimeHour;
static uint8_t dateTimeMinute;
static uint8_t dateTimeSecond;

//
// UI Layout Menu
//
uint8_t uiLayoutIdx = 0;
const char *const uiLayoutDesc[] =
{ "Default", "S-Meter" };

int getTotalUILayouts() { return(ITEM_COUNT(uiLayoutDesc)); }

//
// USB Port Mode Menu
//

uint8_t usbModeIdx = USB_OFF;
const char *const usbModeDesc[] =
{ "Off", "Ad hoc", "CAT" };

int getTotalUSBModes() { return(ITEM_COUNT(usbModeDesc)); }

//
// TCP Port Mode Menu
//

uint8_t tcpModeIdx = TCP_OFF;
const char *const tcpModeDesc[] =
{ "Off", "Ad hoc" };

int getTotalTCPModes() { return(ITEM_COUNT(tcpModeDesc)); }

//
// Bluetooth Mode Menu
//

uint8_t bleModeIdx = BLE_OFF;
static uint8_t bleModeMenuIdx = BLE_OFF;
const char *const bleModeDesc[] =
{ "Off", "Ad hoc", "HID", "Unpair All" };

int getTotalBluetoothModes() { return(ITEM_COUNT(bleModeDesc)); }

//
// WiFi Mode Menu
//

uint8_t wifiModeIdx = NET_OFF;
const char *const wifiModeDesc[] =
{ "Off", "AP Only", "AP+Connect", "Connect", "Sync Only" };

int getTotalWiFiModes() { return(ITEM_COUNT(wifiModeDesc)); }

//
// Step Menu
//

// FM (Hz)
static const Step fmSteps[] =
{
  {   10000, "10k",   1 },
  {   50000, "50k",   5 },
  {  100000, "100k", 10 },
  {  200000, "200k", 20 },
  { 1000000, "1M",   10 },
};

// SSB (Hz)
static const Step ssbSteps[] =
{
  {    10, "10",  1  },
  {    25, "25",  1  },
  {    50, "50",  1  },
  {   100, "100", 1  },
  {   500, "500", 1  },
  {  1000, "1k",  1  },
  {  5000, "5k",  5  },
  {  9000, "9k",  9  },
  { 10000, "10k", 10 },
};

// AM (Hz)
static const Step amSteps[] =
{
  {    1000, "1k",    1 },
  {    5000, "5k",    5 },
  {    9000, "9k",    9 },
  {   10000, "10k",  10 },
  {   50000, "50k",  10 },
  {  100000, "100k", 10 },
  { 1000000, "1M",   10 },
};

// CW (Hz): fine steps, 10 Hz by default
static const Step cwSteps[] =
{
  {    10, "10",  1 },
  {    25, "25",  1 },
  {    50, "50",  1 },
  {   100, "100", 1 },
  {   500, "500", 1 },
  {  1000, "1k",  1 },
};

static const Step *steps[5] = { fmSteps, ssbSteps, ssbSteps, amSteps, cwSteps };
static const uint8_t defaultStepIdx[5] = { 2, 5, 5, 1, 0 };

static int getLastStep(int mode)
{
  switch(mode)
  {
    case FM:  return(LAST_ITEM(fmSteps));
    case LSB: return(LAST_ITEM(ssbSteps));
    case USB: return(LAST_ITEM(ssbSteps));
    case AM:  return(LAST_ITEM(amSteps));
    case CW:  return(LAST_ITEM(cwSteps));
  }

  return(0);
}

const Step *getCurrentStep()
{
  uint8_t idx = bands[bandIdx].currentStepIdx > getLastStep(currentMode) ? defaultStepIdx[currentMode] : bands[bandIdx].currentStepIdx;
  return(&steps[currentMode][idx]);
}

static uint8_t freqInputPos = 0;

static uint8_t getDefaultFreqInputPos(int step)
{
  return (uint8_t)(log10(step) * 2);
}

void resetFreqInputPos()
{
  freqInputPos = getDefaultFreqInputPos(getCurrentStep()->step);
}

uint8_t getFreqInputPos()
{
  // FM highlighting starts at 10 kHz; AM/SSB highlighting starts at 1 Hz.
  return freqInputPos - (currentMode == FM ? 8 : 0);
}

int getFreqInputStep()
{
  return (freqInputPos % 2 ? 5 : 1) * pow(10, freqInputPos / 2);
}

static uint8_t getMinFreqInputPos()
{
  return getDefaultFreqInputPos(steps[currentMode][0].step);
}

static uint8_t getMaxFreqInputPos()
{
  return (uint8_t)log10(getCurrentBand()->maximumFreq) * 2 - (currentMode == FM ? 2 : 0);
}

//
// Bandwidth Menu
//

static const Bandwidth fmBandwidths[] =
{
  { 0, "Auto" }, // Automatic - default
  { 1, "110k" }, // Force wide (110 kHz) channel filter.
  { 2, "84k"  },
  { 3, "60k"  },
  { 4, "40k"  }
};

static const Bandwidth ssbBandwidths[] =
{
  { 4, "0.5k" },
  { 5, "1.0k" },
  { 0, "1.2k" },
  { 1, "2.2k" },
  { 2, "3.0k" },
  { 3, "4.0k" }
};

static const Bandwidth amBandwidths[] =
{
  { 4, "1.0k" },
  { 5, "1.8k" },
  { 3, "2.0k" },
  { 6, "2.5k" },
  { 2, "3.0k" },
  { 1, "4.0k" },
  { 0, "6.0k" }
};

static const Bandwidth *bandwidths[5] =
{
  fmBandwidths, ssbBandwidths, ssbBandwidths, amBandwidths, ssbBandwidths
};

// CW defaults to the narrow 0.5 kHz filter
static const uint8_t defaultBwIdx[5] = { 0, 4, 4, 4, 0 };

static int getLastBandwidth(int mode)
{
  switch(mode)
  {
    case FM:  return(LAST_ITEM(fmBandwidths));
    case LSB: return(LAST_ITEM(ssbBandwidths));
    case USB: return(LAST_ITEM(ssbBandwidths));
    case AM:  return(LAST_ITEM(amBandwidths));
    case CW:  return(LAST_ITEM(ssbBandwidths));
  }

  return(0);
}

static int8_t getCurrentBandwidthIdx()
{
  int8_t idx = bands[bandIdx].bandwidthIdx;
  return(idx<0 || idx>getLastBandwidth(currentMode)? defaultBwIdx[currentMode] : idx);
}

const Bandwidth *getCurrentBandwidth()
{
  return(&bandwidths[currentMode][getCurrentBandwidthIdx()]);
}

// CW tone pitch. The SI4732 CW filters are band-pass: 0.5k covers 250..750 Hz
// and 1.0k covers 500..1500 Hz, so the pitch sits in the middle of the filter.
int cwPitch()
{
  switch(getCurrentBandwidth()->idx)
  {
    case 4:  return(500);   // 0.5k
    case 5:  return(1000);  // 1.0k
    default: return(700);   // Wider low-pass filters
  }
}

static void setBandwidth()
{
  uint8_t idx = getCurrentBandwidth()->idx;

  switch(currentMode)
  {
    case FM:
      rx.setFmBandwidth(idx);
      break;
    case AM:
      rx.setBandwidth(idx, 1);
      break;
    case LSB:
    case USB:
    case CW:
      // Set Audio
      rx.setSSBAudioBandwidth(idx);
      // If audio bandwidth selected is about 2 kHz or below, it is
      // recommended to set Sideband Cutoff Filter to 0.
      rx.setSSBSidebandCutoffFilter(idx==0 || idx==4 || idx==5? 0 : 1);
      break;
  }
}

// Seek mode. Pass true to toggle, false to return the current one
uint8_t seekMode(bool toggle)
{
  static uint8_t mode = SEEK_DEFAULT;

  mode = toggle ? (mode == SEEK_DEFAULT ? SEEK_SCHEDULE : SEEK_DEFAULT) : mode;

  // Use normal seek on FM or if there is no schedule loaded
  if(currentMode == FM || !eibiAvailable() || !clockAvailable())
    return(SEEK_DEFAULT);

  return(mode);
}

//
// Utility functions to change menu values
//

static inline int min(int x, int y) { return(x<y? x:y); }

static inline int wrap_range(int v, int enc, int vMin, int vMax)
{
  v += enc;
  v  = v>vMax? vMin + (v - vMax - 1) % (vMax - vMin + 1) : v<vMin? vMax - (vMin - v - 1) % (vMax - vMin + 1) : v;
  return(v);
}

static inline int clamp_range(int v, int enc, int vMin, int vMax)
{
  v += enc;
  v  = v>vMax? vMax : v<vMin? vMin : v;
  return(v);
}

static uint8_t dateTimeDaysInMonth(uint16_t year, uint8_t month)
{
  struct tm fields = {};
  fields.tm_year = year - 1900;
  fields.tm_mon = month;
  fields.tm_mday = 0;
  mktime(&fields);
  return(fields.tm_mday);
}

static void dateTimeRefresh()
{
  dateTimeYear = dateTimeMonth = dateTimeDay = 0;
  dateTimeHour = dateTimeMinute = dateTimeSecond = 0;

  if(!clockAvailable()) return;

  time_t now = time(NULL);
  struct tm fields;
  gmtime_r(&now, &fields);
  dateTimeHour = fields.tm_hour;
  dateTimeMinute = fields.tm_min;
  dateTimeSecond = fields.tm_sec;

  // A time-only RDS update does not provide a usable date.
  if(!clockGetDate(NULL, NULL, NULL, NULL)) return;

  dateTimeYear = min(fields.tm_year + 1900, dateTimeMaxYear);
  dateTimeMonth = fields.tm_mon + 1;
  dateTimeDay = fields.tm_mday;
}

static void dateTimeInit()
{
  dateTimeField = DATETIME_YEAR;
  dateTimeEdited = false;
  dateTimeRefresh();
}

static void doDateTime(int16_t enc)
{
  dateTimeEdited = true;
  bool dateMissing = dateTimeYear < CLOCK_MIN_YEAR || !dateTimeMonth || !dateTimeDay;
  if(dateMissing)
  {
    dateTimeYear = CLOCK_MIN_YEAR;
    dateTimeMonth = 1;
    dateTimeDay = 1;
    // The first step toward a higher value selects the minimum date value.
    if(dateTimeField <= DATETIME_DAY && enc > 0) enc--;
  }

  switch(dateTimeField)
  {
    case DATETIME_YEAR:   dateTimeYear   = clamp_range(dateTimeYear,   enc, CLOCK_MIN_YEAR, dateTimeMaxYear);break;
    case DATETIME_MONTH:  dateTimeMonth  = wrap_range(dateTimeMonth,  enc, 1, 12);break;
    case DATETIME_DAY:    dateTimeDay    = wrap_range(dateTimeDay,    enc, 1, dateTimeDaysInMonth(dateTimeYear, dateTimeMonth));break;
    case DATETIME_HOUR:   dateTimeHour   = wrap_range(dateTimeHour,   enc, 0, 23);break;
    case DATETIME_MINUTE: dateTimeMinute = wrap_range(dateTimeMinute, enc, 0, 59);break;
    case DATETIME_SECOND: dateTimeSecond = wrap_range(dateTimeSecond, enc, 0, 59);break;
  }

  // Keep the day valid when the month or year changes.
  if(dateTimeField <= DATETIME_MONTH)
    dateTimeDay = min(dateTimeDay, dateTimeDaysInMonth(dateTimeYear, dateTimeMonth));
}

static void clickDateTime(bool shortPress)
{
  if(!shortPress)
  {
    dateTimeField = (dateTimeField + 1) % DATETIME_FIELD_COUNT;
    return;
  }

  // Nothing changed: leave without setting the clock to the time the screen opened
  uint32_t epoch;
  if(dateTimeEdited &&
     clockUTCDateTimeToEpoch(dateTimeYear, dateTimeMonth, dateTimeDay,
                            dateTimeHour, dateTimeMinute, dateTimeSecond, &epoch))
    clockSetEpoch(epoch);

  currentCmd = CMD_NONE;
}

//
// Encoder input handlers
//

void doSelectDigit(int16_t enc)
{
  freqInputPos = clamp_range(freqInputPos, -enc, getMinFreqInputPos(), getMaxFreqInputPos());
}

void doVolume(int16_t enc)
{
  volume = clamp_range(volume, enc, 0, 63);
  rx.setVolume(volume);
  if(enc) prefsRequestSave(SAVE_SETTINGS);
}

static void clickVolume(bool shortPress)
{
  if(shortPress) muteOn(MUTE_MAIN, !muteOn(MUTE_MAIN)); else currentCmd = CMD_NONE;
}

static void clickSquelch(bool shortPress)
{
  if(shortPress)
  {
    if(currentSquelch[currentMode] & 0x7f)
      currentSquelch[currentMode] &= 0x80;
    else
      currentSquelch[currentMode] ^= 0x80;
    prefsRequestSave(SAVE_SETTINGS);
  }
  else
  {
    currentCmd = CMD_NONE;
  }
}

static void clickSeek(bool shortPress)
{
  if(shortPress) seekMode(true); else currentCmd = CMD_NONE;
}

static void clickScan(bool shortPress)
{
  if(shortPress)
  {
    // Clear stale parameters
    clearStationInfo();
    rssi = snr = 0;
    drawScreen();
    drawMessage("Scanning...");
    scanRun(currentFrequency, currentMode == FM ? 100000 : 10000);
  }
  else currentCmd = CMD_NONE;
}

static void doTheme(int16_t enc)
{
  themeIdx = wrap_range(themeIdx, enc, 0, getTotalThemes() - 1);
  prefsRequestSave(SAVE_SETTINGS);
}

static void doBandPlan(int16_t enc)
{
  bandPlanIdx = wrap_range(bandPlanIdx, enc, 0, LAST_ITEM(bandPlanDesc));
  prefsRequestSave(SAVE_SETTINGS);
}

static void doAutoAtt(int16_t enc)
{
  autoAttIdx = wrap_range(autoAttIdx, enc, 0, LAST_ITEM(autoAttDesc));
  autoAttReset();
  prefsRequestSave(SAVE_SETTINGS);
}

static void doMenuStyle(int16_t enc)
{
  menuStyleIdx = wrap_range(menuStyleIdx, enc, 0, LAST_ITEM(menuStyleDesc));
  prefsRequestSave(SAVE_SETTINGS);
}

static void doUILayout(int16_t enc)
{
  uiLayoutIdx = uiLayoutIdx > LAST_ITEM(uiLayoutDesc) ? UI_DEFAULT : wrap_range(uiLayoutIdx, enc, 0, LAST_ITEM(uiLayoutDesc));
  prefsRequestSave(SAVE_SETTINGS);
}

void doAvc(int16_t enc)
{
  // Only allow for AM and SSB modes
  if(currentMode==FM) return;

  // wrap_range expects to wrap a range of incremental numbers. avc instead is a range of all even numbers
  int8_t newAvcIdx = wrap_range((isSSB() ? SsbAvcIdx : AmAvcIdx) / 2, enc, 12 / 2, 90 / 2) * 2;
  if(isSSB())
  {
    SsbAvcIdx = newAvcIdx;
    if(enc && ssbAvcHold)
    {
      ssbAvcHold = false;
      rx.setSSBAutomaticVolumeControl(1);
    }
  }
  else
  {
    AmAvcIdx = newAvcIdx;
  }
  rx.setAvcAmMaxGain(newAvcIdx);
  if(enc) prefsRequestSave(SAVE_SETTINGS);
}

static void clickAvc(bool shortPress)
{
  if(shortPress && isSSB())
  {
    ssbAvcHold = !ssbAvcHold;
    rx.setSSBAutomaticVolumeControl(!ssbAvcHold);
  }
  else currentCmd = CMD_NONE;
}

void doFmRegion(int16_t enc)
{
  FmRegionIdx = wrap_range(FmRegionIdx, enc, 0, LAST_ITEM(fmRegions));
  if(currentMode==FM)
    rx.setFMDeEmphasis(fmRegions[FmRegionIdx].value);
  if(enc) prefsRequestSave(SAVE_SETTINGS);
}

//
// Apply the stereo setting. The receiver blends down to mono on its
// own as the signal gets worse, so forcing mono is a matter of moving
// the blend thresholds out of reach. This has to run again after every
// band change, because the FM tuner starts up with the defaults.
//
void applyFmStereo()
{
  if(currentMode!=FM) return;

  if(fmStereoIdx==FM_STEREO_MONO)
    rx.setFmStereoOff();
  else
    rx.setFmStereoOn();
}

void doFmStereo(int16_t enc)
{
  fmStereoIdx = wrap_range(fmStereoIdx, enc, 0, LAST_ITEM(fmStereoDesc));
  applyFmStereo();
  if(enc) prefsRequestSave(SAVE_SETTINGS);
}

static uint8_t dspPatchSlots(uint8_t slots[PATCH_SET_COUNT + 1], uint8_t &selected)
{
  uint8_t count = 1;
  slots[0] = DSP_PATCHES_DEFAULT;
  selected = 0;
  for(uint8_t slot = 1; slot <= PATCH_SET_COUNT; slot++)
    if(patchesModes(slot))
    {
      if(slot == dspPatchesIdx) selected = count;
      slots[count++] = slot;
    }
  return count;
}

static void doDspPatches(int16_t enc)
{
  uint8_t slots[PATCH_SET_COUNT + 1], selected;
  uint8_t count = dspPatchSlots(slots, selected);
  if(!patchesSelect(slots[wrap_range(selected, enc, 0, count - 1)]))
    statusShow("Patch request busy", "Try again when finished");
}

void doCal(int16_t enc)
{
  if (currentMode == USB || currentMode == CW)
    bands[bandIdx].usbCal = clamp_range(bands[bandIdx].usbCal, 10*enc, -MAX_CAL, MAX_CAL);
  else if (currentMode == LSB)
    bands[bandIdx].lsbCal = clamp_range(bands[bandIdx].lsbCal, 10*enc, -MAX_CAL, MAX_CAL);
  // else: no calibration change for other modes

  // If in SSB mode set the SI4732/5 BFO value
  // This adjusts the BFO while in the calibration menu
  if(isSSB())
  {
    updateFrequency(currentFrequency, true);
    if(enc) prefsRequestSave(SAVE_CUR_BAND);
  }
}

void doBrt(int16_t enc)
{
  currentBrt = clamp_range(currentBrt, 5*enc, 10, 255);
  if(!sleepOn()) ledcWrite(PIN_LCD_BL, currentBrt);
  if(enc) prefsRequestSave(SAVE_SETTINGS);
}

static void doSleep(int16_t enc)
{
  currentSleep = clamp_range(currentSleep, 5*enc, 0, 255);
  prefsRequestSave(SAVE_SETTINGS);
}

static void doOffTimer(int16_t enc)
{
  if(!enc) return;
  offTimerMinutes = clamp_range(offTimerMinutes, 5 * enc, 0, 240);
  offTimerRestart();
  prefsRequestSave(SAVE_SETTINGS);
}

static void doSleepMode(int16_t enc)
{
  sleepModeIdx = wrap_range(sleepModeIdx, enc, 0, LAST_ITEM(sleepModeDesc));
  prefsRequestSave(SAVE_SETTINGS);
}

static void doUSBMode(int16_t enc)
{
  usbModeIdx = wrap_range(usbModeIdx, enc, 0, LAST_ITEM(usbModeDesc));
  prefsRequestSave(SAVE_SETTINGS);
}

static void doTCPMode(int16_t enc)
{
  tcpModeIdx = wrap_range(tcpModeIdx, enc, 0, LAST_ITEM(tcpModeDesc));
  prefsRequestSave(SAVE_SETTINGS);
}

static void doBleMode(int16_t enc)
{
  bleModeMenuIdx = wrap_range(bleModeMenuIdx, enc, 0, LAST_ITEM(bleModeDesc));
}

static void doWiFiMode(int16_t enc)
{
  wifiModeIdx = wrap_range(wifiModeIdx, enc, 0, LAST_ITEM(wifiModeDesc));
  prefsRequestSave(SAVE_SETTINGS);
}

static void clickBleMode(uint8_t mode, bool shortPress)
{
  currentCmd = CMD_NONE;
  bleModeIdx = mode;
  prefsRequestSave(SAVE_SETTINGS);
  bleInit(mode);
}

static void clickWiFiMode(uint8_t mode, bool shortPress)
{
  currentCmd = CMD_NONE;
  netInit(mode);
}

void doRDSMode(int16_t enc)
{
  rdsModeIdx = wrap_range(rdsModeIdx, enc, 0, LAST_ITEM(rdsMode));
  if(currentMode==FM) rx.setRdsConfig(getRDSMode()!=RDS_OFF, 2, 2, 2, 2);
  if(!(getRDSMode() & RDS_CT) && !ntpIsAvailable()) clockReset();
  prefsRequestSave(SAVE_SETTINGS);
}

static void doUTCOffset(int16_t enc)
{
  utcOffsetIdx = wrap_range(utcOffsetIdx, enc, 0, LAST_ITEM(timeZones));
  prefsRequestSave(SAVE_SETTINGS);
}

static void doZoom(int16_t enc)
{
  zoomMenu = !zoomMenu;
  prefsRequestSave(SAVE_SETTINGS);
}

static void doScrollDir(int16_t enc)
{
  scrollDirection = (scrollDirection == 1) ? -1 : 1;
  prefsRequestSave(SAVE_SETTINGS);
}

uint8_t doAbout(int16_t enc)
{
  static uint8_t aboutScreen = 0;
  aboutScreen = clamp_range(aboutScreen, enc, 0, 2);
  return aboutScreen;
}

bool tuneToMemory(const Memory *memory)
{
  // Must have frequency
  if(!memory->freq) return(false);

  // Must have valid band index
  if(memory->band>=getTotalBands()) return(false);

  // Band must contain frequency and modulation
  if(!isMemoryInBand(&bands[memory->band], memory)) return(false);

  // Must differ from the current band, frequency and modulation
  if(memory->band==bandIdx && memory->freq==bands[bandIdx].currentFreq && memory->mode==bands[bandIdx].bandMode)
    return(true);

  // Use default step when changing modes
  if(bands[memory->band].bandMode != memory->mode)
    bands[memory->band].currentStepIdx = defaultStepIdx[memory->mode];

  // Load frequency and modulation from memory slot
  bands[memory->band].currentFreq = memory->freq;
  bands[memory->band].bandMode    = memory->mode;

  // Enable the new band
  selectBand(memory->band);

  // Switching bands must also save pending edits to the previous band.
  prefsRequestSave(SAVE_SETTINGS | SAVE_BANDS);
  return(true);
}

// The active VFO follows live tuning; capture it before saving or switching.
VfoState vfos[2] = {};
uint8_t vfoIdx = 0;
bool vfoInitialized = false;

VfoState captureVfo()
{
  const Band *band = getCurrentBand();
  return {
    currentFrequency,
    (uint8_t)bandIdx, currentMode, band->currentStepIdx, getCurrentBandwidthIdx()
  };
}

static bool validVfo(const VfoState &vfo)
{
  if(vfo.band>=getTotalBands() || vfo.mode>=getTotalModes()) return(false);
  const Band &band = bands[vfo.band];
  if((vfo.mode==FM) != (band.bandType==FM_BAND_TYPE)) return(false);
  if(!isFreqInBand(&band, vfo.freq)) return(false);
  return(vfo.stepIdx>=0 && vfo.stepIdx<=getLastStep(vfo.mode) &&
         vfo.bandwidthIdx>=0 && vfo.bandwidthIdx<=getLastBandwidth(vfo.mode));
}

static void restoreVfo(bool drawLoadingPatch = true)
{
  const VfoState &vfo = vfos[vfoIdx];
  Band &band = bands[vfo.band];
  band.currentFreq = vfo.freq;
  band.bandMode = vfo.mode;
  band.currentStepIdx = vfo.stepIdx;
  band.bandwidthIdx = vfo.bandwidthIdx;
  selectBand(vfo.band, drawLoadingPatch);
}

void initVfo()
{
  if(vfoIdx<2 && validVfo(vfos[0]) && validVfo(vfos[1]))
    restoreVfo(false);
  else
  {
    selectBand(bandIdx, false);
    vfoIdx = 0;
    vfos[0] = vfos[1] = captureVfo();
    prefsRequestSave(SAVE_CUR_BAND);
  }
  vfoInitialized = true;
}

static void clickVfo(bool shortPress)
{
  vfos[vfoIdx] = captureVfo();
  if(shortPress)
    vfos[vfoIdx ^ 1] = vfos[vfoIdx];
  else
  {
    vfoIdx ^= 1;
    restoreVfo();
  }
  // Save both bands: they may contain pending edits from before the switch.
  prefsRequestSave(SAVE_SETTINGS | SAVE_BANDS);
}

static void doMemory(int16_t enc)
{
  memoryIdx = wrap_range(memoryIdx, enc, 0, getTotalMemories() - 1);
  Memory memory = getMemory(memoryIdx);
  if(!tuneToMemory(&memory)) tuneToMemory(&newMemory);
}

static void clickMemory(uint8_t idx, bool shortPress)
{
  // Must have a valid index
  if(idx>=getTotalMemories()) return;

  if(shortPress)
  {
    // If clicking on an empty memory slot, save to it
    Memory memory = getMemory(idx);
    if(!memory.freq) memory = newMemory;
    // Otherwise, delete memory slot contents
    else memory.freq = 0;
    setMemory(idx, memory);
    prefsRequestSave(SAVE_MEMORIES);
  }
  // On a click, do nothing, slot already activated in doMemory()
  else currentCmd = CMD_NONE;
}

void doStep(int16_t enc)
{
  uint8_t idx = bands[bandIdx].currentStepIdx;

  idx = wrap_range(idx, enc, 0, getLastStep(currentMode));
  bands[bandIdx].currentStepIdx = idx;

  rx.setFrequencyStep(isSSB()? 0 : freqFromHz(steps[currentMode][idx].step, currentMode));

  // Set seek spacing
  if(currentMode==FM)
    rx.setSeekFmSpacing(steps[currentMode][idx].spacing);
  else
    rx.setSeekAmSpacing(steps[currentMode][idx].spacing);
  if(enc) prefsRequestSave(SAVE_CUR_BAND);
}

//
// Automatic attenuation (RESONIK), AM only and only while AGC is On.
// Very strong signals overload the receiver front end and cause distortion
// and mixing products; the SI4732 AGC alone does not always prevent that.
// When the signal stays above ATT_HIGH, add attenuation in steps; when it
// stays below ATT_LOW (for example after tuning to a weak station), remove it.
//
#define ATT_HIGH       65   // dBuV: engage or increase attenuation above this
#define ATT_LOW        40   // dBuV: decrease attenuation below this
#define ATT_STEP        6   // Attenuation index step (SI4732 range 0..36)
#define ATT_MAX        36
#define ATT_STRONG_N    3   // Consecutive strong readings before acting (~1.2 s each)
#define ATT_WEAK_N      2   // Consecutive weak readings before releasing

uint8_t autoAttLevel = 0;
static uint8_t autoAttStrong = 0, autoAttWeak = 0;

void autoAttReset()
{
  if(autoAttLevel && currentMode == AM && !agcIdx)
    rx.setAutomaticGainControl(0, 0);
  autoAttLevel = 0;
  autoAttStrong = autoAttWeak = 0;
}

// Called with each new RSSI reading. Returns true if the display should update.
bool autoAttTick(uint8_t rssiNow)
{
  if(!autoAttIdx || currentMode != AM || agcIdx)
  {
    if(autoAttLevel) autoAttReset();
    return(false);
  }

  uint8_t level = autoAttLevel;
  autoAttStrong = rssiNow >= ATT_HIGH ? autoAttStrong + 1 : 0;
  autoAttWeak   = rssiNow <  ATT_LOW  ? autoAttWeak + 1 : 0;

  if(autoAttStrong >= ATT_STRONG_N && level < ATT_MAX)
  {
    level = level + ATT_STEP > ATT_MAX ? ATT_MAX : level + ATT_STEP;
    autoAttStrong = 0;
  }
  else if(autoAttWeak >= ATT_WEAK_N && level)
  {
    level = level > ATT_STEP ? level - ATT_STEP : 0;
    autoAttWeak = 0;
  }

  if(level == autoAttLevel) return(false);
  autoAttLevel = level;
  // AGC off with a fixed attenuation, or back to normal AGC
  rx.setAutomaticGainControl(level ? 1 : 0, level);
  return(true);
}

void doAgc(int16_t enc)
{
  if(currentMode==FM)
    agcIdx = FmAgcIdx = wrap_range(FmAgcIdx, enc, 0, 27);
  else if(isSSB())
    agcIdx = SsbAgcIdx = wrap_range(SsbAgcIdx, enc, 0, 1);
  else
    agcIdx = AmAgcIdx = wrap_range(AmAgcIdx, enc, 0, 37);

  // Process agcIdx to generate disableAgc and agcIdx
  // agcIdx     0 1 2 3 4 5 6  ..... n    (n:    FM = 27, AM = 37, SSB = 1)
  // agcNdx     0 0 1 2 3 4 5  ..... n -1 (n -1: FM = 26, AM = 36, SSB = 0)
  // disableAgc 0 1 1 1 1 1 1  ..... 1

  // if true, disable AGC; else, AGC is enabled
  disableAgc = agcIdx>0? 1 : 0;
  agcNdx     = agcIdx>1? agcIdx - 1 : 0;

  // Configure SI4732/5 (if agcNdx = 0, no attenuation)
  rx.setAutomaticGainControl(disableAgc, agcNdx);
  // The user's setting (or a new band/mode) takes over from automatic attenuation
  autoAttLevel = 0;
  autoAttStrong = autoAttWeak = 0;
  if(enc) prefsRequestSave(SAVE_SETTINGS);
}

void doMode(int16_t enc)
{
  // This is our current mode for the current band
  currentMode = bands[bandIdx].bandMode;

  // Cannot change away from FM mode
  if(currentMode==FM) return;

  // Change AM/LSB/USB/CW modes, do not allow FM mode
  do
    currentMode = wrap_range(currentMode, enc, 0, LAST_ITEM(bandModeDesc));
  while(currentMode==FM);

  // Save current band settings
  bands[bandIdx].currentStepIdx = defaultStepIdx[currentMode];
  bands[bandIdx].bandwidthIdx = defaultBwIdx[currentMode];
  bands[bandIdx].bandMode = currentMode;

  // Enable the new band
  selectBand(bandIdx);
  prefsRequestSave(SAVE_CUR_BAND);
}

void doSquelch(int16_t enc)
{
  uint8_t squelchParam = currentSquelch[currentMode] & 0x80;
  uint8_t squelchValue = currentSquelch[currentMode] & 0x7f;
  currentSquelch[currentMode] = squelchParam | clamp_range(squelchValue, enc, 0, 0x7f);
  if(enc) prefsRequestSave(SAVE_SETTINGS);
}

void doSoftMute(int16_t enc)
{
  // Nothing to do if FM mode
  if(currentMode==FM) return;

  if(isSSB())
    softMuteMaxAttIdx = SsbSoftMuteIdx = wrap_range(SsbSoftMuteIdx, enc, 0, 32);
  else
    softMuteMaxAttIdx = AmSoftMuteIdx = wrap_range(AmSoftMuteIdx, enc, 0, 32);

  rx.setAmSoftMuteMaxAttenuation(softMuteMaxAttIdx);
  if(enc) prefsRequestSave(SAVE_SETTINGS);
}

void doBand(int16_t enc)
{
  // Save current band settings
  bands[bandIdx].bandMode = currentMode;

  // Change band
  bandIdx = wrap_range(bandIdx, enc, 0, LAST_ITEM(bands));

  // Enable the new band
  selectBand(bandIdx);
  prefsRequestSave(SAVE_SETTINGS | SAVE_BANDS);
}

void doBandwidth(int16_t enc)
{
  uint8_t idx = bands[bandIdx].bandwidthIdx;

  idx = wrap_range(idx, enc, 0, getLastBandwidth(currentMode));
  bands[bandIdx].bandwidthIdx = idx;
  setBandwidth();
  // CW: the pitch follows the filter, so retune to keep the signal centred
  if(currentMode == CW) updateFrequency(currentFrequency, true);
  if(enc) prefsRequestSave(SAVE_CUR_BAND);
}

//
// Handle encoder input in menu
//

static void doMenu(int16_t enc)
{
  if(menuStyleIdx == MENU_STYLE_TILES)
    tileSlot = wrap_range(tileSlot, enc, 0, tileSlotCount(tileCat) - 1);
  else
    menuIdx = wrap_range(menuIdx, enc, 0, LAST_ITEM(menu));
}

static void clickMenuItem(int cmd, bool shortPress);
static void clickSettings(int cmd, bool shortPress);

static void clickMenu(int cmd, bool shortPress)
{
  if(menuStyleIdx != MENU_STYLE_TILES)
  {
    clickMenuItem(cmd, shortPress);
    return;
  }

  // Tile menu: a short press closes it from any page
  if(shortPress)
  {
    currentCmd = CMD_NONE;
    return;
  }

  // Top level: open the category, on its first item
  if(tileCat < 0)
  {
    tileCat = tileSlot;
    tileSlot = 1;
    return;
  }

  // Back: return to the top level on this category
  if(tileSlot % 6 == 0)
  {
    tileSlot = tileCat;
    tileCat = -1;
    return;
  }

  // An item: run it as the classic menu would
  int item = tileSlot / 6 * 5 + tileSlot % 6 - 1;
  uint8_t id = tileCategories[tileCat].items[item].id;
  tileValueItem = item;
  if(id & TILE_SET)
  {
    settingsIdx = id & ~TILE_SET;
    clickSettings(settingsIdx, false);
  }
  else
  {
    menuIdx = id;
    clickMenuItem(menuIdx, false);
  }

  // Items that do nothing in this mode stay on the page and say why
  if(currentCmd == CMD_NONE)
  {
    const char *why = nullptr;
    if(id == MENU_AVC || id == MENU_SOFTMUTE) why = currentMode == FM ? "Not used in FM" : nullptr;
    else if(id == (TILE_SET | MENU_CALIBRATION)) why = !isSSB() ? "For LSB, USB and CW" : nullptr;
    if(why)
    {
      statusShow(tileLabel(id), why);
      currentCmd = CMD_MENU;
      return;
    }
  }

  // Settings opened from the tiles come back to their page when done. Radio
  // controls (Band, Mode, Step, Volume...) are used on the normal screen
  // with its frequency and scale, and close the menu like the classic one.
  tileReturnCmd = !(id & TILE_SET) || currentCmd == CMD_MENU || currentCmd == CMD_NONE ? 0 : currentCmd;
}

static void clickMenuItem(int cmd, bool shortPress)
{
  // No command yet
  currentCmd = CMD_NONE;

  switch(cmd)
  {
    case MENU_STEP:     currentCmd = CMD_STEP;      break;
    case MENU_SEEK:     currentCmd = CMD_SEEK;      break;
    case MENU_MODE:     currentCmd = CMD_MODE;      break;
    case MENU_BW:       currentCmd = CMD_BANDWIDTH; break;
    case MENU_AGC_ATT:  currentCmd = CMD_AGC;       break;
    case MENU_BAND:     currentCmd = CMD_BAND;      break;
    case MENU_SETTINGS: currentCmd = CMD_SETTINGS;  break;
    case MENU_BEACONS:  currentCmd = CMD_BEACON;    break;
    case MENU_MEMSCAN:  memScanStart();             break;
    case MENU_SCANNER:  bandScanStart();            break;
    case MENU_SQUELCH:  currentCmd = CMD_SQUELCH;   break;
    case MENU_VOLUME:   currentCmd = CMD_VOLUME;    break;
    case MENU_VFO:      clickVfo(shortPress);       break;

    case MENU_MEMORY:
      currentCmd = CMD_MEMORY;
      newMemory.freq  = currentFrequency;
      newMemory.mode  = currentMode;
      newMemory.band  = bandIdx;
      doMemory(0);
      break;

    case MENU_SOFTMUTE:
      // No soft mute in FM mode
      if(currentMode!=FM) currentCmd = CMD_SOFTMUTE;
      break;

    case MENU_AVC:
      // No AVC in FM mode
      if(currentMode!=FM) currentCmd = CMD_AVC;
      break;

    case MENU_SCAN:
      // Run a band scan around current frequency with the same
      // step as scale resolution (10kHz for AM, 100kHz for FM)
      currentCmd = CMD_SCAN;
      clickScan(true);
      break;
  }
}

static void doSettings(int16_t enc)
{
  settingsIdx = wrap_range(settingsIdx, enc, 0, LAST_ITEM(settings));
}

static void clickSettings(int cmd, bool shortPress)
{
  // No command yet
  currentCmd = CMD_NONE;

  switch(cmd)
  {
    case MENU_BRIGHTNESS: currentCmd = CMD_BRT; break;
    case MENU_CALIBRATION:
      if(isSSB()) currentCmd = CMD_CAL;
      break;
    case MENU_THEME:      currentCmd = CMD_THEME;      break;
    case MENU_UI:         currentCmd = CMD_UI;         break;
    case MENU_RDS:        currentCmd = CMD_RDS;        break;
    case MENU_DATETIME:
      dateTimeInit();
      currentCmd = CMD_DATETIME;
      break;
    case MENU_ZOOM:       currentCmd = CMD_ZOOM;       break;
    case MENU_SCROLL:     currentCmd = CMD_SCROLL;     break;
    case MENU_OFFTIMER:
      if(shortPress) receiverShutdown();
      else currentCmd = CMD_OFFTIMER;
      break;
    case MENU_SLEEP:      currentCmd = CMD_SLEEP;      break;
    case MENU_SLEEPMODE:  currentCmd = CMD_SLEEPMODE;  break;
    case MENU_UTCOFFSET:  currentCmd = CMD_UTCOFFSET;  break;
    case MENU_USBMODE:    currentCmd = CMD_USBMODE;    break;
    case MENU_TCPMODE:    currentCmd = CMD_TCPMODE;    break;
    case MENU_BLEMODE:
      bleModeMenuIdx = bleModeIdx;
      currentCmd = CMD_BLEMODE;
      break;
    case MENU_WIFIMODE:   currentCmd = CMD_WIFIMODE;   break;
    case MENU_FM_REGION:  currentCmd = CMD_FM_REGION; break;
    case MENU_FM_STEREO:  currentCmd = CMD_FM_STEREO; break;
    case MENU_DSP_PATCHES: currentCmd = CMD_DSP_PATCHES; break;
    case MENU_ABOUT:      currentCmd = CMD_ABOUT;     break;
    case MENU_KNOB:       currentCmd = CMD_KNOB;      break;
    case MENU_BANDPLAN:   currentCmd = CMD_BANDPLAN;  break;
    case MENU_BPAREA:     currentCmd = CMD_BPAREA;    break;
    case MENU_AUTOATT:    currentCmd = CMD_AUTOATT;   break;
    case MENU_MENUSTYLE:  currentCmd = CMD_MENUSTYLE; break;
    case MENU_LOGGER:     currentCmd = CMD_LOGGER;    break;
    case MENU_CWDEC:      cwPageOpen();               break;
    case MENU_FT8:        ft8PageOpen();              break;
    case MENU_CWSTRIP:    currentCmd = CMD_CWSTRIP;   break;
    case MENU_PROPAG:
      if(getWiFiStatus() >= 2)
      {
        statusShow("Propagation", "Loading...", 0);
        drawScreen();
        if(!propagationFetch()) statusShow("Propagation", "No answer from hamqsl.com");
        else statusShow("");
      }
      currentCmd = CMD_PROPAG;
      break;
    case MENU_UPDATEFW:
      updateFwIdx = 0;
      currentCmd = CMD_UPDATEFW;
      break;

    case MENU_LOADEIBI:
      eibiLoadSchedule();
      break;
  }
}

// Status messages over the tiles, so warnings are not hidden by the menu
static void tileDrawStatus()
{
  if(!statusLines[0][0] && !statusLines[1][0]) return;
  bool two = statusLines[0][0] && statusLines[1][0];
  int h = two ? 44 : 26, y = 168 - h;
  spr.fillRoundRect(8, y, 304, h, 8, TILE_NAVY);
  spr.drawRoundRect(8, y, 304, h, 8, tileColors[2]);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(0xFFFF);
  if(two)
  {
    spr.drawString(statusLines[0], 160, y + 12, tileFont(false));
    spr.setTextColor(tileColors[2]);
    spr.drawString(statusLines[1], 160, y + 31, tileFont(true));
  }
  else spr.drawString(statusLines[0][0] ? statusLines[0] : statusLines[1], 160, y + h / 2, tileFont(false));
}

bool doSideBar(uint16_t cmd, int16_t enc, int16_t enca)
{
  // Ignore idle encoder
  if(!enc) return(false);

  switch(cmd)
  {
    // Menus and list-based options must take scrollDirection into account
    case CMD_MENU:       doMenu(scrollDirection * enc);break;
    case CMD_MODE:       doMode(scrollDirection * enc);break;
    case CMD_STEP:       doStep(scrollDirection * enc);break;
    case CMD_AGC:        doAgc(enc);break;
    case CMD_BANDWIDTH:  doBandwidth(scrollDirection * enc);break;
    case CMD_VOLUME:     doVolume(enca);break;
    case CMD_SOFTMUTE:   doSoftMute(enc);break;
    case CMD_BAND:       doBand(scrollDirection * enc);break;
    case CMD_AVC:        doAvc(enc);break;
    case CMD_FM_REGION:  doFmRegion(scrollDirection * enc);break;
    case CMD_FM_STEREO:  doFmStereo(scrollDirection * enc);break;
    case CMD_DSP_PATCHES: doDspPatches(scrollDirection * enc);break;
    case CMD_SETTINGS:   doSettings(scrollDirection * enc);break;
    case CMD_BRT:        doBrt(enca);break;
    case CMD_CAL:        doCal(enca);break;
    case CMD_THEME:      doTheme(scrollDirection * enc);break;
    case CMD_UI:         doUILayout(scrollDirection * enc);break;
    case CMD_BANDPLAN:   doBandPlan(scrollDirection * enc);break;
    case CMD_AUTOATT:    doAutoAtt(scrollDirection * enc);break;
    case CMD_MENUSTYLE:  doMenuStyle(scrollDirection * enc);break;
    case CMD_CWSTRIP:
      cwStripIdx = wrap_range(cwStripIdx, scrollDirection * enc, 0, 1);
      prefsRequestSave(SAVE_SETTINGS);
      break;
    case CMD_BPAREA:
      bandPlanArea = wrap_range(bandPlanArea, scrollDirection * enc, 0, BP_AREA_COUNT - 1);
      if(bandPlanArea == BP_AREA_IRAN) statusShow("Iran: Region 1 plan", "National table not yet confirmed");
      prefsRequestSave(SAVE_SETTINGS);
      break;
    case CMD_KNOB:
      // 20-click knobs count full steps, 30-click knobs half steps
      setEncoderHalfStep(!encoderHalfStep);
      prefsRequestSave(SAVE_SETTINGS);
      break;
    case CMD_LOGGER:
      loggerIdx = wrap_range(loggerIdx, scrollDirection * enc, 0, LOG_CLEAR);
      if(loggerIdx != LOG_CLEAR) prefsRequestSave(SAVE_SETTINGS);
      break;
    case CMD_BEACON:
      beaconFollow(wrap_range(beaconFollowIdx + 1, scrollDirection * enc, 0, 18) - 1);
      break;
    case CMD_PROPAG:     break;
    case CMD_RDS:        doRDSMode(scrollDirection * enc);break;
    case CMD_MEMORY:     doMemory(scrollDirection * enca);break;
    case CMD_OFFTIMER:   doOffTimer(enca);break;
    case CMD_SLEEP:      doSleep(enca);break;
    case CMD_SLEEPMODE:  doSleepMode(scrollDirection * enc);break;
    case CMD_USBMODE:    doUSBMode(scrollDirection * enc);break;
    case CMD_TCPMODE:    doTCPMode(scrollDirection * enc);break;
    case CMD_BLEMODE:    doBleMode(scrollDirection * enc);break;
    case CMD_WIFIMODE:   doWiFiMode(scrollDirection * enc);break;
    case CMD_ZOOM:       doZoom(enc);break;
    case CMD_SCROLL:     doScrollDir(enc);break;
    case CMD_UTCOFFSET:  doUTCOffset(scrollDirection * enc);break;
    case CMD_DATETIME:   doDateTime(enc);break;
    case CMD_SQUELCH:    doSquelch(enca);break;
    case CMD_UPDATEFW:   updateFwIdx = wrap_range(updateFwIdx, scrollDirection * enc, 0, LAST_ITEM(updateFwActions));break;
    case CMD_ABOUT:      doAbout(enc);break;
    default:             return(false);
  }

  // Encoder input handled
  return(true);
}

bool clickHandler(uint16_t cmd, bool shortPress)
{
  switch(cmd)
  {
    case CMD_MENU:     clickMenu(menuIdx, shortPress);break;
    case CMD_CWDEC:
      // CW decoder page: click clears the text, a short press leaves
      if(shortPress) currentCmd = CMD_NONE;
      else cwPageClear();
      break;
    case CMD_FT8:
      // FT8 page: click clears the list, a short press leaves
      if(shortPress) currentCmd = CMD_NONE;
      else ft8Clear();
      break;
    case CMD_SETTINGS: clickSettings(settingsIdx, shortPress);break;
    case CMD_UPDATEFW:
      // Click runs Check or Update; a short press leaves
      if(shortPress) currentCmd = CMD_NONE;
      else otaRequestLatest(updateFwIdx == 1);
      break;
    case CMD_MEMORY:   clickMemory(memoryIdx, shortPress);break;
    case CMD_BLEMODE:  clickBleMode(bleModeMenuIdx, shortPress);break;
    case CMD_WIFIMODE: clickWiFiMode(wifiModeIdx, shortPress);break;
    case CMD_OFFTIMER:
      if(shortPress)
      {
        offTimerMinutes = 0;
        offTimerRestart();
        prefsRequestSave(SAVE_SETTINGS);
      }
      else currentCmd = CMD_NONE;
      break;
    case CMD_VOLUME:   clickVolume(shortPress);break;
    case CMD_AVC:      clickAvc(shortPress);break;
    case CMD_SQUELCH:  clickSquelch(shortPress);break;
    case CMD_SEEK:     clickSeek(shortPress);break;
    case CMD_SCAN:     clickScan(shortPress);break;
    case CMD_FREQ:     return(clickFreq(shortPress));
    case CMD_DATETIME: clickDateTime(shortPress);break;
    case CMD_LOGGER:
      // "Clear log" acts on a click; the intervals apply as soon as they are chosen
      if(loggerIdx == LOG_CLEAR)
      {
        loggerClear();
        loggerIdx = 0;
        prefsRequestSave(SAVE_SETTINGS);
        statusShow("Signal log cleared");
      }
      currentCmd = CMD_NONE;
      break;
    default:           return(false);
  }

  // Encoder input handled
  return(true);
}

//
// Selecting given band
//

// Band plan segment of the last tuned frequency (for automatic sideband)
static const BandPlanSegment *bandPlanLastSeg = nullptr;

void selectBand(uint8_t idx, bool drawLoadingPatch)
{
  // Silence clicks during reconfiguration using the configured hardware mute method.
  // https://github.com/esp32-si4732/ats-mini/discussions/103
  muteOn(MUTE_TEMP, true, !audioClamp);

  // Set band and mode
  bandIdx = min(idx, LAST_ITEM(bands));
  currentMode = bands[bandIdx].bandMode;

  // Load the selected DSP patch as needed
  loadDSPPatch(getCurrentBandwidth()->idx, drawLoadingPatch);

  // Switch radio to the selected band
  useBand(&bands[bandIdx]);

  // Set bandwidth for the current mode
  setBandwidth();

  // Clear current station info (RDS/CB)
  clearStationInfo();

  // Check for named frequencies
  identifyFrequency(currentFrequency);

  // Set default digit position based on the current step
  resetFreqInputPos();

  muteOn(MUTE_TEMP, false, !audioClamp);

  // Band plan: entering a phone segment is measured from here
  bandPlanLastSeg = bandPlanFindArea(currentFrequency, bandPlanArea);
}

//
// Draw functions
//

static void drawCommon(const char *title, int x, int y, int sx, bool cursor = false)
{
  spr.setTextDatum(MC_DATUM);

  spr.setTextColor(TH.menu_hdr);
  spr.fillRoundRect(1+x, 1+y, 76+sx, 110, 4, TH.menu_bg);
  spr.drawRoundRect(1+x, 1+y, 76+sx, 110, 4, TH.menu_border);

  spr.drawString(title, 40+x+(sx/2), 12+y, FONT_SMALL);
  spr.drawLine(1+x, 23+y, 76+sx, 23+y, TH.menu_border);

  spr.setFont(FONT_DEFAULT);
  spr.setTextColor(TH.menu_item);
  if(cursor)
    spr.fillRoundRect(6+x, 24+y+(2*16), 66+sx, 16, 2, TH.menu_hl_bg);
}

static void drawMenu(int x, int y, int sx)
{
  spr.setTextDatum(MC_DATUM);

  spr.fillRoundRect(1+x, 1+y, 76+sx, 110, 4, TH.menu_bg);
  spr.drawRoundRect(1+x, 1+y, 76+sx, 110, 4, TH.menu_border);
  spr.setTextColor(TH.menu_hdr);

  spr.drawString("Menu", 40+x+(sx/2), 12+y, FONT_SMALL);
  spr.drawLine(1+x, 23+y, 76+sx, 23+y, TH.menu_border);

  spr.setFont(FONT_DEFAULT);
  spr.setTextColor(TH.menu_item);
  spr.fillRoundRect(6+x, 24+y+(2*16), 66+sx, 16, 2, TH.menu_hl_bg);

  int count = ITEM_COUNT(menu);
  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0) {
      drawZoomedMenu(menu[abs((menuIdx+count+i)%count)]);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }
    spr.setTextDatum(MC_DATUM);
    spr.drawString(menu[abs((menuIdx+count+i)%count)], 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawSettings(int x, int y, int sx)
{
  spr.setTextDatum(MC_DATUM);

  spr.fillRoundRect(1+x, 1+y, 76+sx, 110, 4, TH.menu_bg);
  spr.drawRoundRect(1+x, 1+y, 76+sx, 110, 4, TH.menu_border);
  spr.setTextColor(TH.menu_hdr);
  spr.drawString("Settings", 40+x+(sx/2), 12+y, FONT_SMALL);
  spr.drawLine(1+x, 23+y, 76+sx, 23+y, TH.menu_border);

  spr.setFont(FONT_DEFAULT);
  spr.setTextColor(TH.menu_item);
  spr.fillRoundRect(6+x, 24+y+(2*16), 66+sx, 16, 2, TH.menu_hl_bg);

  int count = ITEM_COUNT(settings);
  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0) {
      drawZoomedMenu(settings[abs((settingsIdx+count+i)%count)]);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(settings[abs((settingsIdx+count+i)%count)], 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawMode(int x, int y, int sx)
{
  drawCommon(menu[MENU_MODE], x, y, sx, true);

  // FM cannot be chosen on AM/SW bands, so the list cycles through the others
  static const uint8_t amModes[] = { LSB, USB, AM, CW };
  int count = ITEM_COUNT(amModes);
  int pos = 0;
  for(int j=0 ; j<count ; j++) if(amModes[j]==currentMode) pos = j;

  for(int i=-2 ; i<3 ; i++)
  {
    const char *label = currentMode==FM ? (i==0 ? bandModeDesc[FM] : nullptr)
                                        : bandModeDesc[amModes[(pos+count+i)%count]];
    if(i==0) {
      drawZoomedMenu(label);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    spr.setTextDatum(MC_DATUM);
    if(label) spr.drawString(label, 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawStep(int x, int y, int sx)
{
  int count = getLastStep(currentMode) + 1;
  int idx   = bands[bandIdx].currentStepIdx + count;

  drawCommon(menu[MENU_STEP], x, y, sx, true);

  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0) {
      drawZoomedMenu(steps[currentMode][abs((idx+i)%count)].desc);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(steps[currentMode][abs((idx+i)%count)].desc, 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawSeek(int x, int y, int sx)
{
  drawCommon(menu[MENU_SEEK], x, y, sx);
  spr.fillArc(40+x+(sx/2), 66+y, 30, 27, 135, 270, TH.menu_param);
  spr.fillTriangle(40+x+(sx/2)-5, 66+y-32, 40+x+(sx/2)+5, 66+y-27, 40+x+(sx/2)-5, 66+y-22, TH.menu_param);
  spr.fillArc(40+x+(sx/2), 66+y, 30, 27, 315, 450, TH.menu_param);
  spr.fillTriangle(40+x+(sx/2)+5, 66+y+32, 40+x+(sx/2)-5, 66+y+27, 40+x+(sx/2)+5, 66+y+22, TH.menu_param);

  if(seekMode()==SEEK_SCHEDULE)
  {
    spr.drawCircle(40+x+(sx/2), 66+y, 10, TH.menu_param);
    spr.drawLine(40+x+(sx/2), 66+y, 40+x+(sx/2), 66+y-7, TH.menu_param);
    spr.drawLine(40+x+(sx/2), 66+y, 40+x+(sx/2)+4, 66+y+4, TH.menu_param);
  }
}

static void drawScan(int x, int y, int sx)
{
  drawCommon(menu[MENU_SCAN], x, y, sx);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TH.scan_rssi);
  spr.drawString("S", 40+x+(sx/2)-30, 66+y+30, FONT_SMALL);
  spr.setTextColor(TH.scan_snr);
  spr.drawString("N", 40+x+(sx/2)+30, 66+y+30, FONT_SMALL);

  spr.fillArc(40+x+(sx/2), 66+y, 30, 27, 135, 270, TH.menu_param);
  spr.fillTriangle(40+x+(sx/2)-5, 66+y-32, 40+x+(sx/2)+5, 66+y-27, 40+x+(sx/2)-5, 66+y-22, TH.menu_param);
  spr.fillArc(40+x+(sx/2), 66+y, 30, 27, 315, 450, TH.menu_param);
  spr.fillTriangle(40+x+(sx/2)+5, 66+y+32, 40+x+(sx/2)-5, 66+y+27, 40+x+(sx/2)+5, 66+y+22, TH.menu_param);

  spr.drawLine(40+x+(sx/2)-17, 66+y+5, 40+x+(sx/2)-4, 66+y+5, TH.menu_param);
  spr.drawLine(40+x+(sx/2)-4, 66+y+5, 40+x+(sx/2), 66+y-16+5, TH.menu_param);
  spr.drawLine(40+x+(sx/2), 66+y-16+5, 40+x+(sx/2)+4, 66+y+5, TH.menu_param);
  spr.drawLine(40+x+(sx/2)+4, 66+y+5, 40+x+(sx/2)+17, 66+y+5, TH.menu_param);
}

static void drawBand(int x, int y, int sx)
{
  drawCommon(menu[MENU_BAND], x, y, sx, true);

  int count = ITEM_COUNT(bands);
  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0) {
      drawZoomedMenu(bands[abs((bandIdx+count+i)%count)].bandName);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(bands[abs((bandIdx+count+i)%count)].bandName, 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawBandwidth(int x, int y, int sx)
{
  int count = getLastBandwidth(currentMode) + 1;
  int idx   = bands[bandIdx].bandwidthIdx + count;

  drawCommon(menu[MENU_BW], x, y, sx, true);

  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0) {
      drawZoomedMenu(bandwidths[currentMode][abs((idx+i)%count)].desc);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(bandwidths[currentMode][abs((idx+i)%count)].desc, 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawSleepMode(int x, int y, int sx)
{
  drawCommon(settings[MENU_SLEEPMODE], x, y, sx, true);

  int count = ITEM_COUNT(sleepModeDesc);
  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0) {
      drawZoomedMenu(sleepModeDesc[abs((sleepModeIdx+count+i)%count)]);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(sleepModeDesc[abs((sleepModeIdx+count+i)%count)], 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawUSBMode(int x, int y, int sx)
{
  drawCommon(settings[MENU_USBMODE], x, y, sx, true);

  int count = ITEM_COUNT(usbModeDesc);
  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0) {
      drawZoomedMenu(usbModeDesc[abs((usbModeIdx+count+i)%count)]);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    // Prevent repeats for short menus
    if (count < 5 && ((usbModeIdx+i) < 0 || (usbModeIdx+i) >= count)) {
      continue;
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(usbModeDesc[abs((usbModeIdx+count+i)%count)], 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawTCPMode(int x, int y, int sx)
{
  drawCommon(settings[MENU_TCPMODE], x, y, sx, true);

  int count = ITEM_COUNT(tcpModeDesc);
  for(int i=0 ; i<count ; i++)
  {
    if(i == tcpModeIdx)
    {
      drawZoomedMenu(tcpModeDesc[i]);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    }
    else
      spr.setTextColor(TH.menu_item);

    spr.setTextDatum(MC_DATUM);
    spr.drawString(tcpModeDesc[i], 40+x+(sx/2), 64+y+((i-tcpModeIdx)*16), FONT_SMALL);
  }
}

static void drawBleMode(int x, int y, int sx)
{
  drawCommon(settings[MENU_BLEMODE], x, y, sx, true);

  int count = ITEM_COUNT(bleModeDesc);
  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0) {
      drawZoomedMenu(bleModeDesc[abs((bleModeMenuIdx+count+i)%count)]);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    // Prevent repeats for short menus
    if (count < 5 && ((bleModeMenuIdx+i) < 0 || (bleModeMenuIdx+i) >= count)) {
      continue;
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(bleModeDesc[abs((bleModeMenuIdx+count+i)%count)], 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawUpdateFW(int x, int y, int sx)
{
  drawCommon(settings[MENU_UPDATEFW], x, y, sx, true);

  for(int i=0 ; i<ITEM_COUNT(updateFwActions) ; i++)
  {
    if(i == updateFwIdx)
    {
      drawZoomedMenu(updateFwActions[i]);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    }
    else
      spr.setTextColor(TH.menu_item);

    spr.setTextDatum(MC_DATUM);
    spr.drawString(updateFwActions[i], 40+x+(sx/2), 64+y+((i-updateFwIdx)*16), FONT_SMALL);
  }
}

static void drawWiFiMode(int x, int y, int sx)
{
  drawCommon(settings[MENU_WIFIMODE], x, y, sx, true);

  int count = ITEM_COUNT(wifiModeDesc);
  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0) {
      drawZoomedMenu(wifiModeDesc[abs((wifiModeIdx+count+i)%count)]);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(wifiModeDesc[abs((wifiModeIdx+count+i)%count)], 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawTheme(int x, int y, int sx)
{
  drawCommon(settings[MENU_THEME], x, y, sx, true);

  int count = getTotalThemes();
  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0) {
      drawZoomedMenu(theme[abs((themeIdx+count+i)%count)].name);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(theme[abs((themeIdx+count+i)%count)].name, 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawUILayout(int x, int y, int sx)
{
  drawCommon(settings[MENU_UI], x, y, sx, true);

  int count = ITEM_COUNT(uiLayoutDesc);
  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0) {
      drawZoomedMenu(uiLayoutDesc[abs((uiLayoutIdx+count+i)%count)]);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    // Prevent repeats for short menus
    if (count < 5 && ((uiLayoutIdx+i) < 0 || (uiLayoutIdx+i) >= count)) {
      continue;
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(uiLayoutDesc[abs((uiLayoutIdx+count+i)%count)], 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

// Generic option list, as drawUILayout() (RESONIK settings)
static void drawOptionList(const char *title, const char *const *labels, int count, int idx, int x, int y, int sx)
{
  drawCommon(title, x, y, sx, true);

  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0) {
      drawZoomedMenu(labels[abs((idx+count+i)%count)]);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    // Prevent repeats for short menus
    if(count < 5 && ((idx+i) < 0 || (idx+i) >= count)) continue;

    spr.setTextDatum(MC_DATUM);
    spr.drawString(labels[abs((idx+count+i)%count)], 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawRDSMode(int x, int y, int sx)
{
  drawCommon(settings[MENU_RDS], x, y, sx, true);

  int count = ITEM_COUNT(rdsMode);
  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0) {
      drawZoomedMenu(rdsMode[abs((rdsModeIdx+count+i)%count)].desc);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(rdsMode[abs((rdsModeIdx+count+i)%count)].desc, 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawUTCOffset(int x, int y, int sx)
{
  drawCommon(settings[MENU_UTCOFFSET], x, y, sx, true);

  int count = ITEM_COUNT(timeZones);
  uint8_t idx = utcOffsetIdx;

  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0)
    {
      drawZoomedMenu(timeZones[abs((idx+count+i)%count)].desc);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    }
    else
    {
      spr.setTextColor(TH.menu_item);
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(timeZones[abs((idx+count+i)%count)].desc, 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawDateTimeHighlight(const char *text, uint8_t start, uint8_t length, int x, int y)
{
  char prefix[11];
  char value[5];
  snprintf(prefix, sizeof(prefix), "%.*s", start, text);
  snprintf(value, sizeof(value), "%.*s", length, text + start);

  int left = x - spr.textWidth(text, FONT_SMALL) / 2;
  int valueX = left + spr.textWidth(prefix, FONT_SMALL);
  int valueWidth = spr.textWidth(value, FONT_SMALL);

  spr.fillRoundRect(valueX - 1, y - 8, valueWidth + 2, 16, 2, TH.menu_hl_bg);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
  spr.drawString(value, valueX, y, FONT_SMALL);
}

static void drawDateTime(int x, int y, int sx)
{
  static const uint8_t starts[] = { 0, 5, 8, 0, 3, 6 };
  static const uint8_t lengths[] = { 4, 2, 2, 2, 2, 2 };
  char date[16];
  char time[16];

  if(!dateTimeEdited) dateTimeRefresh();

  snprintf(date, sizeof(date), "%04u-%02u-%02u",
    (unsigned)dateTimeYear, (unsigned)dateTimeMonth, (unsigned)dateTimeDay);
  snprintf(time, sizeof(time), "%02u:%02u:%02u",
    (unsigned)dateTimeHour, (unsigned)dateTimeMinute, (unsigned)dateTimeSecond);

  drawCommon(settings[MENU_DATETIME], x, y, sx);
  drawZoomedMenu(settings[MENU_DATETIME]);

  int center = 40 + x + sx / 2;
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TH.menu_item);
  spr.drawString(date, center, 41 + y, FONT_SMALL);
  spr.drawString(time, center, 64 + y, FONT_SMALL);
  spr.drawString("(UTC)", center, 91 + y, FONT_SMALL);

  drawDateTimeHighlight(
    dateTimeField <= DATETIME_DAY ? date : time,
    starts[dateTimeField], lengths[dateTimeField], center,
    (dateTimeField <= DATETIME_DAY ? 41 : 64) + y
  );
}

static void drawMemory(int x, int y, int sx)
{
  char label_memory[16];
  sprintf(label_memory, "%s %2.2d", menu[MENU_MEMORY], memoryIdx + 1);
  drawCommon(label_memory, x, y, sx, true);

  int count = getTotalMemories();
  for(int i=-2 ; i<3 ; i++)
  {
    int j = abs((memoryIdx+count+i)%count);
    const Memory memory = getMemory(j);
    char buf[16];
    const char *text = buf;

    if(!memory.freq)
      text = "- - -";
    else if(memory.name[0])
      text = memory.name;
    else if(memory.mode==FM)
      sprintf(buf, "%3.2f %s", memory.freq / 1000000.0, bandModeDesc[memory.mode]);
    else
      sprintf(buf, "%5lu %s", memory.freq / 1000, bandModeDesc[memory.mode]);

    if(i==0) {
      drawZoomedMenu(text);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(text, 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawVolume(int x, int y, int sx)
{
  drawCommon(menu[MENU_VOLUME], x, y, sx);
  drawZoomedMenu(menu[MENU_VOLUME]);
  spr.setTextDatum(MC_DATUM);

  spr.setTextColor(TH.menu_param);
  spr.drawNumber(volume, 40+x+(sx/2), 66+y, FONT_DIGITS);

  if(muteOn(MUTE_MAIN))
  {
    for(int i=-3; i<4; i++)
    {
      spr.drawLine(40+x+(sx/2) + 30 + i, 66 + y - 30 + i, 40+x+(sx/2) - 30 + i, 66 + y + 30 + i, TH.menu_param);
    }
  }
}

static void drawAgc(int x, int y, int sx)
{
  drawCommon(menu[MENU_AGC_ATT], x, y, sx);
  drawZoomedMenu(menu[MENU_AGC_ATT]);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TH.menu_param);

  // G8PTN: Read back value is not used
  // rx.getAutomaticGainControl();
  if(!agcNdx && !agcIdx)
  {
    spr.setFont(&lgfx::fonts::Orbitron_Light_24);
    spr.drawString("AGC", 40+x+(sx/2), 48+y);
    spr.drawString("On", 40+x+(sx/2), 72+y);
    spr.setFont(FONT_DEFAULT);
  }
  else
  {
    char text[16];
    sprintf(text, "%2.2d", agcNdx);
    spr.drawString(text, 40+x+(sx/2), 60+y, FONT_DIGITS);
  }
}

static void drawSquelch(int x, int y, int sx)
{
  drawCommon(menu[MENU_SQUELCH], x, y, sx);
  drawZoomedMenu(menu[MENU_SQUELCH]);
  spr.setTextDatum(MC_DATUM);

  uint8_t squelchValue = currentSquelch[currentMode] & 0x7f;
  bool squelchParam = currentSquelch[currentMode] & 0x80;
  if(squelchValue)
  {
    spr.drawNumber(squelchValue, 40+x+(sx/2), 60+y, FONT_LARGE);
    spr.drawString(squelchParam? "dB":"dBuV", 40+x+(sx/2), 90+y, FONT_LARGE);
  }
  else
  {
    spr.drawString("Off", 40+x+(sx/2), 60+y, FONT_LARGE);
    spr.drawString(squelchParam? "(snr)":"(rssi)", 40+x+(sx/2), 90+y, FONT_LARGE);
  }
}

static void drawSoftMuteMaxAtt(int x, int y, int sx)
{
  drawCommon(menu[MENU_SOFTMUTE], x, y, sx);
  drawZoomedMenu(menu[MENU_SOFTMUTE]);
  spr.setTextDatum(MC_DATUM);

  spr.setTextColor(TH.menu_param);
  spr.drawString("Max Attn", 40+x+(sx/2), 32+y, FONT_SMALL);
  spr.drawNumber(softMuteMaxAttIdx, 40+x+(sx/2), 60+y, FONT_LARGE);
  spr.drawString("dB", 40+x+(sx/2), 90+y, FONT_LARGE);
}

static void drawCal(int x, int y, int sx)
{
  drawCommon(settings[MENU_CALIBRATION], x, y, sx);
  drawZoomedMenu(settings[MENU_CALIBRATION]);
  spr.setTextDatum(MC_DATUM);

  spr.setTextColor(TH.menu_param);
  if (currentMode == USB || currentMode == CW)
  {
    spr.drawString("USB", 40+x+(sx/2), 35+y, FONT_SMALL);
    spr.drawNumber(getCurrentBand()->usbCal, 40+x+(sx/2), 65+y, FONT_LARGE);
  }
  else if (currentMode == LSB)
  {
    spr.drawString("LSB", 40+x+(sx/2), 35+y, FONT_SMALL);
    spr.drawNumber(getCurrentBand()->lsbCal, 40+x+(sx/2), 65+y, FONT_LARGE);
  }
  else
    spr.drawNumber(0, 40+x+(sx/2), 65+y, FONT_LARGE);  // Display zero or nothing for other modes

  spr.drawString("Hz", 40+x+(sx/2), 95+y, FONT_LARGE);
}

static void drawAvc(int x, int y, int sx)
{
  drawCommon(menu[MENU_AVC], x, y, sx);
  drawZoomedMenu(menu[MENU_AVC]);
  spr.setTextDatum(MC_DATUM);

  spr.setTextColor(TH.menu_param);
  if(isSSB() && ssbAvcHold)
  {
    spr.drawString("Hold", 40+x+(sx/2), 60+y, FONT_LARGE);
    return;
  }
  spr.drawString("Max Gain", 40+x+(sx/2), 32+y, FONT_SMALL);

  // Only show AVC for AM and SSB modes
  if(currentMode!=FM)
  {
    int currentAvc = isSSB()? SsbAvcIdx : AmAvcIdx;
    spr.drawNumber(currentAvc, 40+x+(sx/2), 60+y, FONT_LARGE);
    spr.drawString("dB", 40+x+(sx/2), 90+y, FONT_LARGE);
  }
}

static void drawFmRegion(int x, int y, int sx)
{
  drawCommon(settings[MENU_FM_REGION], x, y, sx, true);

  int count = ITEM_COUNT(fmRegions);
  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0) {
      drawZoomedMenu(fmRegions[abs((FmRegionIdx+count+i)%count)].desc);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    // Prevent repeats for short menus
    if (count < 5 && ((FmRegionIdx+i) < 0 || (FmRegionIdx+i) >= count)) {
      continue;
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(fmRegions[abs((FmRegionIdx+count+i)%count)].desc, 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawFmStereo(int x, int y, int sx)
{
  drawCommon(settings[MENU_FM_STEREO], x, y, sx, true);

  int count = ITEM_COUNT(fmStereoDesc);
  for(int i=-2 ; i<3 ; i++)
  {
    if(i==0) {
      drawZoomedMenu(fmStereoDesc[abs((fmStereoIdx+count+i)%count)]);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    } else {
      spr.setTextColor(TH.menu_item);
    }

    // Prevent repeats for short menus
    if (count < 5 && ((fmStereoIdx+i) < 0 || (fmStereoIdx+i) >= count)) {
      continue;
    }

    spr.setTextDatum(MC_DATUM);
    spr.drawString(fmStereoDesc[abs((fmStereoIdx+count+i)%count)], 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawDspPatches(int x, int y, int sx)
{
  drawCommon(settings[MENU_DSP_PATCHES], x, y, sx, true);
  uint8_t slots[PATCH_SET_COUNT + 1], selected;
  uint8_t count = dspPatchSlots(slots, selected);
  for(int i = -2; i < 3; i++)
  {
    int item = selected + i;
    if(item < 0 || item >= count) continue;
    uint8_t slot = slots[item];
    if(i == 0)
    {
      drawZoomedMenu(patchSetNames[slot]);
      spr.setTextColor(TH.menu_hl_text, TH.menu_hl_bg);
    }
    else spr.setTextColor(TH.menu_item);
    spr.setTextDatum(MC_DATUM);
    spr.drawString(patchSetNames[slot], 40+x+(sx/2), 64+y+(i*16), FONT_SMALL);
  }
}

static void drawBrt(int x, int y, int sx)
{
  drawCommon(settings[MENU_BRIGHTNESS], x, y, sx);
  drawZoomedMenu(settings[MENU_BRIGHTNESS]);
  spr.setTextDatum(MC_DATUM);

  spr.setTextColor(TH.menu_param);
  spr.drawNumber(currentBrt, 40+x+(sx/2), 60+y, FONT_LARGE);
}

static void drawOffTimer(int x, int y, int sx)
{
  drawCommon(settings[MENU_OFFTIMER], x, y, sx);
  drawZoomedMenu(settings[MENU_OFFTIMER]);
  spr.setTextDatum(MC_DATUM);
  spr.setTextColor(TH.menu_param);
  uint16_t minutes = offTimerMinutes;
  char text[16];
  if(minutes) snprintf(text, sizeof(text), "%u", minutes);
  else strlcpy(text, "Off", sizeof(text));
  spr.drawString(text, 40+x+(sx/2), 60+y, FONT_LARGE);
  if(minutes) spr.drawString("min", 40+x+(sx/2), 88+y, FONT_SMALL);
}

static void drawSleep(int x, int y, int sx)
{
  drawCommon(settings[MENU_SLEEP], x, y, sx);
  drawZoomedMenu(settings[MENU_SLEEP]);
  spr.setTextDatum(MC_DATUM);

  spr.setTextColor(TH.menu_param);
  if(currentSleep)
  {
    spr.drawNumber(currentSleep, 40+x+(sx/2), 60+y, FONT_LARGE);
    spr.drawString("sec", 40+x+(sx/2), 88+y, FONT_SMALL);
  }
  else
    spr.drawString("Off", 40+x+(sx/2), 60+y, FONT_LARGE);
}

static void drawZoom(int x, int y, int sx)
{
  drawCommon(settings[MENU_ZOOM], x, y, sx);
  drawZoomedMenu(settings[MENU_ZOOM]);
  spr.setTextDatum(MC_DATUM);

  spr.setTextColor(TH.menu_param);
  spr.drawString(zoomMenu ? "On" : "Off", 40+x+(sx/2), 60+y, FONT_LARGE);
}

static void drawScrollDir(int x, int y, int sx)
{
  drawCommon(settings[MENU_SCROLL], x, y, sx);
  drawZoomedMenu(settings[MENU_SCROLL]);

  spr.fillRect(37+x+(sx/2), 45+y, 5, 40, TH.menu_param);
  if(scrollDirection>0)
    spr.fillTriangle(39+x+(sx/2)-5, 45+y, 39+x+(sx/2)+5, 45+y, 39+x+(sx/2), 45+y-5, TH.menu_param);
  else
    spr.fillTriangle(39+x+(sx/2)-5, 85+y, 39+x+(sx/2)+5, 85+y, 39+x+(sx/2), 85+y+5, TH.menu_param);
}

static void drawInfo(int x, int y, int sx)
{
  char text[16];

  // Info box
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(TH.box_text);
  spr.fillRoundRect(1+x, 1+y, 76+sx, 110, 4, TH.box_bg);
  spr.drawRoundRect(1+x, 1+y, 76+sx, 110, 4, TH.box_border);

  spr.drawString("Step:", 6+x, 64+y+(-3*16), FONT_SMALL);
  spr.drawString(getCurrentStep()->desc, 48+x, 64+y+(-3*16), FONT_SMALL);

  spr.drawString("BW:", 6+x, 64+y+(-2*16), FONT_SMALL);
  spr.drawString(getCurrentBandwidth()->desc, 48+x, 64+y+(-2*16), FONT_SMALL);

  if(autoAttLevel && !agcIdx)
  {
    // Automatic attenuation active: "A" marks it as automatic
    sprintf(text, "A%2.2d", autoAttLevel);
    spr.drawString("Att:", 6+x, 64+y+(-1*16), FONT_SMALL);
    spr.drawString(text, 48+x, 64+y+(-1*16), FONT_SMALL);
  }
  else if(!agcNdx && !agcIdx)
  {
    spr.drawString("AGC:", 6+x, 64+y+(-1*16), FONT_SMALL);
    spr.drawString("On", 48+x, 64+y+(-1*16), FONT_SMALL);
  }
  else
  {
    sprintf(text, "%2.2d", agcNdx);
    spr.drawString("Att:", 6+x, 64+y+(-1*16), FONT_SMALL);
    spr.drawString(text, 48+x, 64+y+(-1*16), FONT_SMALL);
  }

  spr.drawString("Vol:", 6+x, 64+y+(0*16), FONT_SMALL);
  if(muteOn(MUTE_MAIN) || muteOn(MUTE_SQUELCH))
  {
    spr.setTextColor(TH.box_off_text, TH.box_off_bg);
    sprintf(text, muteOn(MUTE_MAIN) ? "Muted" : "%d/sq", volume);
    spr.drawString(text, 48+x, 64+y+(0*16), FONT_SMALL);
    spr.setTextColor(TH.box_text);
  }
  else
  {
    spr.setTextColor(TH.box_text);
    spr.drawNumber(volume, 48+x, 64+y+(0*16), FONT_SMALL);
  }

  // Draw RDS PI code, if present
  uint16_t piCode = getRdsPiCode();
  if(piCode && currentMode == FM)
  {
    sprintf(text, "%04X", piCode);
    spr.drawString("PI:", 6+x, 64+y + (1*16), FONT_SMALL);
    spr.drawString(text, 48+x, 64+y + (1*16), FONT_SMALL);
  }
  else
  {
    spr.drawString("AVC:", 6+x, 64+y + (1*16), FONT_SMALL);

    if(currentMode==FM)
      sprintf(text, "n/a");
    else if(isSSB() && ssbAvcHold)
      sprintf(text, "Hold");
    else if(isSSB())
      sprintf(text, "%2.2ddB", SsbAvcIdx);
    else
      sprintf(text, "%2.2ddB", AmAvcIdx);

    spr.drawString(text, 48+x, 64+y + (1*16), FONT_SMALL);
  }

  // Alternate the Off Timer countdown with the current date and time.
  // RESONIK dual clock: with a local zone other than UTC, every other
  // refresh shows UTC time (for schedules and logs) instead of local time.
  const char *clock = clockGet();
  uint16_t minutes = offTimerRemainingMinutes();
  uint32_t phase = millis() / BACKGROUND_REFRESH_TIME;
  bool showUtc = clock && getCurrentUTCOffset() && (minutes ? phase % 3 == 2 : phase % 2);
  if(minutes && (!clock || phase % 3 == 1))
  {
    snprintf(text, sizeof(text), "%02u:%02u", minutes / 60, minutes % 60);
    spr.drawString("Zzz:", 6+x, 64+y+(2*16), FONT_SMALL);
    spr.drawString(text, 48+x, 64+y+(2*16), FONT_SMALL);
  }
  else if(showUtc)
  {
    uint8_t hours, mins;
    if(clockGetHM(&hours, &mins))
    {
      snprintf(text, sizeof(text), "%02u:%02u", hours, mins);
      spr.drawString("UTC:", 6+x, 64+y+(2*16), FONT_SMALL);
      spr.drawString(text, 48+x, 64+y+(2*16), FONT_SMALL);
    }
  }
  else if(clock)
  {
    uint16_t year;
    uint8_t month, day, weekday;
    char date[6];
    const char *label = "Time:";

    if(clockGetDate(&year, &month, &day, &weekday))
    {
      if(month > 12 || day > 31) return;
      sprintf(date, "%02d.%02d", month, day);
      label = date;
    }

    spr.drawString(label, 6+x, 64+y+(2*16), FONT_SMALL);
    spr.drawString(clock, 48+x, 64+y+(2*16), FONT_SMALL);
  }
  else
  {
    // No time yet: say so, instead of an empty line (Wi-Fi Sync, RDS or Date/Time sets it)
    spr.drawString("Time:", 6+x, 64+y+(2*16), FONT_SMALL);
    spr.setTextColor(TH.box_off_text, TH.box_off_bg);
    spr.drawString("--:--", 48+x, 64+y+(2*16), FONT_SMALL);
    spr.setTextColor(TH.box_text);
  }
}

//
// Draw side bar (menu or information)
//
void drawSideBar(uint16_t cmd, int x, int y, int sx)
{
  if(sleepOn()) return;

  switch(cmd)
  {
    case CMD_MENU:       drawMenu(x, y, sx);       break;
    case CMD_SETTINGS:   drawSettings(x, y, sx);   break;
    case CMD_MODE:       drawMode(x, y, sx);       break;
    case CMD_STEP:       drawStep(x, y, sx);       break;
    case CMD_SEEK:       drawSeek(x, y, sx);       break;
    case CMD_SCAN:       drawScan(x, y, sx);       break;
    case CMD_BAND:       drawBand(x, y, sx);       break;
    case CMD_BANDWIDTH:  drawBandwidth(x, y, sx);  break;
    case CMD_THEME:      drawTheme(x, y, sx);      break;
    case CMD_UI:         drawUILayout(x, y, sx);   break;
    case CMD_BANDPLAN:   drawOptionList(settings[MENU_BANDPLAN], bandPlanDesc, ITEM_COUNT(bandPlanDesc), bandPlanIdx, x, y, sx); break;
    case CMD_AUTOATT:    drawOptionList(settings[MENU_AUTOATT], autoAttDesc, ITEM_COUNT(autoAttDesc), autoAttIdx, x, y, sx); break;
    case CMD_MENUSTYLE:  drawOptionList(settings[MENU_MENUSTYLE], menuStyleDesc, ITEM_COUNT(menuStyleDesc), menuStyleIdx, x, y, sx); break;
    case CMD_LOGGER:     drawOptionList(settings[MENU_LOGGER], loggerDesc, 5, loggerIdx, x, y, sx); break;
    case CMD_CWSTRIP:    drawOptionList(settings[MENU_CWSTRIP], cwStripDesc, 2, cwStripIdx, x, y, sx); break;
    case CMD_BPAREA:     drawOptionList(settings[MENU_BPAREA], bandPlanAreaDesc, BP_AREA_COUNT, bandPlanArea, x, y, sx); break;
    case CMD_KNOB:       drawOptionList(settings[MENU_KNOB], knobDesc, 2, encoderHalfStep ? 1 : 0, x, y, sx); break;
    case CMD_BEACON:     drawOptionList(menu[MENU_BEACONS], beaconMenuLabels, 19, beaconFollowIdx + 1, x, y, sx); break;
    case CMD_VOLUME:     drawVolume(x, y, sx);     break;
    case CMD_AGC:        drawAgc(x, y, sx);        break;
    case CMD_SOFTMUTE:   drawSoftMuteMaxAtt(x, y, sx);break;
    case CMD_CAL:        drawCal(x, y, sx);        break;
    case CMD_AVC:        drawAvc(x, y, sx);        break;
    case CMD_FM_REGION:  drawFmRegion(x, y, sx);   break;
    case CMD_FM_STEREO:  drawFmStereo(x, y, sx);   break;
    case CMD_DSP_PATCHES: drawDspPatches(x, y, sx); break;
    case CMD_BRT:        drawBrt(x, y, sx);        break;
    case CMD_RDS:        drawRDSMode(x, y, sx);    break;
    case CMD_MEMORY:     drawMemory(x, y, sx);     break;
    case CMD_OFFTIMER:   drawOffTimer(x, y, sx);   break;
    case CMD_SLEEP:      drawSleep(x, y, sx);      break;
    case CMD_SLEEPMODE:  drawSleepMode(x, y, sx);  break;
    case CMD_USBMODE:    drawUSBMode(x, y, sx);    break;
    case CMD_TCPMODE:    drawTCPMode(x, y, sx);    break;
    case CMD_BLEMODE:    drawBleMode(x, y, sx);    break;
    case CMD_WIFIMODE:   drawWiFiMode(x, y, sx);   break;
    case CMD_UPDATEFW:   drawUpdateFW(x, y, sx);   break;
    case CMD_ZOOM:       drawZoom(x, y, sx);       break;
    case CMD_SCROLL:     drawScrollDir(x, y, sx);  break;
    case CMD_UTCOFFSET:  drawUTCOffset(x, y, sx);  break;
    case CMD_DATETIME:   drawDateTime(x, y, sx);   break;
    case CMD_SQUELCH:    drawSquelch(x, y, sx);    break;
    default:             drawInfo(x, y, sx);       break;
  }
}

//
// Band plan: automatic setup (RESONIK). With Band Plan set to Auto, tuning
// into a segment of an amateur band sets the radio up for it: CW parts get
// CW with the 0.5k filter and 100 Hz steps, digital parts USB 3.0k, beacons
// CW 1.0k, phone parts LSB below 10 MHz or USB above with 2.2k and 1 kHz
// steps. Only on entering a segment, so a choice made by hand inside it
// sticks. AM and FM listening is left alone. Returns true if anything changed.
//
static int bandwidthIndexOf(const char *desc)
{
  for(int i = 0 ; i <= getLastBandwidth(LSB) ; i++) if(!strcmp(bandwidths[LSB][i].desc, desc)) return(i);
  return(-1);
}

bool bandPlanAutoSideband()
{
  const BandPlanSegment *seg = bandPlanFindArea(currentFrequency, bandPlanArea);
  bool entered = seg && seg != bandPlanLastSeg;
  bandPlanLastSeg = seg;

  if(bandPlanIdx != BANDPLAN_AUTO_SSB || !entered) return(false);
  if(seg->use == BP_BLOCKED)
  {
    statusShow("Band plan", "Not allowed here in this country");
    return(true);
  }
  if(currentMode != LSB && currentMode != USB && currentMode != CW) return(false);

  uint8_t mode;
  const char *bw;
  int stepHz;
  switch(seg->use)
  {
    case BP_CW:     mode = CW;  bw = "0.5k"; stepHz = 100;  break;
    case BP_BEACON: mode = CW;  bw = "1.0k"; stepHz = 100;  break;
    case BP_NARROW: mode = USB; bw = "3.0k"; stepHz = 1000; break;
    case BP_ALL:    mode = bandPlanPhoneIsLSB(currentFrequency) ? LSB : USB; bw = "2.2k"; stepHz = 1000; break;
    default:        return(false);
  }

  int bwIdx = bandwidthIndexOf(bw);
  int stepIdx = -1;
  for(int i = 0 ; i <= getLastStep(mode) ; i++) if(steps[mode][i].step == stepHz) stepIdx = i;
  Band &band = bands[bandIdx];
  if(mode == currentMode && (bwIdx < 0 || band.bandwidthIdx == bwIdx) && (stepIdx < 0 || band.currentStepIdx == stepIdx))
    return(false);

  band.bandMode = mode;
  band.currentFreq = currentFrequency;
  if(bwIdx >= 0) band.bandwidthIdx = bwIdx;
  if(stepIdx >= 0) band.currentStepIdx = stepIdx;
  selectBand(bandIdx, false);

  char line[32];
  snprintf(line, sizeof(line), "%s %s %s", bandPlanLabel(seg->use), bandModeDesc[mode], bw);
  statusShow("Band plan", line);
  prefsRequestSave(SAVE_CUR_BAND);
  return(true);
}

//
// FT8 page (RESONIK R3): the FT8 dial frequencies of the amateur bands in
// the band list (60 m is not in it), always USB with the 3.0k filter. The
// page saves the whole band table on opening and puts it back on leaving, so
// the radio returns exactly to where it was.
//
static const struct { const char *band; uint32_t freq; } ft8Channels[] =
{
  { "160M", 1840000 }, { "80M", 3573000 }, { "40M", 7074000 }, { "30M", 10136000 },
  { "20M", 14074000 }, { "17M", 18100000 }, { "15M", 21074000 }, { "12M", 24915000 },
  { "10M", 28074000 },
};

int ft8ChannelCount() { return(ITEM_COUNT(ft8Channels)); }
const char *ft8ChannelName(int ch) { return(ft8Channels[ch].band); }

static int bandIndexOf(const char *name)
{
  for(int i = 0 ; i < (int)ITEM_COUNT(bands) ; i++) if(!strcmp(bands[i].bandName, name)) return(i);
  return(-1);
}

int ft8ChannelOfCurrent()
{
  for(int i = 0 ; i < (int)ITEM_COUNT(ft8Channels) ; i++)
    if(!strcmp(ft8Channels[i].band, bands[bandIdx].bandName)) return(i);
  return(-1);
}

void ft8SelectChannel(int ch)
{
  int idx = bandIndexOf(ft8Channels[ch].band);
  if(idx < 0) return;
  Band &band = bands[idx];
  band.bandMode = USB;
  band.currentFreq = ft8Channels[ch].freq;
  int bw = bandwidthIndexOf("3.0k");
  if(bw >= 0) band.bandwidthIdx = bw;
  for(int i = 0 ; i <= getLastStep(USB) ; i++) if(steps[USB][i].step == 1000) band.currentStepIdx = i;
  selectBand(idx, false);
}

static Band ft8SavedBands[ITEM_COUNT(bands)];
static int ft8SavedIdx = -1;

void ft8SaveRadio()
{
  memcpy(ft8SavedBands, bands, sizeof(bands));
  ft8SavedIdx = bandIdx;
  // The current frequency lives in the band only once it is saved there
  ft8SavedBands[bandIdx].currentFreq = currentFrequency;
  ft8SavedBands[bandIdx].bandMode = currentMode;
}

void ft8RestoreRadio()
{
  if(ft8SavedIdx < 0) return;
  memcpy(bands, ft8SavedBands, sizeof(bands));
  selectBand(ft8SavedIdx, false);
  ft8SavedIdx = -1;
}

//
// Tile menu (RESONIK): the main menu and Settings as full-screen pages of
// large coloured tiles with icons, three by two, in the style of the
// Signal Mini design. The encoder moves the highlight in reading order and a
// click selects, exactly as in the classic menu. Drawing is in TileDraw.h.
//
const lgfx::IFont *uiFont(bool small) { return(tileFont(small)); }
//
// Tile value screens (RESONIK): a setting opened from the tiles is shown
// full screen in the same style, as a list of values or as a number. Band,
// Memory, Seek, Scan, Date/Time and DSP Patches keep their classic panels.
//
typedef struct
{
  bool list;          // A list of values, else a number
  int count, idx;     // List: number of values and the one in use
  char value[16];     // Number: the value as shown
  const char *unit;   // Number: unit or a second line, may be nullptr
  int pct;            // Number: bar position 0..100, -1 for no bar
} TileValue;

static const uint8_t tileAmModes[] = { LSB, USB, AM, CW };

static const char *tileValueLabel(uint16_t cmd, int i)
{
  switch(cmd)
  {
    case CMD_MODE:      return(currentMode == FM ? bandModeDesc[FM] : bandModeDesc[tileAmModes[i]]);
    case CMD_STEP:      return(steps[currentMode][i].desc);
    case CMD_BANDWIDTH: return(bandwidths[currentMode][i].desc);
    case CMD_FM_REGION: return(fmRegions[i].desc);
    case CMD_RDS:       return(rdsMode[i].desc);
    case CMD_THEME:     return(theme[i].name);
    case CMD_UI:        return(uiLayoutDesc[i]);
    case CMD_FM_STEREO: return(fmStereoDesc[i]);
    case CMD_BANDPLAN:  return(bandPlanDesc[i]);
    case CMD_AUTOATT:   return(autoAttDesc[i]);
    case CMD_MENUSTYLE: return(menuStyleDesc[i]);
    case CMD_LOGGER:    return(loggerDesc[i]);
    case CMD_BEACON:    return(beaconMenuLabels[i]);
    case CMD_SLEEPMODE: return(sleepModeDesc[i]);
    case CMD_USBMODE:   return(usbModeDesc[i]);
    case CMD_TCPMODE:   return(tcpModeDesc[i]);
    case CMD_BLEMODE:   return(bleModeDesc[i]);
    case CMD_WIFIMODE:  return(wifiModeDesc[i]);
    case CMD_UPDATEFW:  return(updateFwActions[i]);
    case CMD_UTCOFFSET: return(timeZones[i].desc);
    case CMD_ZOOM:      return(i ? "On" : "Off");
    case CMD_SCROLL:    return(i ? "Reversed" : "Normal");
    case CMD_CWSTRIP:   return(cwStripDesc[i]);
    case CMD_KNOB:      return(knobDesc[i]);
    case CMD_BPAREA:    return(bandPlanAreaDesc[i]);
    default:            return("");
  }
}

static bool tileValueGet(uint16_t cmd, TileValue &v)
{
  v.list = true;
  v.unit = nullptr;
  v.pct = -1;
  v.value[0] = '\0';
  switch(cmd)
  {
    case CMD_MODE:
      v.count = currentMode == FM ? 1 : ITEM_COUNT(tileAmModes);
      v.idx = 0;
      for(int j = 0 ; currentMode != FM && j < v.count ; j++) if(tileAmModes[j] == currentMode) v.idx = j;
      return(true);
    case CMD_STEP:      v.count = getLastStep(currentMode) + 1; v.idx = bands[bandIdx].currentStepIdx; return(true);
    case CMD_BANDWIDTH: v.count = getLastBandwidth(currentMode) + 1; v.idx = bands[bandIdx].bandwidthIdx; return(true);
    case CMD_FM_REGION: v.count = ITEM_COUNT(fmRegions); v.idx = FmRegionIdx; return(true);
    case CMD_RDS:       v.count = ITEM_COUNT(rdsMode); v.idx = rdsModeIdx; return(true);
    case CMD_THEME:     v.count = getTotalThemes(); v.idx = themeIdx; return(true);
    case CMD_UI:        v.count = ITEM_COUNT(uiLayoutDesc); v.idx = uiLayoutIdx; return(true);
    case CMD_FM_STEREO: v.count = ITEM_COUNT(fmStereoDesc); v.idx = fmStereoIdx; return(true);
    case CMD_BANDPLAN:  v.count = ITEM_COUNT(bandPlanDesc); v.idx = bandPlanIdx; return(true);
    case CMD_AUTOATT:   v.count = ITEM_COUNT(autoAttDesc); v.idx = autoAttIdx; return(true);
    case CMD_MENUSTYLE: v.count = ITEM_COUNT(menuStyleDesc); v.idx = menuStyleIdx; return(true);
    case CMD_LOGGER:    v.count = 5; v.idx = loggerIdx; return(true);
    case CMD_BEACON:    v.count = 19; v.idx = beaconFollowIdx + 1; return(true);
    case CMD_SLEEPMODE: v.count = ITEM_COUNT(sleepModeDesc); v.idx = sleepModeIdx; return(true);
    case CMD_USBMODE:   v.count = ITEM_COUNT(usbModeDesc); v.idx = usbModeIdx; return(true);
    case CMD_TCPMODE:   v.count = ITEM_COUNT(tcpModeDesc); v.idx = tcpModeIdx; return(true);
    case CMD_BLEMODE:   v.count = ITEM_COUNT(bleModeDesc); v.idx = bleModeMenuIdx; return(true);
    case CMD_WIFIMODE:  v.count = ITEM_COUNT(wifiModeDesc); v.idx = wifiModeIdx; return(true);
    case CMD_UPDATEFW:  v.count = ITEM_COUNT(updateFwActions); v.idx = updateFwIdx; return(true);
    case CMD_UTCOFFSET: v.count = ITEM_COUNT(timeZones); v.idx = utcOffsetIdx; return(true);
    case CMD_ZOOM:      v.count = 2; v.idx = zoomMenu ? 1 : 0; return(true);
    case CMD_SCROLL:    v.count = 2; v.idx = scrollDirection > 0 ? 0 : 1; return(true);
    case CMD_CWSTRIP:   v.count = 2; v.idx = cwStripIdx; return(true);
    case CMD_BPAREA:    v.count = BP_AREA_COUNT; v.idx = bandPlanArea; return(true);
    case CMD_KNOB:      v.count = 2; v.idx = encoderHalfStep ? 1 : 0; return(true);
  }

  v.list = false;
  switch(cmd)
  {
    case CMD_VOLUME:
      snprintf(v.value, sizeof(v.value), "%d", volume);
      v.unit = muteOn(MUTE_MAIN) ? "Muted" : nullptr;
      v.pct = volume * 100 / 63;
      return(true);
    case CMD_BRT:
      snprintf(v.value, sizeof(v.value), "%d", currentBrt);
      v.pct = (currentBrt - 10) * 100 / 245;
      return(true);
    case CMD_SQUELCH:
    {
      uint8_t sq = currentSquelch[currentMode] & 0x7f;
      bool snrSq = currentSquelch[currentMode] & 0x80;
      if(sq) snprintf(v.value, sizeof(v.value), "%d", sq); else strcpy(v.value, "Off");
      v.unit = sq ? (snrSq ? "dB SNR" : "dBuV") : (snrSq ? "SNR" : "RSSI");
      return(true);
    }
    case CMD_SOFTMUTE:
      snprintf(v.value, sizeof(v.value), "%d", softMuteMaxAttIdx);
      v.unit = "dB max attenuation";
      v.pct = softMuteMaxAttIdx * 100 / 32;
      return(true);
    case CMD_AVC:
      if(isSSB() && ssbAvcHold) { strcpy(v.value, "Hold"); return(true); }
      snprintf(v.value, sizeof(v.value), "%d", isSSB() ? SsbAvcIdx : AmAvcIdx);
      v.unit = "dB max gain";
      return(true);
    case CMD_AGC:
      if(!agcNdx && !agcIdx) strcpy(v.value, "AGC On");
      else { snprintf(v.value, sizeof(v.value), "%d", agcNdx); v.unit = "Attenuation"; }
      return(true);
    case CMD_CAL:
      snprintf(v.value, sizeof(v.value), "%d", currentMode == LSB ? getCurrentBand()->lsbCal :
               (currentMode == USB || currentMode == CW) ? getCurrentBand()->usbCal : 0);
      v.unit = currentMode == LSB ? "Hz LSB" : "Hz USB";
      return(true);
    case CMD_OFFTIMER:
      if(offTimerMinutes) { snprintf(v.value, sizeof(v.value), "%u", offTimerMinutes); v.unit = "min"; }
      else strcpy(v.value, "Off");
      v.pct = offTimerMinutes * 100 / 240;
      return(true);
    case CMD_SLEEP:
      if(currentSleep) { snprintf(v.value, sizeof(v.value), "%u", currentSleep); v.unit = "s"; }
      else strcpy(v.value, "Off");
      v.pct = currentSleep * 100 / 255;
      return(true);
  }
  return(false);
}

static void tileDrawValue(uint16_t cmd, const TileValue &v)
{
  const TileCategory &c = tileCategories[tileCat];
  const TileEntry &e = c.items[tileValueItem];
  int ci = (tileValueItem + tileCat * 3 + 1) % 5;
  uint16_t col = tileColors[ci];
  bool light = ci >= 1 && ci <= 3;
  uint16_t ink = light ? TILE_NAVY : 0xFFFF;

  spr.fillSprite(0x0000);

  // Header in the tile's own colour, with its icon and the category
  spr.fillRoundRect(6, 6, 308, 40, 10, col);
  static const int8_t d[8][2] = {{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{1,-1},{-1,1},{1,1}};
  for(int j = 0 ; j < 8 ; j++) tileDrawIcon(spr, e.icon, 30 + d[j][0], 26 + d[j][1], TILE_NAVY, col);
  tileDrawIconColor(spr, e.icon, 30, 26, col);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(ink);
  spr.drawString(tileLabel(e.id), 58, 26, tileFont(false));
  spr.setTextDatum(MR_DATUM);
  spr.drawString(c.label, 304, 26, tileFont(true));

  const uint16_t muted = TILE_RGB(0x8F, 0xB3, 0xC0);
  if(v.list)
  {
    // Three rows; the highlighted one pops out in the tile's colour
    const int top = 56, rh = 24, vis = 3;
    int first = v.idx < 1 ? 0 : v.idx - 1;
    if(first > v.count - vis) first = v.count - vis;
    if(first < 0) first = 0;
    for(int k = 0 ; k < vis && first + k < v.count ; k++)
    {
      int i = first + k, y = top + k * (rh + 4);
      bool hl = i == v.idx;
      if(hl) spr.fillRoundRect(6, y - 2, 308, rh + 4, 8, col);
      else spr.fillRoundRect(10, y, 300, rh, 8, TILE_NAVY);
      spr.setTextDatum(ML_DATUM);
      spr.setTextColor(hl ? ink : 0xFFFF);
      spr.drawString(tileValueLabel(cmd, i), 24, y + rh / 2, tileFont(false));
    }
    if(v.count > vis)
    {
      int h = 3 * 28, bh = h * vis / v.count;
      int by = top + (h - bh) * first / (v.count - vis);
      spr.fillRoundRect(316, top, 3, h, 1, TILE_NAVY);
      spr.fillRoundRect(316, by, 3, bh, 1, 0xFFFF);
    }
  }
  else
  {
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(0xFFFF);
    spr.drawString(v.value, 160, v.pct >= 0 ? 84 : 92, &lgfx::fonts::FreeSansBold24pt7b);
    if(v.unit)
    {
      spr.setTextColor(muted);
      spr.drawString(v.unit, 160, v.pct >= 0 ? 112 : 130, tileFont(false));
    }
    if(v.pct >= 0)
    {
      int p = v.pct < 0 ? 0 : v.pct > 100 ? 100 : v.pct;
      spr.fillRoundRect(20, 128, 280, 12, 6, TILE_NAVY);
      if(p) spr.fillRoundRect(20, 128, 280 * p / 100 < 12 ? 12 : 280 * p / 100, 12, 6, tileColors[3]);
    }
  }

  spr.setTextDatum(BC_DATUM);
  spr.setTextColor(muted);
  spr.drawString(v.list ? "Turn: choose   Click: done" : "Turn: adjust   Click: done", 160, 168, tileFont(true));
}

bool drawTileMenu()
{
  if(menuStyleIdx != MENU_STYLE_TILES) return(false);

  // A setting opened from the tiles: its value screen
  TileValue v;
  if(tileReturnCmd && currentCmd == tileReturnCmd && tileCat >= 0 && tileValueGet(currentCmd, v))
  {
    tileDrawValue(currentCmd, v);
    tileDrawStatus();
    return(true);
  }

  if(currentCmd != CMD_MENU) return(false);

  TileItem items[6];
  int count = 0, sel = 0, page = 0, pages = 1;

  if(tileCat < 0)
  {
    // Top level: one colour per category
    for(int k = 0 ; k < (int)ITEM_COUNT(tileCategories) ; k++)
      items[count++] = tileMake(tileCategories[k].label, tileCategories[k].icon, k);
    sel = tileSlot;
  }
  else
  {
    // A category page: Back, then up to five items in the five bright colours
    const TileCategory &c = tileCategories[tileCat];
    page = tileSlot / 6;
    pages = (c.count + 4) / 5;
    sel = tileSlot % 6;
    items[count++] = tileMake("Back", TI_BACK, 5);
    for(int i = page * 5 ; i < c.count && count < 6 ; i++)
      items[count++] = tileMake(tileLabel(c.items[i].id), c.items[i].icon, (i + tileCat * 3 + 1) % 5);
  }

  // Always on black, whatever the colour theme, so the tile colours read the same
  tileDrawPage(spr, items, count, sel, 0x0000, 0x7BEF, tileFont(false), false,
               &tileStyle, page, pages, tileFont(true));
  tileDrawStatus();
  return(true);
}
