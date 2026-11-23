#ifndef DIAG_H
#define DIAG_H

#include <stdint.h>

// RESONIK R5: what the firmware was doing when a watchdog or a crash restarted
// it. The last steps below are kept in RTC memory, which a restart does not
// clear, with the task each core was running when the task watchdog fired;
// after the restart the Status page shows them next to "Last restart", for a
// report. (23 Nov: a watchdog restart when the Digital page went to NAVTEX,
// and no USB log to be had.)
//
//  1 Digital: mode change begins      2 radio tuned for it    3 change done
//  4 Digital: page opened             5 page closed
// 10 band change begins              11 fast SSB change done
// 12 full band setup begins          13 full band setup done
// 20..24 decoder begun for Digital mode 0..4 (RTTY, DWD, NAVTEX, PSK31, WEFAX)
// 30 audio sampling starts           31 audio sampling pauses
// 40 network job begins              41 network job ends
// 50 log line being written          51 log line written
void diagStep(uint8_t code);

// At start, before anything else: keep what the last run recorded if a
// watchdog or a crash ended it, then begin a new record
void diagBoot();

// What the last run recorded, for the Status page; "" if there is nothing
const char *diagLastText();

#endif // DIAG_H
