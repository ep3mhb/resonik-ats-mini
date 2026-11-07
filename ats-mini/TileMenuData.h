#ifndef TILE_MENU_DATA_H
#define TILE_MENU_DATA_H

// Icons for the tile menu, in the order of menu[] and settings[] in Menu.cpp
#include "TileDraw.h"

static const TileIcon tileMainIcons[] =
{
  TI_MODE, TI_BAND, TI_VOLUME, TI_VFO, TI_STEP, TI_SEEK, TI_SCAN, TI_MEMORY,
  TI_SQUELCH, TI_BANDWIDTH, TI_AGC, TI_AVC, TI_SOFTMUTE, TI_BEACON, TI_MEMSCAN, TI_SETTINGS,
};

static const TileIcon tileSettingsIcons[] =
{
  TI_BRIGHT, TI_CLOCK, TI_GLOBE, TI_MOON, TI_MOON, TI_POWER, TI_PALETTE, TI_LAYOUT,
  TI_ZOOM, TI_SCROLL, TI_RDS, TI_STEREO, TI_REGION, TI_DIAL, TI_CHIP, TI_DOWNLOAD,
  TI_UPLOAD, TI_USB, TI_NETWORK, TI_BT, TI_WIFI, TI_BANDPLAN, TI_ATT, TI_TILES,
  TI_LOG, TI_PROP, TI_INFO,
};

// Colours in a fixed order, so neighbouring tiles differ like the reference design
static inline uint16_t tileColorFor(int i)
{
  static const uint8_t order[6] = { 0, 1, 2, 3, 6, 4 };
  return(tilePalette[(order[i % 6] + (i / 6) * 2) % 8]);
}

#endif // TILE_MENU_DATA_H
