#ifndef BAND_VIEW_H
#define BAND_VIEW_H

// RESONIK R5: Band View waterfall and Auto-Store (BandView.cpp)
void bandViewOpen();
void bandViewTurn(int enc);
void bandViewClick(bool shortPress);
bool bandViewTick();
void drawBandView();
void autoStoreRun();

#endif // BAND_VIEW_H
