#ifndef FT8_H
#define FT8_H

// RESONIK R3: FT8 page. Decodes FT8 and FT4 from the speaker line on GPIO11 (ADC2)
// with the CW decoder's sampler at 12 kHz. See Ft8.cpp.

#include <stdint.h>

bool ft8Tick();                 // From the main loop; true when the screen needs a redraw
void ft8PageOpen();
void ft8Turn(int16_t enc);      // Knob: next or previous FT8 band
void ft8Clear();                // Click: clear the list
void drawFt8Page();

// In Menu.cpp: FT8 dial frequencies and the radio set-up around the page
int ft8ChannelCount();
int ft8ChannelOfCurrent();      // FT8 channel of the band in use, or -1
void ft8SelectChannel(int ch);  // Band, USB 3.0k on the FT8 dial frequency
const char *ft8ChannelName(int ch);
bool ft8ChannelIsFt4(int ch);    // R3: FT4 channel (7.5 s slots)
void ft8SaveRadio();
void ft8RestoreRadio();

#endif // FT8_H
