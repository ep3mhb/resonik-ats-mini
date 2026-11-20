#ifndef DIGI_PAGE_H
#define DIGI_PAGE_H

#include <stdint.h>

// RESONIK R5: RTTY, NAVTEX and PSK31 page (DigiPage.cpp)
void digiPageOpen();
void digiClick(bool shortPress);   // Click: next mode; short press: leave
bool digiTick();                   // Main loop; true when the screen needs a redraw
void drawDigiPage();

// In Menu.cpp: USB with the 3.0k filter, on hz (0: where the radio is)
void digiSetUsb(uint32_t hz);

#endif // DIGI_PAGE_H
