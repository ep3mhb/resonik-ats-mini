#ifndef TILE_DRAW_H
#define TILE_DRAW_H

// Tile menu drawing (RESONIK): large coloured tiles with a white icon and a
// label, three by two to a page, page dots at the bottom. Only LovyanGFX
// calls, so the same code renders previews on a PC (tools/tile_preview.cpp).

#include <LovyanGFX.hpp>
#include <ctype.h>

#define TILE_RGB(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))

enum TileIcon : uint8_t
{
  TI_NONE, TI_MODE, TI_BAND, TI_VOLUME, TI_VFO, TI_STEP, TI_SEEK, TI_SCAN, TI_MEMORY,
  TI_SQUELCH, TI_BANDWIDTH, TI_AGC, TI_AVC, TI_SOFTMUTE, TI_BEACON, TI_MEMSCAN, TI_SETTINGS,
  TI_BRIGHT, TI_CLOCK, TI_GLOBE, TI_MOON, TI_POWER, TI_PALETTE, TI_LAYOUT, TI_ZOOM, TI_SCROLL,
  TI_RDS, TI_STEREO, TI_DIAL, TI_CHIP, TI_DOWNLOAD, TI_FIRMWARE, TI_USB, TI_NETWORK, TI_BT,
  TI_WIFI, TI_BANDPLAN, TI_ATT, TI_TILES, TI_LOG, TI_PROP, TI_INFO, TI_REGION,
  TI_BACK, TI_CLOSE, TI_SLEEPTIMER, TI_FORK,
};

typedef struct
{
  const char *label;
  TileIcon icon;
  uint16_t color;      // Tile colour (bottom colour when color2 is set)
  uint16_t color2;     // Optional top colour for a vertical gradient, 0 = solid
  uint16_t text;       // Optional label colour for light tiles, 0 = white
  bool paletteIcon;    // Paint the icon in the style's accent palette
} TileItem;

// Tile colours, picked to sit well on the dark background with white text
static const uint16_t tilePalette[8] =
{
  TILE_RGB(124, 179,  66),   // Green
  TILE_RGB(126,  87, 194),   // Purple
  TILE_RGB( 30, 136, 229),   // Blue
  TILE_RGB(  0, 150, 136),   // Teal
  TILE_RGB(239, 108,   0),   // Orange
  TILE_RGB(216,  27,  96),   // Pink
  TILE_RGB( 57,  73, 171),   // Indigo
  TILE_RGB(  3, 155, 229),   // Light blue
};

static inline uint16_t tileBlend(uint16_t c, uint16_t w, uint8_t a)
{
  uint32_t r = ((c >> 11) & 31) * (255 - a) + ((w >> 11) & 31) * a;
  uint32_t g = ((c >> 5) & 63) * (255 - a) + ((w >> 5) & 63) * a;
  uint32_t b = (c & 31) * (255 - a) + (w & 31) * a;
  return((uint16_t)(((r / 255) << 11) | ((g / 255) << 5) | (b / 255)));
}

// Accent colours of the full-colour icons (see tileDrawIconColor)
typedef struct
{
  uint16_t w, y, o, r, g, c, b, p, k;
} TileAccents;

// Optional styles: outline or dimming to mark the selection, icon badge
typedef struct
{
  bool    outline;     // White outline around the selected tile
  uint8_t dimOthers;   // 0..255: darken the tiles that are not selected
  bool    badge;       // Icon painted in tile colour on a white disc
  uint8_t iconMode;    // 0 white, 1 tint, 2 tint with shadow, 3 engraved, 4 full colour
  uint8_t labelShadow; // 0 = none, else strength of a dark shadow under the label
  uint8_t pop;         // Pixels the selected tile grows by on each side
  uint16_t iconOutline; // Colour of a thin outline around full-colour icons, 0 = soft shadow
  const TileAccents *accents; // Icon accent colours, nullptr = the default set
} TileStyle;

// Rounded rectangle with a vertical gradient from top to bottom colour
template<typename S>
static void tileFillGradient(S &s, int x, int y, int w, int h, int r, uint16_t top, uint16_t bottom)
{
  for(int i = 0 ; i < h ; i++)
  {
    int inset = 0;
    int d = i < r ? r - i : (i >= h - r ? i - (h - r - 1) : 0);
    if(d > 0) inset = r - (int)sqrtf((float)(r * r - (d - 0.5f) * (d - 0.5f)));
    uint16_t c = tileBlend(bottom, top, (uint8_t)(255 - i * 255 / (h - 1)));
    s.drawFastHLine(x + inset, y + i, w - 2 * inset, c);
  }
}

// Icons are drawn in a 26x26 box centred on (x, y), in colour c
template<typename S>
static void tileDrawIcon(S &s, TileIcon icon, int x, int y, uint16_t c, uint16_t bg)
{
  switch(icon)
  {
    case TI_MODE:        // Sine wave
      for(int i = -12 ; i < 12 ; i++)
      {
        int y0 = y - (int)(8 * sinf(i * 0.27f)), y1 = y - (int)(8 * sinf((i + 1) * 0.27f));
        s.drawWideLine(x + i, y0, x + i + 1, y1, 1.2f, c);
      }
      break;
    case TI_BAND:        // Signal bars
      for(int i = 0 ; i < 5 ; i++) s.fillRoundRect(x - 12 + i * 5, y + 10 - i * 5, 4, 2 + i * 5, 1, c);
      break;
    case TI_VOLUME:      // Speaker and waves
    case TI_SOFTMUTE:
      s.fillRect(x - 11, y - 4, 5, 8, c);
      s.fillTriangle(x - 7, y, x - 1, y - 9, x - 1, y + 9, c);
      if(icon == TI_VOLUME)
      {
        s.drawArc(x - 1, y, 6, 4, 300, 60, c);
        s.drawArc(x - 1, y, 11, 9, 300, 60, c);
      }
      else
      {
        s.drawWideLine(x + 4, y - 5, x + 12, y + 5, 1.2f, c);
        s.drawWideLine(x + 4, y + 5, x + 12, y - 5, 1.2f, c);
      }
      break;
    case TI_VFO:         // Two overlapping windows, A and B
      s.drawRoundRect(x - 12, y - 9, 15, 13, 2, c);
      s.fillRoundRect(x - 3, y - 4, 15, 13, 2, c);
      s.setTextColor(bg);
      s.setTextDatum(middle_center);
      s.drawString("B", x + 5, y + 3, &lgfx::fonts::Font0);
      s.setTextColor(c);
      s.drawString("A", x - 6, y - 3, &lgfx::fonts::Font0);
      break;
    case TI_STEP:        // Stairs
      s.drawWideLine(x - 12, y + 10, x - 12, y + 4, 1.2f, c);
      for(int i = 0 ; i < 4 ; i++)
      {
        s.drawWideLine(x - 12 + i * 6, y + 4 - i * 5, x - 6 + i * 6, y + 4 - i * 5, 1.2f, c);
        if(i < 3) s.drawWideLine(x - 6 + i * 6, y + 4 - i * 5, x - 6 + i * 6, y - 1 - i * 5, 1.2f, c);
      }
      break;
    case TI_SEEK:        // Fast forward to a bar
      s.fillTriangle(x - 12, y - 8, x - 12, y + 8, x - 3, y, c);
      s.fillTriangle(x - 3, y - 8, x - 3, y + 8, x + 6, y, c);
      s.fillRect(x + 7, y - 8, 3, 17, c);
      break;
    case TI_SCAN:        // Spectrum with a sweep
      for(int i = 0 ; i < 7 ; i++)
      {
        static const uint8_t h[7] = { 4, 8, 14, 6, 18, 9, 5 };
        s.fillRect(x - 12 + i * 3 + i / 2 * 1, y + 10 - h[i], 2, h[i], c);
      }
      s.drawWideLine(x + 9, y - 11, x + 9, y + 10, 1.0f, c);
      break;
    case TI_MEMORY:      // Floppy disk
    case TI_MEMSCAN:
      s.fillRoundRect(x - 11, y - 11, 22, 22, 3, c);
      s.fillRect(x - 6, y - 11, 11, 7, bg);
      s.fillRect(x + 1, y - 10, 3, 5, c);
      s.fillRoundRect(x - 7, y + 1, 14, 9, 1, bg);
      if(icon == TI_MEMSCAN)
      {
        s.fillTriangle(x - 3, y + 2, x - 3, y + 9, x + 3, y + 5, c);
      }
      else
      {
        s.drawFastHLine(x - 5, y + 4, 10, c);
        s.drawFastHLine(x - 5, y + 7, 10, c);
      }
      break;
    case TI_SQUELCH:     // Noise cut by a threshold line
      for(int i = -12 ; i < 10 ; i += 4)
        s.drawWideLine(x + i, y + ((i / 4) & 1 ? -6 : 6), x + i + 4, y + (((i / 4) & 1) ? 6 : -6), 1.0f, c);
      s.drawWideLine(x - 13, y, x + 13, y, 1.2f, c);
      break;
    case TI_BANDWIDTH:   // Filter shape
      s.drawWideLine(x - 13, y + 8, x - 6, y - 7, 1.5f, c);
      s.drawWideLine(x - 6, y - 7, x + 6, y - 7, 1.5f, c);
      s.drawWideLine(x + 6, y - 7, x + 13, y + 8, 1.5f, c);
      s.drawWideLine(x - 13, y + 8, x + 13, y + 8, 1.5f, c);
      break;
    case TI_AGC:         // Amplifier
    case TI_ATT:
      s.drawWideLine(x - 8, y - 10, x - 8, y + 10, 1.5f, c);
      s.drawWideLine(x - 8, y - 10, x + 10, y, 1.5f, c);
      s.drawWideLine(x - 8, y + 10, x + 10, y, 1.5f, c);
      s.drawWideLine(x - 13, y, x - 8, y, 1.2f, c);
      if(icon == TI_ATT) s.drawWideLine(x - 5, y + 4, x + 3, y - 4, 1.2f, c);
      else s.drawWideLine(x - 4, y, x + 1, y, 1.2f, c);
      break;
    case TI_AVC:         // Level that rises and then holds
      s.drawWideLine(x - 13, y + 8, x - 2, y - 6, 1.5f, c);
      s.drawWideLine(x - 2, y - 6, x + 13, y - 6, 1.5f, c);
      s.drawWideLine(x - 13, y + 10, x + 13, y + 10, 1.0f, c);
      break;
    case TI_BEACON:      // Mast with waves
      s.drawWideLine(x, y - 4, x - 6, y + 12, 1.2f, c);
      s.drawWideLine(x, y - 4, x + 6, y + 12, 1.2f, c);
      s.drawWideLine(x - 3, y + 5, x + 3, y + 5, 1.0f, c);
      s.fillCircle(x, y - 6, 2, c);
      s.drawArc(x, y - 6, 7, 5, 210, 330, c);
      s.drawArc(x, y - 6, 12, 10, 210, 330, c);
      break;
    case TI_SETTINGS:    // Gear
      for(int i = 0 ; i < 8 ; i++)
      {
        float a = i * 0.785f;
        s.drawWideLine(x + 7 * cosf(a), y + 7 * sinf(a), x + 12 * cosf(a), y + 12 * sinf(a), 2.2f, c);
      }
      s.fillCircle(x, y, 8, c);
      s.fillCircle(x, y, 3, bg);
      break;
    case TI_BRIGHT:      // Sun
      s.fillCircle(x, y, 5, c);
      for(int i = 0 ; i < 8 ; i++)
      {
        float a = i * 0.785f;
        s.drawWideLine(x + 8 * cosf(a), y + 8 * sinf(a), x + 12 * cosf(a), y + 12 * sinf(a), 1.0f, c);
      }
      break;
    case TI_CLOCK:
      s.fillArc(x, y, 12, 10, 0, 360, c);
      s.drawWideLine(x, y, x, y - 7, 1.2f, c);
      s.drawWideLine(x, y, x + 5, y + 3, 1.2f, c);
      break;
    case TI_GLOBE:
    case TI_REGION:
      s.fillArc(x, y, 12, 10, 0, 360, c);
      s.drawEllipse(x, y, 5, 11, c);
      s.drawFastHLine(x - 11, y, 23, c);
      s.drawFastVLine(x, y - 11, 23, c);
      if(icon == TI_GLOBE) { s.drawFastHLine(x - 9, y - 6, 19, c); s.drawFastHLine(x - 9, y + 6, 19, c); }
      break;
    case TI_MOON:
      s.fillCircle(x, y, 11, c);
      s.fillCircle(x + 6, y - 5, 10, bg);
      break;
    case TI_POWER:
      s.drawArc(x, y + 1, 11, 9, 300, 240, c);
      s.drawWideLine(x, y - 12, x, y + 1, 1.5f, c);
      break;
    case TI_PALETTE:     // Three colour dots
      s.fillCircle(x - 6, y + 4, 6, c);
      s.fillCircle(x + 6, y + 4, 6, tileBlend(c, bg, 90));
      s.fillCircle(x, y - 6, 6, tileBlend(c, bg, 170));
      break;
    case TI_LAYOUT:
      s.drawRoundRect(x - 12, y - 10, 25, 20, 2, c);
      s.fillRect(x - 12, y - 10, 25, 5, c);
      s.fillRect(x - 9, y - 2, 7, 9, c);
      s.drawFastHLine(x + 1, y - 1, 9, c);
      s.drawFastHLine(x + 1, y + 3, 9, c);
      break;
    case TI_ZOOM:
      s.fillArc(x - 3, y - 3, 9, 7, 0, 360, c);
      s.drawWideLine(x + 4, y + 4, x + 11, y + 11, 2.0f, c);
      s.drawWideLine(x - 7, y - 3, x + 1, y - 3, 1.0f, c);
      s.drawWideLine(x - 3, y - 7, x - 3, y + 1, 1.0f, c);
      break;
    case TI_SCROLL:      // Up and down arrows
      s.fillTriangle(x - 11, y - 4, x - 5, y - 12, x + 1, y - 4, c);
      s.fillRect(x - 6, y - 4, 3, 14, c);
      s.fillTriangle(x - 1, y + 4, x + 5, y + 12, x + 11, y + 4, c);
      s.fillRect(x + 4, y - 10, 3, 14, c);
      break;
    case TI_RDS:
      s.drawRoundRect(x - 13, y - 8, 27, 17, 3, c);
      s.setTextColor(c);
      s.setTextDatum(middle_center);
      s.drawString("RDS", x + 1, y + 1, &lgfx::fonts::Font0);
      break;
    case TI_STEREO:
      s.fillArc(x - 5, y, 9, 7, 0, 360, c);
      s.fillArc(x + 5, y, 9, 7, 0, 360, c);
      break;
    case TI_DIAL:        // Tuning dial with a pointer
      s.fillArc(x, y + 4, 12, 10, 180, 360, c);
      s.drawWideLine(x, y + 4, x + 6, y - 5, 1.5f, c);
      s.fillCircle(x, y + 4, 2, c);
      break;
    case TI_CHIP:
      s.fillRoundRect(x - 7, y - 7, 15, 15, 2, c);
      for(int i = -4 ; i <= 4 ; i += 4)
      {
        s.drawFastHLine(x - 11, y + i, 4, c); s.drawFastHLine(x + 8, y + i, 4, c);
        s.drawFastVLine(x + i, y - 11, 4, c); s.drawFastVLine(x + i, y + 8, 4, c);
      }
      break;
    case TI_DOWNLOAD:
    case TI_FIRMWARE:
      s.drawWideLine(x - 11, y + 5, x - 11, y + 11, 1.2f, c);
      s.drawWideLine(x - 11, y + 11, x + 11, y + 11, 1.2f, c);
      s.drawWideLine(x + 11, y + 11, x + 11, y + 5, 1.2f, c);
      s.fillRect(x - 1, y - 11, 3, 15, c);
      s.fillTriangle(x - 7, y + 1, x + 8, y + 1, x, y + 8, c);   // Down arrow into the tray
      break;
    case TI_USB:         // USB trident
      s.drawWideLine(x, y + 10, x, y - 8, 1.2f, c);
      s.fillTriangle(x - 4, y - 7, x + 4, y - 7, x, y - 12, c);
      s.drawWideLine(x, y + 4, x - 7, y - 1, 1.2f, c);
      s.drawWideLine(x - 7, y - 1, x - 7, y - 4, 1.2f, c);
      s.fillCircle(x - 7, y - 5, 2, c);
      s.drawWideLine(x, y + 1, x + 7, y - 3, 1.2f, c);
      s.fillRect(x + 5, y - 7, 5, 4, c);
      s.fillCircle(x, y + 10, 3, c);
      break;
    case TI_NETWORK:     // Three linked nodes
      s.drawWideLine(x, y - 7, x - 8, y + 7, 1.2f, c);
      s.drawWideLine(x, y - 7, x + 8, y + 7, 1.2f, c);
      s.drawWideLine(x - 8, y + 7, x + 8, y + 7, 1.2f, c);
      s.fillCircle(x, y - 7, 4, c); s.fillCircle(x - 8, y + 7, 4, c); s.fillCircle(x + 8, y + 7, 4, c);
      break;
    case TI_BT:          // Bluetooth rune
      s.drawWideLine(x - 1, y - 12, x - 1, y + 12, 1.2f, c);
      s.drawWideLine(x - 1, y - 12, x + 7, y - 5, 1.2f, c);
      s.drawWideLine(x + 7, y - 5, x - 8, y + 6, 1.2f, c);
      s.drawWideLine(x - 1, y + 12, x + 7, y + 5, 1.2f, c);
      s.drawWideLine(x + 7, y + 5, x - 8, y - 6, 1.2f, c);
      break;
    case TI_WIFI:
      s.fillCircle(x, y + 9, 2, c);
      s.drawArc(x, y + 9, 8, 6, 225, 315, c);
      s.drawArc(x, y + 9, 14, 12, 225, 315, c);
      s.drawArc(x, y + 9, 20, 18, 228, 312, c);
      break;
    case TI_BANDPLAN:    // The coloured band plan strip
      s.fillRect(x - 13, y + 2, 7, 5, TILE_RGB(0, 255, 255));
      s.fillRect(x - 6, y + 2, 5, 5, TILE_RGB(255, 255, 0));
      s.fillRect(x - 1, y + 2, 2, 5, TILE_RGB(255, 60, 60));
      s.fillRect(x + 1, y + 2, 12, 5, TILE_RGB(0, 255, 0));
      for(int i = -12 ; i <= 12 ; i += 4) s.drawFastVLine(x + i, y - 8, i % 8 ? 6 : 9, c);
      break;
    case TI_TILES:
      for(int r = 0 ; r < 2 ; r++)
        for(int k = 0 ; k < 3 ; k++)
          s.fillRoundRect(x - 13 + k * 9, y - 9 + r * 10, 8, 8, 2, c);
      break;
    case TI_LOG:         // Document with lines
      s.drawRoundRect(x - 9, y - 12, 19, 24, 2, c);
      for(int i = 0 ; i < 4 ; i++) s.fillRect(x - 5, y - 7 + i * 5, i == 3 ? 7 : 11, 2, c);
      break;
    case TI_PROP:        // Sun over a wave
      s.fillCircle(x - 4, y - 4, 6, c);
      for(int i = 0 ; i < 6 ; i++)
      {
        float a = 3.14f + i * 0.63f;
        s.drawWideLine(x - 4 + 9 * cosf(a), y - 4 + 9 * sinf(a), x - 4 + 12 * cosf(a), y - 4 + 12 * sinf(a), 1.0f, c);
      }
      for(int i = -12 ; i < 12 ; i++)
        s.drawWideLine(x + i, y + 9 - (int)(3 * sinf(i * 0.5f)), x + i + 1, y + 9 - (int)(3 * sinf((i + 1) * 0.5f)), 1.0f, c);
      break;
    case TI_SLEEPTIMER:  // Moon with "z"
      s.fillCircle(x - 3, y + 2, 10, c);
      s.fillCircle(x + 3, y - 3, 9, bg);
      s.drawWideLine(x + 3, y - 11, x + 11, y - 11, 1.0f, c);
      s.drawWideLine(x + 11, y - 11, x + 3, y - 3, 1.0f, c);
      s.drawWideLine(x + 3, y - 3, x + 11, y - 3, 1.0f, c);
      break;
    case TI_FORK:        // Tuning fork
      s.drawWideLine(x - 5, y - 12, x - 5, y + 1, 1.5f, c);
      s.drawWideLine(x + 5, y - 12, x + 5, y + 1, 1.5f, c);
      s.drawArc(x, y + 1, 6, 4, 0, 180, c);
      s.drawWideLine(x, y + 6, x, y + 13, 1.8f, c);
      break;
    case TI_BACK:        // Arrow back
      s.drawWideLine(x - 11, y, x + 11, y, 1.8f, c);
      s.drawWideLine(x - 11, y, x - 3, y - 8, 1.8f, c);
      s.drawWideLine(x - 11, y, x - 3, y + 8, 1.8f, c);
      break;
    case TI_CLOSE:       // Cross
      s.drawWideLine(x - 9, y - 9, x + 9, y + 9, 1.8f, c);
      s.drawWideLine(x - 9, y + 9, x + 9, y - 9, 1.8f, c);
      break;
    case TI_INFO:
      s.fillArc(x, y, 12, 10, 0, 360, c);
      s.fillCircle(x, y - 5, 2, c);
      s.fillRect(x - 1, y - 1, 3, 9, c);
      break;
    default:
      break;
  }
}

// Full-colour icons (RESONIK): white line work with bright accents, in the
// manner of the band plan strip. Accent colours are the iOS dark-mode set.
// Accent colours of the full-colour icons. A menu can set its own set to
// match its palette; tileDrawPage swaps any accent that would vanish on the
// tile it sits on.

static const TileAccents tileAccentsDefault =
{
  0xFFFF, TILE_RGB(255, 214, 10), TILE_RGB(255, 159, 10), TILE_RGB(255, 69, 58),
  TILE_RGB(48, 209, 88), TILE_RGB(100, 210, 255), TILE_RGB(10, 132, 255),
  TILE_RGB(255, 100, 170), TILE_RGB(40, 40, 44),
};
static TileAccents tileAcc = tileAccentsDefault;

#define TC_W  tileAcc.w
#define TC_Y  tileAcc.y
#define TC_O  tileAcc.o
#define TC_R  tileAcc.r
#define TC_G  tileAcc.g
#define TC_C  tileAcc.c
#define TC_B  tileAcc.b
#define TC_P  tileAcc.p
#define TC_K  tileAcc.k

// Distance between two colours, weighted for the eye
static inline int tileDist(uint16_t a, uint16_t b)
{
  int dr = (int)((a >> 11) & 31) - (int)((b >> 11) & 31);
  int dg = (int)((a >> 5) & 63) - (int)((b >> 5) & 63);
  int db = (int)(a & 31) - (int)(b & 31);
  return(dr * dr * 4 + dg * dg + db * db * 4);
}

// Set the accents for one tile: an accent in the tile's own colour would
// vanish, so it takes the palette colour that stands out most on that tile
static void tileAccentsFor(const TileAccents &base, uint16_t tile)
{
  tileAcc = base;
  const uint16_t cand[5] = { base.y, base.o, base.r, base.g, base.b };
  uint16_t swap = cand[0];
  for(int j = 1 ; j < 5 ; j++)
    if(tileDist(cand[j], tile) > tileDist(swap, tile)) swap = cand[j];
  uint16_t *f = &tileAcc.w;
  for(int j = 0 ; j < 9 ; j++)
    if(tileDist(f[j], tile) < 60) f[j] = swap;
}

template<typename S>
static void tileDrawIconColor(S &s, TileIcon icon, int x, int y, uint16_t bg)
{
  switch(icon)
  {
    case TI_MODE:        // Two waves: AM and FM
      for(int i = -12 ; i < 12 ; i++)
      {
        int y0 = y - (int)(7 * sinf(i * 0.27f)), y1 = y - (int)(7 * sinf((i + 1) * 0.27f));
        s.drawWideLine(x + i, y0 - 2, x + i + 1, y1 - 2, 1.4f, TC_C);
        int z0 = y + 6 - (int)(3 * sinf(i * 0.8f)), z1 = y + 6 - (int)(3 * sinf((i + 1) * 0.8f));
        s.drawWideLine(x + i, z0, x + i + 1, z1, 1.0f, TC_Y);
      }
      break;
    case TI_BAND:        // Signal bars, green to red
    {
      const uint16_t bc[5] = { TC_G, TC_G, TC_Y, TC_O, TC_R };
      for(int i = 0 ; i < 5 ; i++) s.fillRoundRect(x - 12 + i * 5, y + 10 - i * 5, 4, 2 + i * 5, 1, bc[i]);
      break;
    }
    case TI_VOLUME:
    case TI_SOFTMUTE:
      s.fillRect(x - 11, y - 4, 5, 8, TC_W);
      s.fillTriangle(x - 7, y, x - 1, y - 9, x - 1, y + 9, TC_W);
      if(icon == TI_VOLUME)
      {
        s.drawArc(x - 1, y, 6, 4, 300, 60, TC_G);
        s.drawArc(x - 1, y, 11, 9, 300, 60, TC_C);
      }
      else
      {
        s.drawWideLine(x + 4, y - 5, x + 12, y + 5, 1.5f, TC_R);
        s.drawWideLine(x + 4, y + 5, x + 12, y - 5, 1.5f, TC_R);
      }
      break;
    case TI_VFO:
      s.fillRoundRect(x - 12, y - 9, 15, 13, 2, TC_C);
      s.fillRoundRect(x - 3, y - 4, 15, 13, 2, TC_Y);
      s.drawRoundRect(x - 3, y - 4, 15, 13, 2, bg);
      s.setTextDatum(middle_center);
      s.setTextColor(TC_K);
      s.drawString("A", x - 6, y - 3, &lgfx::fonts::Font0);
      s.drawString("B", x + 5, y + 3, &lgfx::fonts::Font0);
      break;
    case TI_STEP:        // Coloured steps
    {
      const uint16_t sc[4] = { TC_G, TC_C, TC_Y, TC_O };
      for(int i = 0 ; i < 4 ; i++) s.fillRect(x - 12 + i * 6, y + 4 - i * 5, 6, 8 + i * 5, sc[i]);
      break;
    }
    case TI_SEEK:
      s.fillTriangle(x - 12, y - 8, x - 12, y + 8, x - 3, y, TC_Y);
      s.fillTriangle(x - 3, y - 8, x - 3, y + 8, x + 6, y, TC_O);
      s.fillRect(x + 7, y - 8, 3, 17, TC_W);
      break;
    case TI_SCAN:        // Spectrum and a red sweep line
    {
      static const uint8_t h[7] = { 4, 8, 14, 6, 18, 9, 5 };
      const uint16_t hc[7] = { TC_G, TC_G, TC_Y, TC_G, TC_O, TC_Y, TC_G };
      for(int i = 0 ; i < 7 ; i++) s.fillRect(x - 12 + i * 3 + i / 2, y + 10 - h[i], 2, h[i], hc[i]);
      s.drawWideLine(x + 9, y - 11, x + 9, y + 10, 1.0f, TC_R);
      break;
    }
    case TI_MEMORY:
    case TI_MEMSCAN:     // Floppy disk
      s.fillRoundRect(x - 11, y - 11, 22, 22, 3, TC_B);
      s.fillRect(x - 6, y - 11, 11, 7, TC_W);
      s.fillRect(x + 1, y - 10, 3, 5, TC_B);
      s.fillRoundRect(x - 7, y + 1, 14, 9, 1, TC_W);
      if(icon == TI_MEMSCAN) s.fillTriangle(x - 3, y + 2, x - 3, y + 9, x + 3, y + 5, TC_G);
      else { s.drawFastHLine(x - 5, y + 4, 10, TC_C); s.drawFastHLine(x - 5, y + 7, 10, TC_C); }
      s.drawRoundRect(x - 11, y - 11, 22, 22, 3, TC_W);
      break;
    case TI_SQUELCH:     // Noise cut by a red threshold
      for(int i = -12 ; i < 10 ; i += 4)
        s.drawWideLine(x + i, y + ((i / 4) & 1 ? -6 : 6), x + i + 4, y + (((i / 4) & 1) ? 6 : -6), 1.0f, TC_Y);
      s.drawWideLine(x - 13, y, x + 13, y, 1.4f, TC_R);
      break;
    case TI_BANDWIDTH:   // Filled filter shape
      s.fillTriangle(x - 13, y + 8, x - 6, y - 7, x - 6, y + 8, TC_G);
      s.fillRect(x - 6, y - 7, 13, 16, TC_G);
      s.fillTriangle(x + 6, y - 7, x + 13, y + 8, x + 6, y + 8, TC_G);
      s.drawWideLine(x - 13, y + 8, x - 6, y - 7, 1.2f, TC_W);
      s.drawWideLine(x - 6, y - 7, x + 6, y - 7, 1.2f, TC_W);
      s.drawWideLine(x + 6, y - 7, x + 13, y + 8, 1.2f, TC_W);
      s.drawWideLine(x - 14, y + 9, x + 14, y + 9, 1.2f, TC_W);
      break;
    case TI_AGC:
    case TI_ATT:         // Amplifier
      s.fillTriangle(x - 8, y - 10, x - 8, y + 10, x + 10, y, icon == TI_AGC ? TC_O : TC_C);
      s.drawWideLine(x - 8, y - 10, x + 10, y, 1.2f, TC_W);
      s.drawWideLine(x - 8, y + 10, x + 10, y, 1.2f, TC_W);
      s.drawWideLine(x - 8, y - 10, x - 8, y + 10, 1.2f, TC_W);
      s.drawWideLine(x - 13, y, x - 8, y, 1.2f, TC_W);
      s.drawWideLine(x + 10, y, x + 13, y, 1.2f, TC_W);
      if(icon == TI_ATT) s.drawWideLine(x - 5, y + 4, x + 2, y - 3, 1.2f, TC_K);
      break;
    case TI_AVC:
      s.drawWideLine(x - 13, y + 8, x - 2, y - 6, 1.6f, TC_G);
      s.drawWideLine(x - 2, y - 6, x + 13, y - 6, 1.6f, TC_C);
      s.drawWideLine(x - 13, y + 10, x + 13, y + 10, 1.0f, TC_W);
      break;
    case TI_BEACON:      // Mast with orange waves
      s.drawWideLine(x, y - 4, x - 6, y + 12, 1.2f, TC_W);
      s.drawWideLine(x, y - 4, x + 6, y + 12, 1.2f, TC_W);
      s.drawWideLine(x - 3, y + 5, x + 3, y + 5, 1.0f, TC_W);
      s.fillCircle(x, y - 6, 2, TC_R);
      s.drawArc(x, y - 6, 7, 5, 210, 330, TC_Y);
      s.drawArc(x, y - 6, 12, 10, 210, 330, TC_O);
      break;
    case TI_SETTINGS:    // Gear
      for(int i = 0 ; i < 8 ; i++)
      {
        float a = i * 0.785f;
        s.drawWideLine(x + 7 * cosf(a), y + 7 * sinf(a), x + 12 * cosf(a), y + 12 * sinf(a), 2.2f, TILE_RGB(210, 210, 216));
      }
      s.fillCircle(x, y, 8, TILE_RGB(210, 210, 216));
      s.fillCircle(x, y, 4, TC_O);
      break;
    case TI_BRIGHT:      // Sun
      for(int i = 0 ; i < 8 ; i++)
      {
        float a = i * 0.785f;
        s.drawWideLine(x + 8 * cosf(a), y + 8 * sinf(a), x + 12 * cosf(a), y + 12 * sinf(a), 1.2f, TC_O);
      }
      s.fillCircle(x, y, 6, TC_Y);
      break;
    case TI_CLOCK:
      s.fillCircle(x, y, 12, TC_W);
      s.drawWideLine(x, y, x, y - 8, 1.4f, TC_K);
      s.drawWideLine(x, y, x + 6, y + 3, 1.4f, TC_K);
      s.drawWideLine(x, y, x - 5, y + 7, 0.8f, TC_R);
      s.fillCircle(x, y, 2, TC_R);
      break;
    case TI_GLOBE:
    case TI_REGION:      // Blue globe, green land
      s.fillCircle(x, y, 12, icon == TI_GLOBE ? TC_B : TC_C);
      s.fillEllipse(x - 4, y - 3, 5, 6, TC_G);
      s.fillEllipse(x + 6, y + 4, 4, 5, TC_G);
      s.drawEllipse(x, y, 5, 11, TC_W);
      s.drawFastHLine(x - 11, y, 23, TC_W);
      s.drawCircle(x, y, 12, TC_W);
      break;
    case TI_MOON:
    case TI_SLEEPTIMER:
      s.fillCircle(x - 2, y + 2, 11, TC_Y);
      s.fillCircle(x + 5, y - 4, 10, bg);
      if(icon == TI_SLEEPTIMER)
      {
        s.drawWideLine(x + 4, y - 11, x + 11, y - 11, 1.0f, TC_C);
        s.drawWideLine(x + 11, y - 11, x + 4, y - 3, 1.0f, TC_C);
        s.drawWideLine(x + 4, y - 3, x + 11, y - 3, 1.0f, TC_C);
      }
      else
      {
        s.fillCircle(x + 9, y - 9, 1, TC_W);
        s.fillCircle(x + 5, y + 5, 1, TC_W);
      }
      break;
    case TI_POWER:
      s.drawArc(x, y + 1, 11, 8, 300, 240, TC_G);
      s.drawWideLine(x, y - 12, x, y + 1, 1.8f, TC_W);
      break;
    case TI_PALETTE:     // Red, green, blue
      s.fillCircle(x - 6, y + 4, 6, TC_R);
      s.fillCircle(x + 6, y + 4, 6, TC_B);
      s.fillCircle(x, y - 6, 6, TC_G);
      break;
    case TI_LAYOUT:
      s.fillRoundRect(x - 12, y - 10, 25, 20, 2, TC_W);
      s.fillRect(x - 12, y - 10, 25, 5, TC_C);
      s.fillRect(x - 9, y - 2, 7, 9, TC_Y);
      s.drawFastHLine(x + 1, y - 1, 9, TC_K);
      s.drawFastHLine(x + 1, y + 3, 9, TC_K);
      s.drawFastHLine(x + 1, y + 6, 6, TC_K);
      break;
    case TI_ZOOM:
      s.fillCircle(x - 3, y - 3, 8, TC_C);
      s.drawWideLine(x + 4, y + 4, x + 11, y + 11, 2.2f, TC_O);
      s.drawCircle(x - 3, y - 3, 9, TC_W);
      s.drawCircle(x - 3, y - 3, 8, TC_W);
      s.drawWideLine(x - 7, y - 3, x + 1, y - 3, 1.0f, TC_W);
      s.drawWideLine(x - 3, y - 7, x - 3, y + 1, 1.0f, TC_W);
      break;
    case TI_SCROLL:
      s.fillTriangle(x - 11, y - 4, x - 5, y - 12, x + 1, y - 4, TC_G);
      s.fillRect(x - 6, y - 4, 3, 14, TC_G);
      s.fillTriangle(x - 1, y + 4, x + 5, y + 12, x + 11, y + 4, TC_O);
      s.fillRect(x + 4, y - 10, 3, 14, TC_O);
      break;
    case TI_RDS:
      s.fillRoundRect(x - 13, y - 8, 27, 17, 3, TC_Y);
      s.setTextColor(TC_K);
      s.setTextDatum(middle_center);
      s.drawString("RDS", x + 1, y + 1, &lgfx::fonts::Font0);
      break;
    case TI_STEREO:
      s.fillArc(x - 5, y, 9, 6, 0, 360, TC_C);
      s.fillArc(x + 5, y, 9, 6, 0, 360, TC_P);
      break;
    case TI_DIAL:        // Meter arc, green to red, white needle
      s.fillArc(x, y + 5, 12, 9, 180, 240, TC_G);
      s.fillArc(x, y + 5, 12, 9, 240, 300, TC_Y);
      s.fillArc(x, y + 5, 12, 9, 300, 360, TC_R);
      s.drawWideLine(x, y + 5, x + 6, y - 6, 1.4f, TC_W);
      s.fillCircle(x, y + 5, 2, TC_W);
      break;
    case TI_FORK:        // Tuning fork with sound
      s.drawWideLine(x - 4, y - 12, x - 4, y + 1, 1.5f, TC_W);
      s.drawWideLine(x + 4, y - 12, x + 4, y + 1, 1.5f, TC_W);
      s.drawArc(x, y + 1, 5, 3, 0, 180, TC_W);
      s.drawWideLine(x, y + 6, x, y + 13, 1.8f, TC_W);
      s.drawArc(x, y - 6, 11, 10, 300, 330, TC_C);
      s.drawArc(x, y - 6, 11, 10, 210, 240, TC_C);
      break;
    case TI_CHIP:        // Dark chip, gold pins
      for(int i = -4 ; i <= 4 ; i += 4)
      {
        s.fillRect(x - 12, y + i - 1, 5, 2, TC_Y); s.fillRect(x + 8, y + i - 1, 5, 2, TC_Y);
        s.fillRect(x + i - 1, y - 12, 2, 5, TC_Y); s.fillRect(x + i - 1, y + 8, 2, 5, TC_Y);
      }
      s.fillRoundRect(x - 8, y - 8, 17, 17, 2, TC_K);
      s.drawRoundRect(x - 8, y - 8, 17, 17, 2, TC_W);
      s.fillCircle(x - 4, y - 4, 1, TC_C);
      break;
    case TI_DOWNLOAD:
    case TI_FIRMWARE:
      s.drawWideLine(x - 11, y + 5, x - 11, y + 11, 1.2f, TC_W);
      s.drawWideLine(x - 11, y + 11, x + 11, y + 11, 1.2f, TC_W);
      s.drawWideLine(x + 11, y + 11, x + 11, y + 5, 1.2f, TC_W);
      {
        // Both arrows point down into the tray: green for EiBi, blue for firmware
        uint16_t ac = icon == TI_DOWNLOAD ? TC_G : TC_C;
        s.fillRect(x - 1, y - 11, 4, 13, ac);
        s.fillTriangle(x - 7, y + 1, x + 9, y + 1, x + 1, y + 8, ac);
      }
      break;
    case TI_USB:
      s.drawWideLine(x, y + 10, x, y - 8, 1.4f, TC_W);
      s.fillTriangle(x - 4, y - 7, x + 4, y - 7, x, y - 12, TC_W);
      s.drawWideLine(x, y + 4, x - 7, y - 1, 1.2f, TC_W);
      s.drawWideLine(x - 7, y - 1, x - 7, y - 4, 1.2f, TC_W);
      s.fillCircle(x - 7, y - 5, 3, TC_C);
      s.drawWideLine(x, y + 1, x + 7, y - 3, 1.2f, TC_W);
      s.fillRect(x + 4, y - 8, 6, 5, TC_Y);
      s.fillCircle(x, y + 10, 3, TC_W);
      break;
    case TI_NETWORK:
      s.drawWideLine(x, y - 7, x - 8, y + 7, 1.2f, TC_W);
      s.drawWideLine(x, y - 7, x + 8, y + 7, 1.2f, TC_W);
      s.drawWideLine(x - 8, y + 7, x + 8, y + 7, 1.2f, TC_W);
      s.fillCircle(x, y - 7, 4, TC_C); s.fillCircle(x - 8, y + 7, 4, TC_Y); s.fillCircle(x + 8, y + 7, 4, TC_G);
      break;
    case TI_BT:          // Rune on a Bluetooth-blue disc
      s.fillCircle(x, y, 13, TC_B);
      s.drawWideLine(x - 1, y - 9, x - 1, y + 9, 1.2f, TC_W);
      s.drawWideLine(x - 1, y - 9, x + 5, y - 4, 1.2f, TC_W);
      s.drawWideLine(x + 5, y - 4, x - 6, y + 5, 1.2f, TC_W);
      s.drawWideLine(x - 1, y + 9, x + 5, y + 4, 1.2f, TC_W);
      s.drawWideLine(x + 5, y + 4, x - 6, y - 5, 1.2f, TC_W);
      break;
    case TI_WIFI:        // Strength arcs
      s.fillCircle(x, y + 9, 2, TC_W);
      s.drawArc(x, y + 9, 8, 6, 225, 315, TC_G);
      s.drawArc(x, y + 9, 14, 12, 225, 315, TC_Y);
      s.drawArc(x, y + 9, 20, 18, 228, 312, TC_O);
      break;
    case TI_BANDPLAN:
      s.fillRect(x - 13, y + 2, 7, 5, TILE_RGB(0, 255, 255));
      s.fillRect(x - 6, y + 2, 5, 5, TILE_RGB(255, 255, 0));
      s.fillRect(x - 1, y + 2, 2, 5, TILE_RGB(255, 60, 60));
      s.fillRect(x + 1, y + 2, 12, 5, TILE_RGB(0, 255, 0));
      for(int i = -12 ; i <= 12 ; i += 4) s.drawFastVLine(x + i, y - 8, i % 8 ? 6 : 9, TC_W);
      break;
    case TI_TILES:
    {
      const uint16_t tc[6] = { TC_R, TC_Y, TC_G, TC_C, TC_B, TC_P };
      for(int r = 0 ; r < 2 ; r++)
        for(int k = 0 ; k < 3 ; k++)
          s.fillRoundRect(x - 13 + k * 9, y - 9 + r * 10, 8, 8, 2, tc[r * 3 + k]);
      break;
    }
    case TI_LOG:         // Sheet with a signal graph
      s.fillRoundRect(x - 9, y - 12, 19, 24, 2, TC_W);
      for(int i = 0 ; i < 2 ; i++) s.fillRect(x - 5, y - 8 + i * 4, 11, 2, TILE_RGB(150, 150, 156));
      s.drawWideLine(x - 6, y + 7, x - 2, y + 2, 1.0f, TC_G);
      s.drawWideLine(x - 2, y + 2, x + 1, y + 5, 1.0f, TC_G);
      s.drawWideLine(x + 1, y + 5, x + 6, y - 1, 1.0f, TC_R);
      break;
    case TI_PROP:        // Sun over the ionosphere
      for(int i = 0 ; i < 6 ; i++)
      {
        float a = 3.14f + i * 0.63f;
        s.drawWideLine(x - 4 + 9 * cosf(a), y - 4 + 9 * sinf(a), x - 4 + 12 * cosf(a), y - 4 + 12 * sinf(a), 1.0f, TC_O);
      }
      s.fillCircle(x - 4, y - 4, 6, TC_Y);
      for(int i = -12 ; i < 12 ; i++)
        s.drawWideLine(x + i, y + 9 - (int)(3 * sinf(i * 0.5f)), x + i + 1, y + 9 - (int)(3 * sinf((i + 1) * 0.5f)), 1.2f, TC_C);
      break;
    case TI_INFO:
      s.fillCircle(x, y, 12, TC_C);
      s.fillCircle(x, y - 5, 2, TC_W);
      s.fillRect(x - 1, y - 1, 3, 9, TC_W);
      break;
    default:             // Back, close and anything else: white
      tileDrawIcon(s, icon, x, y, TC_W, bg);
      break;
  }
}

// Draw one page of tiles: three columns, two rows, page dots below.
// sel is the highlighted item; the page is the one that contains it.
template<typename S>
static void tileDrawPage(S &s, const TileItem *items, int count, int sel, uint16_t bg, uint16_t muted,
                         const lgfx::IFont *labelFont = nullptr, bool upper = true,
                         const TileStyle *style = nullptr,
                         int pageNo = -1, int pageCount = 0,
                         const lgfx::IFont *smallFont = nullptr)
{
  const int cols = 3, rows = 2, perPage = cols * rows;
  const int mx = 8, top = 7, gap = 6;
  const int tw0 = (320 - 2 * mx - (cols - 1) * gap) / cols;   // 98
  const int th0 = 69;
  int page = sel / perPage;
  int pages = (count + perPage - 1) / perPage;
  // A caller that pages by itself passes the page it shows and the total
  int dotPage = pageNo >= 0 ? pageNo : page;
  int dotPages = pageCount > 0 ? pageCount : pages;

  s.fillSprite(bg);

  // The selected tile is drawn last so that it can pop out over its neighbours
  for(int o = 0 ; o < perPage ; o++)
  {
    int ks = sel - page * perPage;
    int k = o < ks ? o : (o == perPage - 1 ? ks : o + 1);
    int i = page * perPage + k;
    if(i >= count) continue;
    int x = mx + (k % cols) * (tw0 + gap);
    int y = top + (k / cols) * (th0 + gap);
    bool hl = i == sel;
    static const TileStyle defStyle = { true, 0, false, 0 };
    const TileStyle &st = style ? *style : defStyle;
    int tw = tw0, th = th0;
    if(hl && st.pop) { x -= st.pop; y -= st.pop; tw += 2 * st.pop; th += 2 * st.pop; }
    uint16_t fill = items[i].color;   // Same colour when selected: the white outline marks it
    uint16_t fill2 = items[i].color2;
    if(!hl && st.dimOthers)
    {
      fill = tileBlend(fill, 0x0000, st.dimOthers);
      if(fill2) fill2 = tileBlend(fill2, 0x0000, st.dimOthers);
    }
    bool gradient = fill2 != 0;

    if(hl && st.pop) s.fillRoundRect(x - 2, y - 2, tw + 4, th + 4, 14, bg);   // Gap ring in the screen colour
    if(gradient) tileFillGradient(s, x, y, tw, th, 12, fill2, fill);
    else s.fillRoundRect(x, y, tw, th, 12, fill);
    if(hl && st.outline)
    {
      s.drawRoundRect(x - 1, y - 1, tw + 2, th + 2, 13, 0xFFFF);
      s.drawRoundRect(x, y, tw, th, 12, 0xFFFF);
      s.drawRoundRect(x + 1, y + 1, tw - 2, th - 2, 11, 0xFFFF);
    }

    if(hl && st.pop) { x += st.pop; y += st.pop; tw = tw0; th = th0; }   // Icon and label stay put

    uint16_t iconBg = gradient ? tileBlend(fill, fill2, (uint8_t)(255 - 25 * 255 / (th - 1))) : fill;
    if(st.badge)
    {
      // White disc with the icon painted in the tile's colour
      uint16_t disc = hl || !st.dimOthers ? 0xFFFF : tileBlend(0xFFFF, 0x0000, st.dimOthers / 3);
      s.fillCircle(x + tw / 2, y + 24, 19, disc);
      tileDrawIcon(s, items[i].icon, x + tw / 2, y + 24, fill, disc);
    }
    else
    {
      // Icon colours that belong to the tile: a light tint of its own top
      // colour, optionally with a deep shadow, or engraved into the tile
      uint16_t topC = gradient ? fill2 : fill;
      int cx = x + tw / 2, cy = y + 25;
      switch(st.iconMode)
      {
        case 1:
          tileDrawIcon(s, items[i].icon, cx, cy, tileBlend(topC, 0xFFFF, 165), iconBg);
          break;
        case 2:
          tileDrawIcon(s, items[i].icon, cx + 1, cy + 2, tileBlend(fill, 0x0000, 110), iconBg);
          tileDrawIcon(s, items[i].icon, cx, cy, tileBlend(topC, 0xFFFF, 200), iconBg);
          break;
        case 3:
          tileDrawIcon(s, items[i].icon, cx, cy + 1, tileBlend(topC, 0xFFFF, 110), iconBg);
          tileDrawIcon(s, items[i].icon, cx, cy, tileBlend(fill, 0x0000, 120), iconBg);
          break;
        case 4:
          // A soft shadow in the tile's deep shade keeps every colour clear of the tile
          if(st.iconOutline)
          {
            // A thin outline lets an icon sit on a tile of its own colour
            static const int8_t d[8][2] = {{-1,0},{1,0},{0,-1},{0,1},{-1,-1},{1,-1},{-1,1},{1,1}};
            for(int j = 0 ; j < 8 ; j++)
              tileDrawIcon(s, items[i].icon, cx + d[j][0], cy + d[j][1], st.iconOutline, iconBg);
          }
          else
            tileDrawIcon(s, items[i].icon, cx + 1, cy + 1, tileBlend(fill, 0x0000, 140), iconBg);
          if(st.accents && items[i].paletteIcon) tileAccentsFor(*st.accents, fill);
          tileDrawIconColor(s, items[i].icon, cx, cy, iconBg);
          tileAcc = tileAccentsDefault;
          break;
        default:
          tileDrawIcon(s, items[i].icon, cx, cy, 0xFFFF, iconBg);
          break;
      }
    }

    // Label: a smooth font when given (readable at small size), else the
    // built-in bold font condensed to fit
    char label[20];
    int n = 0;
    for(const char *p = items[i].label ; *p && n < (int)sizeof(label) - 1 ; p++)
      label[n++] = upper ? toupper((unsigned char)*p) : *p;
    label[n] = '\0';
    if(gradient || st.labelShadow) s.setTextColor(0xFFFF); else s.setTextColor(0xFFFF, fill);
    s.setTextDatum(middle_center);
    if(labelFont)
    {
      s.setTextSize(1);
      // Long labels drop to the smaller size to fit the tile
      const lgfx::IFont *lf = smallFont && s.textWidth(label, labelFont) > tw - 6 ? smallFont : labelFont;
      if(items[i].text)
        s.setTextColor(items[i].text);
      else if(st.labelShadow)
      {
        // A dark shadow keeps white text readable on lighter tiles
        s.setTextColor(tileBlend(fill, 0x0000, st.labelShadow));
        s.drawString(label, x + tw / 2 + 1, y + th - 13, lf);
        s.setTextColor(0xFFFF);
      }
      s.drawString(label, x + tw / 2, y + th - 14, lf);
    }
    else
    {
      const lgfx::IFont *font = &lgfx::fonts::FreeSansBold9pt7b;
      s.setTextSize(1);
      float w = s.textWidth(label, font);
      float sx = w > 0 && (tw - 8) / w < 0.75f ? (tw - 8) / w : 0.75f;
      s.setTextSize(sx, 0.85f);
      s.setTextColor(0xFFFF);
      s.drawString(label, x + tw / 2, y + th - 14, font);
      s.setTextSize(1);
    }
  }

  // Page dots: the current page is a wider bar
  int dotsW = dotPages * 10;
  int dx = 160 - dotsW / 2;
  for(int p = 0 ; p < dotPages ; p++)
  {
    if(p == dotPage) s.fillRoundRect(dx + p * 10 - 2, 162, 10, 4, 2, 0xFFFF);
    else s.fillCircle(dx + p * 10 + 3, 163, 2, muted);
  }
}

#endif // TILE_DRAW_H
