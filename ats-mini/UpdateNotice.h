#ifndef UPDATE_NOTICE_H
#define UPDATE_NOTICE_H

// RESONIK R5: new version notice. While Wi-Fi is connected the radio asks
// GitHub for the latest release a few minutes after connecting and then once
// a day, in the background. A newer release opens a small page on the radio
// (Update now / Later / Skip this version), puts a marker beside the Wi-Fi
// icon and a banner on the radio's web pages.

#include <stdint.h>

extern uint8_t updNoticeIdx;                 // 0 Off, 1 On (default On)
extern uint32_t updSkipVersion;              // Release the user chose to skip, 0 none
extern const char *const updNoticeDesc[2];

bool updNoticeDue(uint32_t now);             // Network: time for a background check
void updNoticeChecked(bool ok, uint32_t version);   // Network job: result (any task)
bool updNoticeWaiting();                     // A newer release is out (marker, web banner)
uint32_t updNoticeVersion();                 // That release, major*10000 + minor*100 + release
void updNoticeVersionText(char *buf, int size, uint32_t version);

bool updNoticeTick();                        // Main loop: opens the page when due; true to redraw
void updNoticeTurn(int enc);
void updNoticeClick(bool shortPress);
void drawUpdNotice();

#endif // UPDATE_NOTICE_H
