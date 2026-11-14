//
// RESONIK R4: the Wi-Fi page on the radio (Settings -> Wi-Fi, one item for
// everything Wi-Fi): status and IP address, Mode, Join a network, Power.
// Join a network lists the networks in range (name, signal, security, channel), and types
// the password with the knob on an on-screen keyboard: turn to move, click
// to type, press to go back. The network is saved in the first Wi-Fi slot
// (the others move down) and the radio connects at once, keeping its own
// access point (AP+Connect) unless it was set to Connect only.
//
// The ESP32-S3 has a 2.4 GHz radio only: 5 GHz networks cannot be seen or
// joined, which the list says.
//
#include "Common.h"
#include "Themes.h"
#include "Utils.h"
#include "Menu.h"
#include "Draw.h"
#include "Storage.h"
#include "WifiSetup.h"
#include <WiFi.h>

#define WS_RGB(c)  ((uint16_t)((((c) >> 8) & 0xF800) | (((c) >> 5) & 0x07E0) | (((c) >> 3) & 0x001F)))
// Colours: the tile look with the tile menu; the radio's theme (as the
// classic side box) with the classic menu
struct WsPalette { uint16_t bg, navy, line, green, blue, yellow, red, muted, text, hlText, edit, ok; };
static WsPalette pal;
#define WS_BG      pal.bg
#define WS_NAVY    pal.navy
#define WS_LINE    pal.line
#define WS_GREEN   pal.green
#define WS_BLUE    pal.blue
#define WS_YELLOW  pal.yellow
#define WS_RED     pal.red
#define WS_MUTED   pal.muted
#define WS_TEXT    pal.text
#define WS_HLTEXT  pal.hlText
#define WS_EDIT    pal.edit
#define WS_OK      pal.ok

static void paletteSet()
{
  if(menuStyleIdx == MENU_STYLE_TILES)
    pal = { WS_RGB(0x041F29), WS_RGB(0x073A4B), WS_RGB(0x0E4A5E), WS_RGB(0x06D7A0), WS_RGB(0x108AB1),
            WS_RGB(0xFFD167), WS_RGB(0xFF6B6B), WS_RGB(0x8FB3C0), WS_RGB(0xEEF5F7), WS_RGB(0x041F29), WS_RGB(0x06D7A0), WS_RGB(0x108AB1) };
  else
    pal = { TH.bg, TH.menu_bg, TH.menu_border, TH.menu_hl_bg, TH.menu_hl_bg,
            WS_RGB(0xFFD167), WS_RGB(0xFF6B6B), TH.menu_hdr, TH.menu_item, TH.menu_hl_text, TH.menu_hl_bg, TH.menu_border };
}

enum class Ws : uint8_t { Hub, Scan, List, Keys, Connect, Done, Failed };
static Ws ws = Ws::Hub;

// The Wi-Fi page: three rows; a click on Mode or Power edits it in place
enum { H_MODE, H_JOIN, H_POWER, H_COUNT };
static int hubSel = H_MODE;
static bool hubEdit = false;
static int hubValue = 0;

struct WsNet { char ssid[33]; int8_t rssi; uint8_t ch; uint8_t auth; };
#define WS_MAX_NETS 16
static WsNet nets[WS_MAX_NETS];
static int netCount = 0;
static int listSel = 0;            // 0 = Scan again, 1.. = networks
static bool startedRadio = false;  // Wi-Fi was off: switched on for the scan
static uint32_t stateTime = 0;

static char ssid[33];
static char pass[64];
static uint8_t passLen = 0;
static bool showPass = false;
static uint8_t keyPage = 0;        // 0 abc, 1 ABC, 2 symbols
static int keySel = 0;

//
// Keyboard: four rows of ten characters, then six action keys
//
static const char *const keyRows[3][4] =
{
  { "1234567890", "qwertyuiop", "asdfghjkl-", "zxcvbnm._@" },
  { "1234567890", "QWERTYUIOP", "ASDFGHJKL-", "ZXCVBNM._@" },
  { "!\"#$%&'()*", "+,-./:;<=>", "?@[\\]^_`{|", "}~<>;:,?/=" },
};
enum { K_SHIFT = 40, K_SYM, K_SPACE, K_DEL, K_SHOW, K_OK, K_COUNT };
static const char *const actionLabel[6] = { "aA", "#+=", "Space", "Del", "Show", "OK" };

static const char *authName(uint8_t a)
{
  switch(a)
  {
    case WIFI_AUTH_OPEN:            return("Open");
    case WIFI_AUTH_WEP:             return("WEP");
    case WIFI_AUTH_WPA_PSK:         return("WPA");
    case WIFI_AUTH_WPA2_PSK:        return("WPA2");
    case WIFI_AUTH_WPA_WPA2_PSK:    return("WPA2");
    case WIFI_AUTH_WPA2_ENTERPRISE: return("Ent.");
    case WIFI_AUTH_WPA3_PSK:        return("WPA3");
    case WIFI_AUTH_WPA2_WPA3_PSK:   return("WPA3");
    default:                        return("Sec.");
  }
}

static void startScan()
{
  if(WiFi.getMode() == WIFI_MODE_NULL)
  {
    WiFi.mode(WIFI_STA);
    startedRadio = true;
  }
  WiFi.scanDelete();
  WiFi.scanNetworks(true, false);
  netCount = 0;
  listSel = 0;
  ws = Ws::Scan;
  stateTime = millis();
}

static void collectScan(int n)
{
  netCount = 0;
  for(int i = 0; i < n; i++)
  {
    String name = WiFi.SSID(i);
    if(!name.length()) continue;                 // Hidden network
    int8_t r = WiFi.RSSI(i);
    // The same network from several access points: keep the strongest
    int k;
    for(k = 0; k < netCount; k++) if(!strcmp(nets[k].ssid, name.c_str())) break;
    if(k < netCount) { if(r > nets[k].rssi) { nets[k].rssi = r; nets[k].ch = WiFi.channel(i); } continue; }
    if(netCount >= WS_MAX_NETS) continue;
    WsNet &w = nets[netCount++];
    strlcpy(w.ssid, name.c_str(), sizeof(w.ssid));
    w.rssi = r;
    w.ch = WiFi.channel(i);
    w.auth = WiFi.encryptionType(i);
  }
  // Strongest first
  for(int a = 0; a < netCount; a++)
    for(int b = a + 1; b < netCount; b++)
      if(nets[b].rssi > nets[a].rssi) { WsNet t = nets[a]; nets[a] = nets[b]; nets[b] = t; }
  WiFi.scanDelete();
  ws = Ws::List;
  listSel = netCount ? 1 : 0;
}

// Save as network 1 (moving the others down; the same name is replaced)
// and connect
static void saveAndConnect()
{
  String s[3], p[3];
  prefs.begin("network", false, STORAGE_PARTITION);
  for(int i = 0; i < 3; i++)
  {
    char k[16];
    snprintf(k, sizeof(k), "wifissid%d", i + 1); s[i] = prefs.getString(k, "");
    snprintf(k, sizeof(k), "wifipass%d", i + 1); p[i] = prefs.getString(k, "");
  }
  String ns[3] = { ssid, "", "" }, np[3] = { pass, "", "" };
  int o = 1;
  for(int i = 0; i < 3 && o < 3; i++)
    if(s[i].length() && s[i] != ssid) { ns[o] = s[i]; np[o] = p[i]; o++; }
  for(int i = 0; i < 3; i++)
  {
    char k[16];
    snprintf(k, sizeof(k), "wifissid%d", i + 1); prefs.putString(k, ns[i]);
    snprintf(k, sizeof(k), "wifipass%d", i + 1); prefs.putString(k, np[i]);
  }
  prefs.end();

  // Connect, keeping the access point unless the radio was on Connect only
  if(wifiModeIdx != NET_CONNECT) wifiModeIdx = NET_AP_CONNECT;
  prefsRequestSave(SAVE_SETTINGS);
  startedRadio = false;
  netInit(wifiModeIdx);
  ws = Ws::Connect;
  stateTime = millis();
}

void wifiSetupOpen()
{
  currentCmd = CMD_WIFISETUP;
  startedRadio = false;
  ws = Ws::Hub;
  hubEdit = false;
}

// Back to the Wi-Fi page (from the list, keyboard or result)
static void toHub()
{
  WiFi.scanDelete();
  if(startedRadio) WiFi.mode(WIFI_MODE_NULL);
  startedRadio = false;
  ws = Ws::Hub;
  hubEdit = false;
}

static void leave()
{
  if(ws == Ws::Hub || ws == Ws::Scan || ws == Ws::List || ws == Ws::Keys)
  {
    WiFi.scanDelete();
    if(startedRadio) WiFi.mode(WIFI_MODE_NULL);
  }
  startedRadio = false;
  currentCmd = CMD_NONE;
}

static void typeKey(int k)
{
  if(k < 40)
  {
    char c = keyRows[keyPage][k / 10][k % 10];
    if(passLen < sizeof(pass) - 1) { pass[passLen++] = c; pass[passLen] = 0; }
    // Capitals: back to small letters after one, as on a phone
    if(keyPage == 1 && k >= 10) keyPage = 0;
    return;
  }
  switch(k)
  {
    case K_SHIFT: keyPage = keyPage == 0 ? 1 : 0; break;
    case K_SYM:   keyPage = keyPage == 2 ? 0 : 2; break;
    case K_SPACE: if(passLen < sizeof(pass) - 1) { pass[passLen++] = ' '; pass[passLen] = 0; } break;
    case K_DEL:   if(passLen) pass[--passLen] = 0; break;
    case K_SHOW:  showPass = !showPass; break;
    case K_OK:    if(passLen >= 8) saveAndConnect(); break;   // WPA needs 8 or more
  }
}

void wifiSetupTurn(int16_t enc)
{
  if(!enc) return;
  if(ws == Ws::Hub)
  {
    if(!hubEdit) hubSel = (hubSel + enc + H_COUNT) % H_COUNT;
    else if(hubSel == H_MODE) hubValue = (hubValue + enc + getTotalWiFiModes()) % getTotalWiFiModes();
    else hubValue = (hubValue + enc + 3) % 3;
    return;
  }
  if(ws == Ws::List)
    listSel = (listSel + enc + netCount + 1) % (netCount + 1);
  else if(ws == Ws::Keys)
    keySel = (keySel + enc + K_COUNT) % K_COUNT;
}

void wifiSetupClick(bool shortPress)
{
  switch(ws)
  {
    case Ws::Hub:
      if(shortPress) { if(hubEdit) hubEdit = false; else leave(); break; }
      if(!hubEdit)
      {
        if(hubSel == H_JOIN) { startScan(); break; }
        hubEdit = true;
        hubValue = hubSel == H_MODE ? wifiModeIdx : wifiPowerIdx;
        break;
      }
      // Apply the new mode or power
      hubEdit = false;
      if(hubSel == H_MODE && hubValue != wifiModeIdx)
      {
        wifiModeIdx = hubValue;
        prefsRequestSave(SAVE_SETTINGS);
        netInit(wifiModeIdx);
      }
      else if(hubSel == H_POWER && hubValue != wifiPowerIdx)
      {
        wifiPowerIdx = hubValue;
        wifiApplyTxPower();
        prefsRequestSave(SAVE_SETTINGS);
      }
      break;
    case Ws::Scan:
      if(shortPress) toHub();
      break;
    case Ws::List:
      if(shortPress) { toHub(); break; }
      if(listSel == 0) { startScan(); break; }
      strlcpy(ssid, nets[listSel - 1].ssid, sizeof(ssid));
      pass[0] = 0; passLen = 0; showPass = false; keyPage = 0; keySel = 11;
      if(nets[listSel - 1].auth == WIFI_AUTH_OPEN) saveAndConnect();
      else ws = Ws::Keys;
      break;
    case Ws::Keys:
      if(shortPress) { ws = Ws::List; break; }       // Back to the list
      typeKey(keySel);
      break;
    case Ws::Connect:
      if(shortPress) toHub();
      break;
    case Ws::Done:
      toHub();
      break;
    case Ws::Failed:
      if(shortPress) toHub();
      else { ws = Ws::Keys; keySel = K_OK; }          // Try the password again
      break;
  }
}

bool wifiSetupTick()
{
  if(currentCmd != CMD_WIFISETUP) return(false);
  uint32_t now = millis();
  if(ws == Ws::Hub)
  {
    static uint32_t last = 0;
    if(now - last > 1000) { last = now; return(true); }
    return(false);
  }
  if(ws == Ws::Scan)
  {
    int n = WiFi.scanComplete();
    if(n >= 0) { collectScan(n); return(true); }
    if(n == WIFI_SCAN_FAILED && now - stateTime > 500) { collectScan(0); return(true); }
    static uint32_t dots = 0;
    if(now - dots > 300) { dots = now; return(true); }
    return(false);
  }
  if(ws == Ws::Connect)
  {
    if(WiFi.status() == WL_CONNECTED && getWiFiIPAddress()[0]) { ws = Ws::Done; return(true); }
    if(now - stateTime > 25000) { ws = Ws::Failed; return(true); }
    static uint32_t dots = 0;
    if(now - dots > 300) { dots = now; return(true); }
  }
  return(false);
}

// One line for the Wi-Fi menu: where to find the radio's web pages
const char *wifiInfoLine()
{
  static char line[64];
  int8_t st = getWiFiStatus();
  wifi_mode_t m = WiFi.getMode();
  if(st == 2) snprintf(line, sizeof(line), "IP %s  or atsmini.local", getWiFiIPAddress());
  else if(m == WIFI_AP || m == WIFI_AP_STA) snprintf(line, sizeof(line), "Access point %s: 10.1.1.1", RECEIVER_NAME);
  else if(m == WIFI_MODE_NULL) strlcpy(line, "Wi-Fi off", sizeof(line));
  else strlcpy(line, "Not connected", sizeof(line));
  return(line);
}

//
// Drawing (320 x 170)
//
static void header(const char *title, const char *right)
{
  spr.fillSprite(WS_BG);
  spr.fillRoundRect(4, 3, 312, 24, 6, WS_NAVY);
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(WS_TEXT);
  spr.drawString(title, 12, 15, uiFont(false));
  if(right)
  {
    spr.setTextDatum(MR_DATUM);
    spr.setTextColor(WS_MUTED);
    spr.drawString(right, 308, 15, uiFont(true));
  }
}

static void footer(const char *text)
{
  spr.setTextDatum(BC_DATUM);
  spr.setTextColor(WS_MUTED);
  spr.drawString(text, 160, 169, uiFont(true));
}

static void bars(int x, int y, int8_t rssi)
{
  int n = rssi > -55 ? 4 : rssi > -67 ? 3 : rssi > -77 ? 2 : 1;
  for(int i = 0; i < 4; i++)
    spr.fillRect(x + i * 5, y - 3 * (i + 1), 3, 3 * (i + 1), i < n ? WS_RGB(0x06D7A0) : WS_RGB(0x3A4A50));
}

static void drawList()
{
  char right[24];
  snprintf(right, sizeof(right), "%d found  2.4 GHz", netCount);
  header("Join a network", right);
  const int top = 31, rh = 22, vis = 5;
  int first = listSel - 2;
  if(first > netCount + 1 - vis) first = netCount + 1 - vis;
  if(first < 0) first = 0;
  for(int k = 0; k < vis && first + k <= netCount; k++)
  {
    int i = first + k, y = top + k * (rh + 2);
    bool hl = i == listSel;
    spr.fillRoundRect(4, y, 312, rh, 5, hl ? WS_BLUE : WS_NAVY);
    spr.setTextDatum(ML_DATUM);
    if(i == 0)
    {
      spr.setTextColor(hl ? WS_HLTEXT : WS_TEXT);
      spr.drawString(netCount ? "Scan again" : "No networks found: scan again", 12, y + rh / 2, uiFont(true));
      continue;
    }
    const WsNet &w = nets[i - 1];
    bars(12, y + 17, w.rssi);
    spr.setTextColor(hl ? WS_HLTEXT : WS_TEXT);
    char name[24];
    strlcpy(name, w.ssid, sizeof(name));
    spr.drawString(name, 38, y + rh / 2, uiFont(true));
    char info[24];
    snprintf(info, sizeof(info), "%s  ch%u  %d", authName(w.auth), w.ch, w.rssi);
    spr.setTextDatum(MR_DATUM);
    spr.setTextColor(hl ? WS_HLTEXT : (w.auth == WIFI_AUTH_OPEN ? WS_YELLOW : WS_MUTED));
    spr.drawString(info, 308, y + rh / 2, uiFont(true));
  }
  footer("Turn: choose   Click: select   Press: back");
}

static void drawKeys()
{
  header(ssid, nullptr);
  // Password field
  spr.fillRoundRect(4, 30, 312, 22, 5, WS_NAVY);
  char shown[66];
  if(showPass || !passLen) strlcpy(shown, passLen ? pass : "Password", sizeof(shown));
  else
  {
    // Dots, with the last character visible so typos show
    int n = passLen - 1;
    for(int i = 0; i < n && i < 60; i++) shown[i] = '*';
    shown[n < 60 ? n : 60] = pass[passLen - 1];
    shown[(n < 60 ? n : 60) + 1] = 0;
  }
  // Keep the end of a long password in view
  const char *view = shown;
  while(spr.textWidth(view, uiFont(true)) > 270) view++;
  spr.setTextDatum(ML_DATUM);
  spr.setTextColor(passLen ? WS_TEXT : WS_MUTED);
  spr.drawString(view, 10, 41, uiFont(true));
  int cx = 10 + (passLen ? spr.textWidth(view, uiFont(true)) : 0);
  spr.fillRect(cx + 1, 33, 2, 16, WS_GREEN);
  char count[8];
  snprintf(count, sizeof(count), "%u", passLen);
  spr.setTextDatum(MR_DATUM);
  spr.setTextColor(passLen >= 8 ? WS_GREEN : WS_MUTED);
  spr.drawString(count, 310, 41, uiFont(true));

  // Character keys
  const int kx = 4, ky = 56, kw = 31, kh = 20;
  for(int k = 0; k < 40; k++)
  {
    int r = k / 10, c = k % 10, x = kx + c * kw, y = ky + r * (kh + 2);
    bool hl = k == keySel;
    spr.fillRoundRect(x, y, kw - 2, kh, 4, hl ? WS_GREEN : WS_NAVY);
    char t[2] = { keyRows[keyPage][r][c], 0 };
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(hl ? WS_HLTEXT : WS_TEXT);
    spr.drawString(t, x + (kw - 2) / 2, y + kh / 2 + 1, uiFont(false));
  }
  // Action keys
  const int ay = ky + 4 * (kh + 2), aw = 52;
  for(int a = 0; a < 6; a++)
  {
    int k = 40 + a, x = kx + a * aw;
    bool hl = k == keySel;
    bool on = (a == 0 && keyPage == 1) || (a == 1 && keyPage == 2) || (a == 4 && showPass);
    bool okOff = a == 5 && passLen < 8;
    uint16_t bg = hl ? WS_GREEN : a == 5 ? (okOff ? WS_NAVY : WS_OK) : on ? WS_LINE : WS_NAVY;
    spr.fillRoundRect(x, ay, aw - 2, kh, 4, bg);
    spr.setTextDatum(MC_DATUM);
    spr.setTextColor(hl ? WS_HLTEXT : okOff ? WS_MUTED : WS_TEXT);
    spr.drawString(a == 4 && showPass ? "Hide" : actionLabel[a], x + (aw - 2) / 2, ay + kh / 2 + 1, uiFont(true));
  }
}

static void drawHub()
{
  int8_t st = getWiFiStatus();
  wifi_mode_t m = WiFi.getMode();
  header("Wi-Fi", st == 2 ? "Connected" : m == WIFI_MODE_NULL ? "Off" : (m == WIFI_AP || m == WIFI_AP_STA) ? "Access point" : "Not connected");
  // Where to find the radio's web pages
  spr.fillRoundRect(4, 30, 312, 40, 6, WS_NAVY);
  spr.setTextDatum(MC_DATUM);
  // Line 1: the address to open in a browser; line 2: how to get there
  char l2[64];
  if(st == 2)
  {
    spr.setTextColor(WS_GREEN);
    spr.drawString(getWiFiIPAddress(), 160, 42, uiFont(false));
    snprintf(l2, sizeof(l2), "on %s, or atsmini.local", WiFi.SSID().c_str());
  }
  else if(m == WIFI_AP || m == WIFI_AP_STA)
  {
    spr.setTextColor(WS_TEXT);
    spr.drawString("10.1.1.1", 160, 42, uiFont(false));
    snprintf(l2, sizeof(l2), "after joining Wi-Fi %s", RECEIVER_NAME);
  }
  else
  {
    spr.setTextColor(WS_TEXT);
    spr.drawString(m == WIFI_MODE_NULL ? "Wi-Fi off" : "Not connected", 160, 42, uiFont(false));
    strlcpy(l2, m == WIFI_MODE_NULL ? "Set Mode to use the web pages" : "Join a network below", sizeof(l2));
  }
  spr.setTextColor(WS_MUTED);
  spr.drawString(l2, 160, 60, uiFont(true));

  static const char *const labels[H_COUNT] = { "Mode", "Join a network", "Power" };
  const int top = 74, rh = 22;
  for(int i = 0; i < H_COUNT; i++)
  {
    int y = top + i * (rh + 3);
    bool hl = i == hubSel;
    spr.fillRoundRect(4, y, 312, rh, 6, hl ? (hubEdit ? WS_EDIT : WS_BLUE) : WS_NAVY);
    spr.setTextDatum(ML_DATUM);
    spr.setTextColor(hl ? WS_HLTEXT : WS_TEXT);
    spr.drawString(labels[i], 12, y + rh / 2, uiFont(false));
    const char *v = nullptr;
    if(i == H_MODE)  v = wifiModeDesc[hl && hubEdit ? hubValue : wifiModeIdx];
    if(i == H_POWER) v = wifiPowerDesc[hl && hubEdit ? hubValue : wifiPowerIdx];
    if(i == H_JOIN)  v = ">";
    spr.setTextDatum(MR_DATUM);
    spr.setTextColor(hl ? WS_HLTEXT : WS_MUTED);
    if(hl && hubEdit) { char t[32]; snprintf(t, sizeof(t), "< %s >", v); spr.drawString(t, 308, y + rh / 2, uiFont(false)); }
    else spr.drawString(v, 308, y + rh / 2, uiFont(false));
  }
  footer(hubEdit ? "Turn: change   Click: apply   Press: cancel" : "Turn: choose   Click: open   Press: back");
}

void drawWifiSetup()
{
  paletteSet();
  char line[48];
  switch(ws)
  {
    case Ws::Hub:
      drawHub();
      break;
    case Ws::Scan:
    {
      header("Join a network", "2.4 GHz");
      static const char *const d[4] = { "", ".", "..", "..." };
      snprintf(line, sizeof(line), "Looking for networks%s", d[(millis() / 300) % 4]);
      spr.setTextDatum(MC_DATUM);
      spr.setTextColor(WS_TEXT);
      spr.drawString(line, 160, 80, uiFont(false));
      spr.setTextColor(WS_MUTED);
      spr.drawString("The radio sees 2.4 GHz networks only", 160, 104, uiFont(true));
      footer("Press: back");
      break;
    }
    case Ws::List:
      drawList();
      break;
    case Ws::Keys:
      drawKeys();
      break;
    case Ws::Connect:
    {
      header("Join a network", nullptr);
      static const char *const d[4] = { "", ".", "..", "..." };
      snprintf(line, sizeof(line), "Connecting%s", d[(millis() / 300) % 4]);
      spr.setTextDatum(MC_DATUM);
      spr.setTextColor(WS_TEXT);
      spr.drawString(line, 160, 72, uiFont(false));
      spr.setTextColor(WS_MUTED);
      spr.drawString(ssid, 160, 96, uiFont(true));
      footer("Press: leave (it keeps trying)");
      break;
    }
    case Ws::Done:
    {
      header("Connected", ssid);
      spr.setTextDatum(MC_DATUM);
      spr.setTextColor(WS_GREEN);
      spr.drawString(getWiFiIPAddress(), 160, 66, &lgfx::fonts::FreeSansBold18pt7b);
      spr.setTextColor(WS_TEXT);
      spr.drawString("Open this address in a browser", 160, 98, uiFont(true));
      spr.setTextColor(WS_MUTED);
      snprintf(line, sizeof(line), "http://%s/control", getWiFiIPAddress());
      spr.drawString(line, 160, 118, uiFont(true));
      spr.drawString("or http://atsmini.local", 160, 136, uiFont(true));
      footer("Click: back to Wi-Fi");
      break;
    }
    case Ws::Failed:
      header("Not connected", ssid);
      spr.setTextDatum(MC_DATUM);
      spr.setTextColor(WS_RED);
      spr.drawString("Could not connect", 160, 70, uiFont(false));
      spr.setTextColor(WS_MUTED);
      spr.drawString("Wrong password, or the network is too weak", 160, 96, uiFont(true));
      footer("Click: retype password   Press: leave");
      break;
  }
  spr.pushSprite(0, 0);
}
