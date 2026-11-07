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
  TI_RDS, TI_STEREO, TI_DIAL, TI_CHIP, TI_DOWNLOAD, TI_UPLOAD, TI_USB, TI_NETWORK, TI_BT,
  TI_WIFI, TI_BANDPLAN, TI_ATT, TI_TILES, TI_LOG, TI_PROP, TI_INFO, TI_REGION,
};

typedef struct
{
  const char *label;
  TileIcon icon;
  uint16_t color;
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
    case TI_UPLOAD:
      s.drawWideLine(x - 11, y + 5, x - 11, y + 11, 1.2f, c);
      s.drawWideLine(x - 11, y + 11, x + 11, y + 11, 1.2f, c);
      s.drawWideLine(x + 11, y + 11, x + 11, y + 5, 1.2f, c);
      s.fillRect(x - 1, y - 11, 3, 15, c);
      if(icon == TI_DOWNLOAD) s.fillTriangle(x - 7, y + 1, x + 8, y + 1, x, y + 8, c);
      else s.fillTriangle(x - 7, y - 6, x + 8, y - 6, x, y - 13, c);
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
    case TI_INFO:
      s.fillArc(x, y, 12, 10, 0, 360, c);
      s.fillCircle(x, y - 5, 2, c);
      s.fillRect(x - 1, y - 1, 3, 9, c);
      break;
    default:
      break;
  }
}

// Draw one page of tiles: three columns, two rows, page dots below.
// sel is the highlighted item; the page is the one that contains it.
template<typename S>
static void tileDrawPage(S &s, const TileItem *items, int count, int sel, uint16_t bg, uint16_t muted)
{
  const int cols = 3, rows = 2, perPage = cols * rows;
  const int mx = 8, top = 7, gap = 6;
  const int tw = (320 - 2 * mx - (cols - 1) * gap) / cols;    // 98
  const int th = 69;
  int page = sel / perPage;
  int pages = (count + perPage - 1) / perPage;

  s.fillSprite(bg);

  for(int k = 0 ; k < perPage ; k++)
  {
    int i = page * perPage + k;
    if(i >= count) break;
    int x = mx + (k % cols) * (tw + gap);
    int y = top + (k / cols) * (th + gap);
    bool hl = i == sel;
    uint16_t fill = hl ? tileBlend(items[i].color, 0xFFFF, 60) : items[i].color;

    s.fillRoundRect(x, y, tw, th, 8, fill);
    if(hl)
    {
      s.drawRoundRect(x - 1, y - 1, tw + 2, th + 2, 9, 0xFFFF);
      s.drawRoundRect(x, y, tw, th, 8, 0xFFFF);
      s.drawRoundRect(x + 1, y + 1, tw - 2, th - 2, 7, 0xFFFF);
    }

    tileDrawIcon(s, items[i].icon, x + tw / 2, y + 25, 0xFFFF, fill);

    // Label in capitals; the narrow font when it would not fit
    char label[20];
    int n = 0;
    for(const char *p = items[i].label ; *p && n < (int)sizeof(label) - 1 ; p++)
      label[n++] = toupper((unsigned char)*p);
    label[n] = '\0';
    // Bold, condensed to fit: like the labels of the reference design
    const lgfx::IFont *font = &lgfx::fonts::FreeSansBold9pt7b;
    s.setTextSize(1);
    float w = s.textWidth(label, font);
    float sx = w > 0 && (tw - 8) / w < 0.75f ? (tw - 8) / w : 0.75f;
    s.setTextSize(sx, 0.85f);
    s.setTextColor(0xFFFF);
    s.setTextDatum(middle_center);
    s.drawString(label, x + tw / 2, y + th - 14, font);
    s.setTextSize(1);
  }

  // Page dots: the current page is a wider bar
  int dotsW = pages * 10;
  int dx = 160 - dotsW / 2;
  for(int p = 0 ; p < pages ; p++)
  {
    if(p == page) s.fillRoundRect(dx + p * 10 - 2, 162, 10, 4, 2, 0xFFFF);
    else s.fillCircle(dx + p * 10 + 3, 163, 2, muted);
  }
}

#endif // TILE_DRAW_H
