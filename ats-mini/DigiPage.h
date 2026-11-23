#ifndef DIGI_PAGE_H
#define DIGI_PAGE_H

#include <stdint.h>

// RESONIK R5: RTTY, NAVTEX and PSK31 page (DigiPage.cpp)
void digiPageOpen();
void digiClick(bool shortPress);   // Click: next mode; short press: leave
bool digiTurn(int16_t enc, bool pressed);   // Push and turn: read the text back; false: the knob tunes
bool digiTick();                   // Main loop; true when the screen needs a redraw
void drawDigiPage();

#include <Arduino.h>
bool digiWebState(uint32_t *seen, String &j);   // R5: for the browser
void digiWebOpen(int a);            // R5: Web Control opens the page in a mode (and channel or frequency)
bool digiWebTune(uint32_t hz);      // R5: a frequency typed in Web Control while the page is open
int digiWebMode();                 // R5: mode being decoded (+ 256 x NAVTEX channel), -1 = page closed
void digiWebText(uint32_t from, String &j);   // R5: the whole text from a character on
bool digiCaptureGet(const int16_t **data, uint32_t *n, uint32_t *rate, char *info, size_t infoSize);   // R5: Save audio
int digiNavCount();                // NAVTEX channels
const char *digiNavName(int i);
int digiModeCount();
const char *digiModeName(int m);

// In Menu.cpp: USB with the 3.0k filter, on hz (0: where the radio is)
void digiSetUsb(uint32_t hz);

#endif // DIGI_PAGE_H
