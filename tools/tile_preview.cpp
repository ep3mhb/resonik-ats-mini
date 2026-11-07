// Renders the tile menu pages on a PC with the real LovyanGFX code, for review.
// Build: see tools/README or the RESONIK notes; output: tile-*.raw (RGB565, 320x170)
#define LGFX_USE_V1
#include <LovyanGFX.hpp>
#include <cstdio>
#include <cstring>
#include "../ats-mini/TileMenuData.h"

static const char *mainLabels[] = { "Mode", "Band", "Volume", "VFO A/B", "Step", "Seek", "Scan", "Memory",
  "Squelch", "Bandwidth", "AGC/ATTN", "AVC", "SoftMute", "Beacons", "Mem Scan", "Settings" };
static const char *setLabels[] = { "Brightness", "Date/Time", "Time Zone", "Sleep Timer", "Sleep Mode", "Off Timer",
  "Theme", "UI Layout", "Zoom Menu", "Scroll Dir.", "RDS", "FM Stereo", "FM Region", "Calibration", "DSP Patches",
  "Load EiBi", "Update FW", "USB Port", "TCP Port", "Bluetooth", "Wi-Fi", "Band Plan", "Auto Att.", "Menu Style",
  "Logger", "Propagation", "About" };

static void render(const char *const *labels, const TileIcon *icons, int count, int sel, const char *out)
{
  TileItem items[32];
  for(int i = 0 ; i < count ; i++) items[i] = { labels[i], icons[i], tileColorFor(i) };
  LGFX_Sprite s;
  s.setColorDepth(16);
  s.createSprite(320, 170);
  tileDrawPage(s, items, count, sel, 0x0000, 0x7BEF);
  FILE *f = fopen(out, "wb");
  fwrite(s.getBuffer(), 2, 320 * 170, f);
  fclose(f);
}

int main()
{
  render(mainLabels, tileMainIcons, 16, 0, "tile-main-1.raw");
  render(mainLabels, tileMainIcons, 16, 10, "tile-main-2.raw");
  render(mainLabels, tileMainIcons, 16, 15, "tile-main-3.raw");
  for(int p = 0 ; p < 5 ; p++)
  {
    char name[32];
    snprintf(name, sizeof(name), "tile-set-%d.raw", p + 1);
    render(setLabels, tileSettingsIcons, 27, p * 6 + 1, name);
  }
  printf("ok\n");
  return 0;
}
