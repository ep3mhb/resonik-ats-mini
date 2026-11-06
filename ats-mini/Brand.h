#ifndef BRAND_H
#define BRAND_H

#include <stdint.h>

// RESONIK brand colours (RGB565), from the brand guidelines
#define BRAND_PEAK_BLUE    0x1BFA  // #1C7ED6
#define BRAND_SIGNAL_BLACK 0x10A2  // #141414
#define BRAND_PAPER_WHITE  0xF7BE  // #F6F5F1
#define BRAND_GRAPHITE     0x52AA  // #555555

// Seven-bar spectrum symbol, bottom-left corner at (x, yBottom), peak bar h px tall.
// Returns the symbol width in pixels.
int brandDrawSymbol(int x, int yBottom, int h, uint16_t sideColor, uint16_t peakColor);

// RESONIK wordmark (Sora SemiBold), top-left at (x, y), anti-aliased against bg.
// large = boot screen size (cap height 30 px), otherwise About size (13 px).
// Returns the wordmark width in pixels.
int brandDrawWordmark(int x, int y, bool large, uint16_t color, uint16_t bg);
int brandWordmarkWidth(bool large);
int brandWordmarkHeight(bool large);

// Built-in boot screen, shown when no custom splash image has been uploaded
void brandDrawBootScreen();

#endif // BRAND_H
