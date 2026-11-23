#include <Arduino.h>
#include <esp_attr.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#if __has_include(<freertos/idf_additions.h>)
#include <freertos/idf_additions.h>
#endif
#include "Diag.h"

// RESONIK R5: see Diag.h
#define DIAG_MAGIC 0x52354447UL   // "R5DG"
#define DIAG_STEPS 32

typedef struct
{
  uint32_t magic;
  uint32_t pos;                   // Steps ever recorded
  uint8_t step[DIAG_STEPS];
  uint32_t ms[DIAG_STEPS];        // Milliseconds since start, for each step
  char task[2][16];               // Running on core 0 and 1 when the task watchdog fired
  uint32_t wdtMs;                 // When it fired, 0 if it did not
} DiagRecord;

static RTC_NOINIT_ATTR DiagRecord rec;
static char lastText[300] = "";

void diagStep(uint8_t code)
{
  if(rec.magic != DIAG_MAGIC) return;
  uint32_t p = rec.pos++ % DIAG_STEPS;
  rec.step[p] = code;
  rec.ms[p] = (uint32_t)(esp_timer_get_time() / 1000);
}

// Called by the task watchdog's interrupt before it restarts the radio: no
// logging, no flash, only copies into RTC memory
extern "C" void IRAM_ATTR esp_task_wdt_isr_user_handler(void)
{
  if(rec.magic != DIAG_MAGIC) return;
  for(int core = 0; core < 2; core++)
  {
    TaskHandle_t h = xTaskGetCurrentTaskHandleForCore(core);
    const char *n = h ? pcTaskGetName(h) : nullptr;
    int i = 0;
    for(; n && n[i] && i < 15; i++) rec.task[core][i] = n[i];
    rec.task[core][i] = '\0';
  }
  rec.wdtMs = (uint32_t)(esp_timer_get_time() / 1000);
}

void diagBoot()
{
  esp_reset_reason_t why = esp_reset_reason();
  bool bad = why == ESP_RST_PANIC || why == ESP_RST_INT_WDT || why == ESP_RST_TASK_WDT || why == ESP_RST_WDT;
  if(bad && rec.magic == DIAG_MAGIC)
  {
    rec.task[0][15] = rec.task[1][15] = '\0';
    size_t len = 0;
    if(rec.wdtMs)
      len = snprintf(lastText, sizeof(lastText), "running: core 0 %s, core 1 %s; ",
                     rec.task[0][0] ? rec.task[0] : "-", rec.task[1][0] ? rec.task[1] : "-");
    uint32_t n = rec.pos < DIAG_STEPS ? rec.pos : DIAG_STEPS;
    len += snprintf(lastText + len, sizeof(lastText) - len, "steps");
    for(uint32_t i = n; i > 0 && len < sizeof(lastText) - 8; i--)
      len += snprintf(lastText + len, sizeof(lastText) - len, " %u", rec.step[(rec.pos - i) % DIAG_STEPS]);
    if(n && rec.wdtMs && len < sizeof(lastText) - 24)
      snprintf(lastText + len, sizeof(lastText) - len, " (last %lu ms before)",
               (unsigned long)(rec.wdtMs - rec.ms[(rec.pos - 1) % DIAG_STEPS]));
  }
  memset(&rec, 0, sizeof(rec));
  rec.magic = DIAG_MAGIC;
}

const char *diagLastText() { return(lastText); }
