#ifndef BAND_VIEW_H
#define BAND_VIEW_H

#include <Arduino.h>

// RESONIK R5: Band View waterfall and Auto-Store (BandView.cpp)
void bandViewOpen();
void bandViewTurn(int enc);
void bandViewClick(bool shortPress);
void bandViewLeaveTo(uint32_t hz);   // R5, Web Control: leave and tune (0: only leave)
bool bandViewWebState(uint32_t *seen, uint32_t *seenSweep, String &j);   // R5: for the browser
bool bandViewTick();
void drawBandView();
void autoStoreRun();

#endif // BAND_VIEW_H
