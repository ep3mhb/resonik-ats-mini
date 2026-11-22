// PC preview of the tile menu and its value screens, drawn by the firmware's
// own code: TileDraw.h, TileMenuData.h and, through gen.h, the menu tables
// and tileDrawValue() of Menu.cpp. Built and run by tools/tile_preview.py.
//
//   preview check                         every tile label against its tile
//   preview page OUT.ppm LABEL            the tile page that holds LABEL, LABEL selected
//   preview value OUT.ppm LABEL IDX A B.. LABEL's value screen with the list A B.., IDX selected
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
enum TileIcon : uint8_t;
#include "TileDraw.h"
#include "TileMenuData.h"
#define ML_DATUM middle_left
#define MR_DATUM middle_right
#define MC_DATUM middle_center
#define BC_DATUM bottom_center
static LGFX_Sprite spr;
static char **valLabels = nullptr;
static const char *tileValueLabel(uint16_t, int i) { return valLabels[i]; }
static const char *wifiInfoLine() { return ""; }
#include "gen.h"

static void save(const char *name)
{
  FILE *f = fopen(name, "wb");
  if(!f) { perror(name); exit(1); }
  fprintf(f, "P6\n320 170\n255\n");
  for(int y = 0; y < 170; y++)
    for(int x = 0; x < 320; x++)
    {
      uint16_t c = spr.readPixel(x, y);
      uint8_t rgb[3] = { (uint8_t)(((c >> 11) & 31) * 255 / 31), (uint8_t)(((c >> 5) & 63) * 255 / 63), (uint8_t)((c & 31) * 255 / 31) };
      fwrite(rgb, 1, 3, f);
    }
  fclose(f);
}

static bool find(const char *label, int &cat, int &item)
{
  for(int c = 0; c < (int)ITEM_COUNT(tileCategories); c++)
    for(int i = 0; i < tileCategories[c].count; i++)
      if(!strcmp(tileLabel(tileCategories[c].items[i].id), label)) { cat = c; item = i; return true; }
  fprintf(stderr, "No tile is called \"%s\"\n", label);
  return false;
}

// As drawTileMenu() in Menu.cpp, for a page inside a category
static void page(int cat, int item, const char *name)
{
  tileCat = cat;
  tileSlot = (item / 5) * 6 + 1 + item % 5;
  const TileCategory &c = tileCategories[tileCat];
  TileItem items[6];
  int count = 0, pg = tileSlot / 6, pages = (c.count + 4) / 5, sel = tileSlot % 6;
  items[count++] = tileMake("Back", TI_BACK, 5);
  for(int i = pg * 5 ; i < c.count && count < 6 ; i++)
    items[count++] = tileMake(tileLabel(c.items[i].id), c.items[i].icon, (i + tileCat * 3 + 1) % 5);
  tileDrawPage(spr, items, count, sel, 0x0000, 0x7BEF, tileFont(false), false, &tileStyle, pg, pages, tileFont(true));
  save(name);
  printf("%s: %s, page %d of %d\n", name, c.label, pg + 1, pages);
}

int main(int argc, char **argv)
{
  spr.setColorDepth(16);
  spr.createSprite(320, 170);
  int c, i;
  if(argc >= 2 && !strcmp(argv[1], "check"))
  {
    // A label is drawn in the 15 px font, or in the 14 px one when wider than the tile's 92 px
    int wide = 0;
    for(c = 0; c < (int)ITEM_COUNT(tileCategories); c++)
      for(i = 0; i < tileCategories[c].count; i++)
      {
        const char *l = tileLabel(tileCategories[c].items[i].id);
        int w15 = spr.textWidth(l, tileFont(false)), w14 = spr.textWidth(l, tileFont(true));
        if((w15 > 92 ? w14 : w15) > 92) { printf("TOO WIDE: %s, %d px in the small font (tile: 92)\n", l, w14); wide++; }
      }
    printf("%d tile label%s too wide\n", wide, wide == 1 ? "" : "s");
    return wide ? 1 : 0;
  }
  if(argc == 4 && !strcmp(argv[1], "groups"))
  {
    // The first page of the menu: the groups, one of them chosen
    tileCat = -1;
    TileItem items[6];
    int count = 0;
    for(int k = 0; k < (int)ITEM_COUNT(tileCategories) && count < 6; k++)
      items[count++] = tileMake(tileCategories[k].label, tileCategories[k].icon, k);
    tileDrawPage(spr, items, count, atoi(argv[3]), 0x0000, 0x7BEF, tileFont(false), false, &tileStyle, 0, 1, tileFont(true));
    save(argv[2]);
    return 0;
  }
  if(argc == 4 && !strcmp(argv[1], "page"))
  {
    if(!find(argv[3], c, i)) return 1;
    page(c, i, argv[2]);
    return 0;
  }
  if(argc >= 6 && !strcmp(argv[1], "value"))
  {
    if(!find(argv[3], c, i)) return 1;
    tileCat = c; tileValueItem = i; valLabels = argv + 5;
    TileValue v;
    v.list = true; v.count = argc - 5; v.idx = atoi(argv[4]); v.unit = nullptr; v.pct = -1; v.value[0] = '\0';
    tileDrawValue(0, v);
    save(argv[2]);
    for(int k = 0; k < v.count; k++)
    {
      int w = spr.textWidth(valLabels[k], tileFont(false));
      if(w > 280) printf("TOO WIDE: %s, %d px (row: 280)\n", valLabels[k], w);
    }
    printf("%s: %s\n", argv[2], argv[3]);
    return 0;
  }
  fprintf(stderr, "See the top of tools/tile_preview.cpp\n");
  return 1;
}
