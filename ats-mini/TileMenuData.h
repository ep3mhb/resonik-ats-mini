#ifndef TILE_MENU_DATA_H
#define TILE_MENU_DATA_H

// Tile menu (RESONIK): colours, fonts and style. The category lists live in
// Menu.cpp next to the menu item numbers they use.
#include "TileDraw.h"
#include "FontInter15.h"
#include "FontInter14.h"

// Palette: pink, coral, yellow, green, blue, and deep navy for Back tiles,
// icon outlines and dark labels. Used exactly as given, never shaded.
#define TILE_NAVY  TILE_RGB(0x07, 0x3A, 0x4B)
static const uint16_t tileColors[6] =
{
  TILE_RGB(0xF0, 0x47, 0x70),   // Pink
  TILE_RGB(0xF7, 0x8C, 0x6A),   // Coral
  TILE_RGB(0xFF, 0xD1, 0x67),   // Yellow
  TILE_RGB(0x06, 0xD7, 0xA0),   // Green
  TILE_RGB(0x10, 0x8A, 0xB1),   // Blue
  TILE_NAVY,                    // Deep navy
};

// Light tiles carry navy labels; white is unreadable on them
static inline TileItem tileMake(const char *label, TileIcon icon, int ci)
{
  bool light = ci >= 1 && ci <= 3;
  return((TileItem){ label, icon, tileColors[ci], 0, (uint16_t)(light ? TILE_NAVY : 0) });
}

// Solid tiles, no outline or dimming, full-colour icons with a navy outline,
// and the selected tile pops out by four pixels
static const TileStyle tileStyle = { false, 0, false, 4, 0, 4, TILE_NAVY };

// Inter SemiBold smooth fonts: 15 px, and 14 px for labels too long to fit
static const lgfx::IFont *tileFont(bool small)
{
  static lgfx::PointerWrapper w15(f_Inter15, sizeof(f_Inter15));
  static lgfx::PointerWrapper w14(f_Inter14, sizeof(f_Inter14));
  static lgfx::VLWfont font15, font14;
  static bool loaded = false;
  if(!loaded)
  {
    font15.loadFont(&w15);
    font14.loadFont(&w14);
    loaded = true;
  }
  return(small ? &font14 : &font15);
}

#endif // TILE_MENU_DATA_H
