//
// RESONIK R5: backup and restore of all settings from the web page.
//
// The backup is a text file with every saved value of the radio (settings,
// bands, memories and Wi-Fi networks), one per line:
//   RESONIK-BACKUP 1 <firmware> <settings> <memories> <bands>
//   <section> TAB <key> TAB <type> TAB <value>
// Strings and blobs are written in hex. A restore is accepted only from the
// same settings, memories and bands versions, replaces those sections
// completely and restarts the radio, so nothing in memory writes the old
// values back.
//
#include "Common.h"
#include "Storage.h"
#include "Backup.h"
#include <nvs.h>
#include <esp_heap_caps.h>

static const char *const sections[] = { "settings", "bands", "memories", "network" };
#define SECTION_COUNT 4
#define BACKUP_MAX    (96 * 1024)

static char *upload = nullptr;
static size_t uploadLen = 0;
static bool uploadTooBig = false;
static volatile bool restoreReady = false;

static const char *typeName(nvs_type_t t)
{
  switch(t)
  {
    case NVS_TYPE_U8: return("u8");   case NVS_TYPE_I8: return("i8");
    case NVS_TYPE_U16: return("u16"); case NVS_TYPE_I16: return("i16");
    case NVS_TYPE_U32: return("u32"); case NVS_TYPE_I32: return("i32");
    case NVS_TYPE_U64: return("u64"); case NVS_TYPE_I64: return("i64");
    case NVS_TYPE_STR: return("str"); case NVS_TYPE_BLOB: return("blob");
    default: return(nullptr);
  }
}

static void hexAppend(String &out, const uint8_t *data, size_t n)
{
  static const char hex[] = "0123456789abcdef";
  for(size_t i = 0; i < n; i++) { out += hex[data[i] >> 4]; out += hex[data[i] & 15]; }
}

static void backupHeader(char *buf, size_t size)
{
  snprintf(buf, size, "RESONIK-BACKUP 1 %u %u %u %u", (unsigned)VER_FULL,
           (unsigned)VER_SETTINGS, (unsigned)VER_MEMORIES, (unsigned)VER_BANDS);
}

String backupMake()
{
  String out;
  out.reserve(16 * 1024);
  char line[64];
  backupHeader(line, sizeof(line));
  out += line; out += '\n';

  for(int s = 0; s < SECTION_COUNT; s++)
  {
    nvs_handle_t h;
    if(nvs_open_from_partition(STORAGE_PARTITION, sections[s], NVS_READONLY, &h) != ESP_OK) continue;
    nvs_iterator_t it = nullptr;
    esp_err_t res = nvs_entry_find(STORAGE_PARTITION, sections[s], NVS_TYPE_ANY, &it);
    while(res == ESP_OK)
    {
      nvs_entry_info_t info;
      nvs_entry_info(it, &info);
      const char *tn = typeName(info.type);
      if(tn)
      {
        String value;
        bool ok = true;
        switch(info.type)
        {
          case NVS_TYPE_U8:  { uint8_t v;  ok = nvs_get_u8(h, info.key, &v) == ESP_OK;  value = String(v); break; }
          case NVS_TYPE_I8:  { int8_t v;   ok = nvs_get_i8(h, info.key, &v) == ESP_OK;  value = String(v); break; }
          case NVS_TYPE_U16: { uint16_t v; ok = nvs_get_u16(h, info.key, &v) == ESP_OK; value = String(v); break; }
          case NVS_TYPE_I16: { int16_t v;  ok = nvs_get_i16(h, info.key, &v) == ESP_OK; value = String(v); break; }
          case NVS_TYPE_U32: { uint32_t v; ok = nvs_get_u32(h, info.key, &v) == ESP_OK; value = String(v); break; }
          case NVS_TYPE_I32: { int32_t v;  ok = nvs_get_i32(h, info.key, &v) == ESP_OK; value = String(v); break; }
          case NVS_TYPE_U64: { uint64_t v; ok = nvs_get_u64(h, info.key, &v) == ESP_OK; char b[24]; snprintf(b, sizeof(b), "%llu", (unsigned long long)v); value = b; break; }
          case NVS_TYPE_I64: { int64_t v;  ok = nvs_get_i64(h, info.key, &v) == ESP_OK; char b[24]; snprintf(b, sizeof(b), "%lld", (long long)v); value = b; break; }
          case NVS_TYPE_STR:
          case NVS_TYPE_BLOB:
          {
            size_t n = 0;
            ok = (info.type == NVS_TYPE_STR ? nvs_get_str(h, info.key, nullptr, &n) : nvs_get_blob(h, info.key, nullptr, &n)) == ESP_OK;
            uint8_t *buf = ok && n ? (uint8_t *)ps_malloc(n) : nullptr;
            if(ok && n && buf)
            {
              ok = (info.type == NVS_TYPE_STR ? nvs_get_str(h, info.key, (char *)buf, &n) : nvs_get_blob(h, info.key, buf, &n)) == ESP_OK;
              // Strings without their terminating zero
              if(ok) hexAppend(value, buf, info.type == NVS_TYPE_STR && n ? n - 1 : n);
            }
            else if(n) ok = false;
            free(buf);
            break;
          }
          default: ok = false; break;
        }
        if(ok)
        {
          out += sections[s]; out += '\t'; out += info.key; out += '\t';
          out += tn; out += '\t'; out += value; out += '\n';
        }
      }
      res = nvs_entry_next(&it);
    }
    nvs_release_iterator(it);
    nvs_close(h);
  }
  return(out);
}

void backupUpload(size_t index, const uint8_t *data, size_t len, bool final)
{
  if(index == 0)
  {
    free(upload);
    upload = (char *)ps_malloc(BACKUP_MAX + 1);
    uploadLen = 0;
    uploadTooBig = false;
    restoreReady = false;
  }
  if(!upload) return;
  if(uploadLen + len > BACKUP_MAX) { uploadTooBig = true; return; }
  memcpy(upload + uploadLen, data, len);
  uploadLen += len;
  if(final) upload[uploadLen] = '\0';
}

static int hexVal(char c)
{
  return(c >= '0' && c <= '9' ? c - '0' : c >= 'a' && c <= 'f' ? c - 'a' + 10 : c >= 'A' && c <= 'F' ? c - 'A' + 10 : -1);
}

// Check the uploaded file without writing anything; nullptr when it is good
const char *backupCheck()
{
  if(!upload || !uploadLen) return("No file received.");
  if(uploadTooBig) return("The file is too large to be a backup.");
  char header[64];
  backupHeader(header, sizeof(header));
  // Compare the format and the three data versions; the firmware may differ
  unsigned fmt, fw, set, mem, bnd;
  if(sscanf(upload, "RESONIK-BACKUP %u %u %u %u %u", &fmt, &fw, &set, &mem, &bnd) != 5 || fmt != 1)
    return("This is not a RESONIK backup file.");
  if(set != VER_SETTINGS || mem != VER_MEMORIES || bnd != VER_BANDS)
    return("The backup was made by a firmware with a different settings layout and cannot be restored here.");
  restoreReady = true;
  return(nullptr);
}

static bool restoreLine(char *line)
{
  char *f[4];
  for(int i = 0; i < 3; i++)
  {
    f[i] = line;
    line = strchr(line, '\t');
    if(!line) return(false);
    *line++ = '\0';
  }
  f[3] = line;
  int s;
  for(s = 0; s < SECTION_COUNT && strcmp(f[0], sections[s]); s++);
  if(s == SECTION_COUNT || strlen(f[1]) > 15) return(false);
  nvs_handle_t h;
  if(nvs_open_from_partition(STORAGE_PARTITION, sections[s], NVS_READWRITE, &h) != ESP_OK) return(false);
  esp_err_t r = ESP_FAIL;
  const char *t = f[2], *v = f[3];
  if(!strcmp(t, "u8"))       r = nvs_set_u8(h, f[1], (uint8_t)strtoul(v, nullptr, 10));
  else if(!strcmp(t, "i8"))  r = nvs_set_i8(h, f[1], (int8_t)strtol(v, nullptr, 10));
  else if(!strcmp(t, "u16")) r = nvs_set_u16(h, f[1], (uint16_t)strtoul(v, nullptr, 10));
  else if(!strcmp(t, "i16")) r = nvs_set_i16(h, f[1], (int16_t)strtol(v, nullptr, 10));
  else if(!strcmp(t, "u32")) r = nvs_set_u32(h, f[1], (uint32_t)strtoul(v, nullptr, 10));
  else if(!strcmp(t, "i32")) r = nvs_set_i32(h, f[1], (int32_t)strtol(v, nullptr, 10));
  else if(!strcmp(t, "u64")) r = nvs_set_u64(h, f[1], (uint64_t)strtoull(v, nullptr, 10));
  else if(!strcmp(t, "i64")) r = nvs_set_i64(h, f[1], (int64_t)strtoll(v, nullptr, 10));
  else if(!strcmp(t, "str") || !strcmp(t, "blob"))
  {
    size_t n = strlen(v) / 2;
    uint8_t *buf = (uint8_t *)ps_malloc(n + 1);
    if(buf)
    {
      bool ok = true;
      for(size_t i = 0; i < n && ok; i++)
      {
        int a = hexVal(v[2 * i]), b = hexVal(v[2 * i + 1]);
        ok = a >= 0 && b >= 0;
        buf[i] = (uint8_t)(a << 4 | b);
      }
      buf[n] = 0;
      if(ok) r = !strcmp(t, "str") ? nvs_set_str(h, f[1], (char *)buf) : nvs_set_blob(h, f[1], buf, n);
      free(buf);
    }
  }
  nvs_commit(h);
  nvs_close(h);
  return(r == ESP_OK);
}

// Main loop: write a checked backup and restart. Nothing runs between the
// writes and the restart, so the values in memory cannot overwrite them.
bool backupTick()
{
  if(!restoreReady) return(false);
  restoreReady = false;
  delay(1500);   // Let the web page receive its answer

  // Replace the sections completely: values missing from the backup go back
  // to their defaults rather than keeping what the radio has now
  for(int s = 0; s < SECTION_COUNT; s++)
  {
    nvs_handle_t h;
    if(nvs_open_from_partition(STORAGE_PARTITION, sections[s], NVS_READWRITE, &h) == ESP_OK)
    {
      nvs_erase_all(h);
      nvs_commit(h);
      nvs_close(h);
    }
  }
  char *line = strchr(upload, '\n');
  while(line)
  {
    char *next = strchr(++line, '\n');
    if(next) *next = '\0';
    char *cr = strchr(line, '\r');
    if(cr) *cr = '\0';
    if(*line) restoreLine(line);
    line = next;
  }
  ESP.restart();
  return(true);
}
