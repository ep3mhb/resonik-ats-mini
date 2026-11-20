#ifndef FREQ_PAD_H
#define FREQ_PAD_H

// RESONIK R5: number pad to type a frequency (FreqPad.cpp)
void freqPadOpen();
void freqPadTurn(int enc);
void freqPadClick(bool shortPress);
void drawFreqPad();

#endif // FREQ_PAD_H
