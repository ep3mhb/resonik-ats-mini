#ifndef WIFI_SETUP_H
#define WIFI_SETUP_H

// RESONIK R4: Wi-Fi Setup page on the radio (WifiSetup.cpp)

void wifiSetupOpen();
void wifiSetupTurn(int16_t enc);
void wifiSetupClick(bool shortPress);
bool wifiSetupTick();              // Main loop; true when the page needs a redraw
void drawWifiSetup();
const char *wifiInfoLine();        // "IP 192.168.1.20 · atsmini.local", access point, or off

#endif
