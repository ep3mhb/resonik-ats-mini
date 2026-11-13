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
uint32_t audioFt8Written();                          // Samples written into the ring so far
uint16_t audioInputLevel();
uint32_t audioTimingRestarts();                      // Times the sampler fell behind (samples lost)
uint32_t audioAdcErrors();                           // Failed ADC reads                          // Mean deviation on GPIO11, ADC counts

#endif // CW_AUDIO_H
