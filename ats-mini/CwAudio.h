#ifndef CW_AUDIO_H
#define CW_AUDIO_H

// RESONIK R3: CW decoder fed from the speaker line on GPIO11 (ADC2), which
// the V4B board wires to the ESP32. See CwAudio.cpp.

#include <stdint.h>

// R4: receiver volume while the CW page or web Listen uses the audio, so the
// level at GPIO11 does not depend on the user's volume (FT8 uses its own 50)
#define AUDIO_LINE_VOLUME 45

// R5: receiver volume on the CW page, at least: the FT8 page's level (Ft8.cpp)
#define CW_PAGE_VOLUME 50

extern uint8_t cwStripIdx;                 // Decoder strip in CW mode: 0 Off, 1 On
extern const char *const cwStripDesc[2];

void cwAudioInit();
bool cwAudioTick();                        // From the main loop; true when the screen needs a redraw
void cwPageOpen();                         // Full-screen decoder page
void cwPageClear();
bool cwStripShown();                       // The strip replaces the scale on the normal screen
void drawCwStrip();
void drawCwPage();
void cwWebText(char *buf, int size, int *wpm);   // R5: decoded text for the browser

// Sampler shared with the FT8 page (Ft8.cpp)
void audioFt8Ring(int16_t *ring, uint32_t size, int fs = 6000);   // nullptr gives it back to CW; fs 6000 or 12000

// RESONIK R5: Decoder Audio setting (CwAudio.cpp)
extern uint8_t decAudioIdx;                // 0: 3 kHz (default), 1: 6 kHz
extern const char *const decAudioDesc[2];
int decoderSampleRate();                   // 6000 or 12000 Hz
uint32_t audioFt8Written();
void audioWebStream(bool on);                          // Web Control audio (8 kHz) on or off
bool audioWebStreaming();
void audioTimedStats(uint32_t *avgUs, uint32_t *maxUs, uint32_t *late, uint32_t *caughtUp, uint32_t *overrun);   // ADC read time, samples dropped
uint32_t audioWebRead(int16_t *dst, uint32_t max, uint32_t *pos);
uint32_t audioWebBacklog(uint32_t pos);                 // Samples written but not yet read                          // Samples written into the ring so far
uint16_t audioInputLevel();
uint32_t audioTimingRestarts();                      // Times the sampler fell behind (samples lost)
uint32_t audioAdcErrors();                           // Failed ADC reads                          // Mean deviation on GPIO11, ADC counts

#endif // CW_AUDIO_H
