#ifndef CW_AUDIO_H
#define CW_AUDIO_H

// RESONIK R3: CW decoder fed from the speaker line on GPIO11 (ADC2), which
// the V4B board wires to the ESP32. See CwAudio.cpp.

#include <stdint.h>

extern uint8_t cwStripIdx;                 // Decoder strip in CW mode: 0 Off, 1 On
extern const char *const cwStripDesc[2];

void cwAudioInit();
bool cwAudioTick();                        // From the main loop; true when the screen needs a redraw
void cwPageOpen();                         // Full-screen decoder page
void cwPageClear();
bool cwStripShown();                       // The strip replaces the scale on the normal screen
void drawCwStrip();
void drawCwPage();

// Sampler shared with the FT8 page (Ft8.cpp)
void audioFt8Ring(int16_t *ring, uint32_t size);   // nullptr gives it back to CW
uint32_t audioFt8Written();
void audioWebStream(bool on);                          // Web Control audio (8 kHz) on or off
bool audioWebStreaming();
void audioTimedStats(uint32_t *avgUs, uint32_t *maxUs, uint32_t *late);   // ADC read time, samples dropped
uint32_t audioWebRead(int16_t *dst, uint32_t max, uint32_t *pos);                          // Samples written into the ring so far
uint16_t audioInputLevel();
uint32_t audioTimingRestarts();                      // Times the sampler fell behind (samples lost)
uint32_t audioAdcErrors();                           // Failed ADC reads                          // Mean deviation on GPIO11, ADC counts

#endif // CW_AUDIO_H
