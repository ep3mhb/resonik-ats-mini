#include "Common.h"
#include "Utils.h"
#include "Brand.h"
#include "BrandBitmaps.h"

// Symbol construction: bar width : gap = 5 : 3, bar heights 1 : 1.5 : 2.5 : 6 : 2.5 : 1.5 : 1,
// corner radius 0.4 x bar width. The peak (centre) bar is 48 units tall, a bar is 5 units wide.
int brandDrawSymbol(int x, int yBottom, int h, uint16_t sideColor, uint16_t peakColor)
{
  static const float heights[7] = { 1.0f, 1.5f, 2.5f, 6.0f, 2.5f, 1.5f, 1.0f };
  const float unit  = h / 48.0f;
  const float pitch = 8.0f * unit;
  int barW = int(5.0f * unit + 0.5f);
  if(barW < 2) barW = 2;
  const int radius = int(barW * 0.4f + 0.5f);

  for(int i = 0 ; i < 7 ; i++)
  {
    int bx = x + int(i * pitch + 0.5f);
    int bh = int(heights[i] * h / 6.0f + 0.5f);
    spr.fillRoundRect(bx, yBottom - bh, barW, bh, radius, i == 3 ? peakColor : sideColor);
  }
  return(int(6 * pitch + 0.5f) + barW);
}

static inline uint16_t blend565(uint16_t fg, uint16_t bg, uint8_t a)
{
  if(a == 0) return(bg);
  if(a == 255) return(fg);
  uint32_t rf = (fg >> 11) & 0x1F, gf = (fg >> 5) & 0x3F, bf = fg & 0x1F;
  uint32_t rb = (bg >> 11) & 0x1F, gb = (bg >> 5) & 0x3F, bb = bg & 0x1F;
  uint32_t r = (rf * a + rb * (255 - a)) / 255;
  uint32_t g = (gf * a + gb * (255 - a)) / 255;
  uint32_t b = (bf * a + bb * (255 - a)) / 255;
  return((r << 11) | (g << 5) | b);
}

int brandWordmarkWidth(bool large)  { return(large ? WORDMARK_L_W : WORDMARK_S_W); }
int brandWordmarkHeight(bool large) { return(large ? WORDMARK_L_H : WORDMARK_S_H); }

int brandDrawWordmark(int x, int y, bool large, uint16_t color, uint16_t bg)
{
  const uint8_t *mask = large ? WORDMARK_L : WORDMARK_S;
  const int w = brandWordmarkWidth(large);
  const int h = brandWordmarkHeight(large);

  for(int j = 0 ; j < h ; j++)
    for(int i = 0 ; i < w ; i++)
    {
      uint8_t a = pgm_read_byte(&mask[j * w + i]);
      if(a) spr.drawPixel(x + i, y + j, blend565(color, bg, a));
    }
  return(w);
}

//
// Boot screen: reversed lockup (Paper White on black, Peak Blue peak),
// then the product line, firmware version and web address.
//
void brandDrawBootScreen()
{
  const int W = spr.width();
  const int H = spr.height();
  const uint16_t bg = TFT_BLACK;

  spr.fillSprite(bg);

  // Lockup: peak bar = 1.7 x cap height, gap = 0.5 x cap height
  const int cap    = 30;
  const int peakH  = int(cap * 1.7f + 0.5f);
  const int gap    = cap / 2;
  const int symW   = int(6 * 8.0f * peakH / 48.0f + 0.5f) + int(5.0f * peakH / 48.0f + 0.5f);
  const int wordW  = brandWordmarkWidth(true);
  const int wordH  = brandWordmarkHeight(true);
  const int totalW = symW + gap + wordW;
  const int x0     = (W - totalW) / 2;
  const int base   = 82;                         // shared baseline of bars and letters

  brandDrawSymbol(x0, base, peakH, BRAND_PAPER_WHITE, BRAND_PEAK_BLUE);
  brandDrawWordmark(x0 + symW + gap, base - wordH, true, BRAND_PAPER_WHITE, bg);

  spr.setTextDatum(TC_DATUM);
  spr.setTextColor(BRAND_PAPER_WHITE, bg);
  char line[40];
  snprintf(line, sizeof(line), "ATS Mini firmware  v%d.%02d-R%d", VER_APP / 100, VER_APP % 100, VER_RESONIK);
  spr.drawString(line, W / 2, base + 22, FONT_SMALL);
  spr.setTextColor(0x9CF3, bg);   // light graphite, readable on black
  spr.drawString(FIRMWARE_SITE, W / 2, H - 24, FONT_SMALL);
  spr.setTextDatum(TL_DATUM);

  spr.pushSprite(0, 0);
}
