#ifndef BACKUP_H
#define BACKUP_H

#include <Arduino.h>

// RESONIK R5: backup and restore of all settings (Backup.cpp)
String backupMake();
void backupUpload(size_t index, const uint8_t *data, size_t len, bool final);
const char *backupCheck();   // nullptr when the uploaded file can be restored
bool backupTick();           // Main loop: restore and restart

#endif // BACKUP_H
