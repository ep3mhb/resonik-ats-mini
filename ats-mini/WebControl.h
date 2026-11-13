#ifndef WEB_CONTROL_H
#define WEB_CONTROL_H

// RESONIK R4 (test): live screen, controls and audio in the browser (/control)

#include <ESPAsyncWebServer.h>

void webControlInit(AsyncWebServer &server, bool (*authenticated)(AsyncWebServerRequest *));
bool webControlTick();     // Main loop; true when the screen needs a redraw

#endif
