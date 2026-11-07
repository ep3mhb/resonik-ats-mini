#ifndef RESONIK_H
#define RESONIK_H

// RESONIK Phase 2: beacon tracker, memory scanner, signal logger,
// propagation data and Kenwood CAT. See Resonik.cpp.

#include "Common.h"
#include "Propagation.h"

// Tune any frequency and mode, switching band when needed
bool tuneToFrequency(uint32_t hz, uint8_t mode);

// NCDXF/IARU beacons
const char *beaconLabel(uint32_t freq);      // Station line text on a beacon frequency, or nullptr
extern int8_t beaconFollowIdx;               // Beacon being followed across bands, -1 = off
extern const char *const beaconMenuLabels[19];
void beaconFollow(int8_t idx);
bool beaconTick();

// Memory scanner
bool memScanActive();
void memScanStart();
void memScanStop(const char *why = nullptr);
bool memScanTick();

// Signal logger (CSV in the flash file system)
#define LOG_PATH   "/log.csv"
#define LOG_CLEAR  4
extern uint8_t loggerIdx;
extern const char *const loggerDesc[5];
bool loggerTick();
void loggerClear();
size_t loggerSize();

// Propagation (hamqsl.com)
bool propagationFetch();
const PropData *propagationData();
void drawPropagation();

// Kenwood CAT on the USB port
bool catSerialLoop();

#endif // RESONIK_H
