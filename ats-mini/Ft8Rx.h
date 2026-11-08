#ifndef FT8_RX_H
#define FT8_RX_H

// RESONIK: FT8 receiver core. Takes raw audio samples (12-bit ADC values,
// any DC offset) at 12 kHz, builds the FT8 waterfall with ft8_lib's monitor
// and decodes a 15 second slot. No Arduino dependencies, so it is tested on
// the host (tests/TestFt8.cpp). The ESP32 side is in Ft8.cpp.
//
// ft8_lib (Karlis Goba, MIT) and KISS FFT (Mark Borgerding, BSD-3) are in
// src/ft8, as adapted for the ESP32-S3 by the epiHATR ats-mini fork.

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "src/ft8/monitor.h"
#include "src/ft8/decode.h"
#include "src/ft8/message.h"
#include "src/ft8/encode.h"
#include "src/ft8/constants.h"

class Ft8Rx
{
public:
  struct Msg
  {
    char text[36];       // Decoded message, for instance "CQ EP2MHB LL58"
    int8_t snr;          // dB in 2500 Hz, as WSJT-X shows it (estimated)
    uint16_t freq;       // Audio frequency of the lowest tone, Hz
  };

  static const int FS = 12000;
  // Half-symbol time and half-bin frequency steps: real signals start and
  // sit anywhere, and a whole-step grid loses several dB on them
  static const int TIME_OSR = 2, FREQ_OSR = 2;
  static const int F_MIN = 200, F_MAX = 2900;

  bool begin()
  {
    if(ready) return(true);
    monitor_config_t cfg = {};
    cfg.f_min = F_MIN;
    cfg.f_max = F_MAX;
    cfg.sample_rate = FS;
    cfg.time_osr = TIME_OSR;
    cfg.freq_osr = FREQ_OSR;
    cfg.protocol = FTX_PROTOCOL_FT8;
    monitor_init(&mon, &cfg);
    frame = new float[mon.block_size];
    if(!mon.wf.mag || !mon.window || !mon.fft_cfg || !frame)
    {
      end();
      return(false);
    }
    ready = true;
    reset();
    return(true);
  }

  void end()
  {
    monitor_free(&mon);
    memset(&mon, 0, sizeof(mon));
    delete[] frame;
    frame = nullptr;
    ready = false;
  }

  // Start a new slot
  void reset()
  {
    if(!ready) return;
    monitor_reset(&mon);
    fill = 0;
  }

  // Feed samples; returns true if a waterfall row was added
  bool push(const int16_t *s, int count)
  {
    if(!ready) return(false);
    bool row = false;
    for(int i = 0 ; i < count ; i++)
    {
      // Slow DC tracker so the raw ADC value can be fed in directly
      if(!dcSet) { dc = s[i]; dcSet = true; }
      dc += ((float)s[i] - dc) * 0.002f;
      float x = ((float)s[i] - dc) * (1.0f / 2048.0f);
      // Input level, mean absolute deviation in ADC counts
      dev += (fabsf((float)s[i] - dc) - dev) * 0.0005f;
      frame[fill++] = x;
      if(fill >= mon.block_size)
      {
        fill = 0;
        if(mon.wf.num_blocks < mon.wf.max_blocks)
        {
          monitor_process(&mon, frame);
          row = true;
        }
      }
    }
    return(row);
  }

  bool isReady() const { return(ready); }
  int blocks() const { return(ready ? mon.wf.num_blocks : 0); }
  int maxBlocks() const { return(ready ? mon.wf.max_blocks : 0); }
  int bins() const { return(ready ? mon.wf.num_bins : 0); }
  int level() const { return((int)(dev + 0.5f)); }

  // Newest waterfall row: bins() values, 0..255 = -120..+7.5 dB
  const uint8_t *lastRow() const
  {
    if(!ready || !mon.wf.num_blocks) return(nullptr);
    return(mon.wf.mag + (mon.wf.num_blocks - 1) * mon.wf.block_stride);
  }

  // Decode the slot collected so far; returns the number of messages
  int decode(Msg *out, int maxOut)
  {
    if(!ready || mon.wf.num_blocks < FT8_NN / 2) return(0);

    static ftx_candidate_t cand[MAX_CANDIDATES];
    int num = ftx_find_candidates(&mon.wf, MAX_CANDIDATES, cand, MIN_SCORE);
    int noise = noiseLevel();
    hashReset();

    int n = 0;
    for(int i = 0 ; i < num && n < maxOut ; i++)
    {
      ftx_message_t msg;
      ftx_decode_status_t status;
      if(!ftx_decode_candidate(&mon.wf, &cand[i], LDPC_ITERATIONS, &msg, &status)) continue;
      if(status.crc_calculated != status.crc_extracted) continue;

      char text[64];
      ftx_message_offsets_t offsets;
      if(ftx_message_decode(&msg, &hashIf, text, &offsets) != FTX_MESSAGE_RC_OK) continue;

      // The same message is found at nearby offsets; keep the first
      bool dup = false;
      for(int j = 0 ; j < n && !dup ; j++) dup = !strcmp(out[j].text, text);
      if(dup) continue;

      Msg &m = out[n++];
      snprintf(m.text, sizeof(m.text), "%.35s", text);
      float bin = mon.min_bin + cand[i].freq_offset + (float)cand[i].freq_sub / mon.wf.freq_osr;
      m.freq = (uint16_t)(bin / mon.symbol_period + 0.5f);
      m.snr = snr(&msg, &cand[i], noise);
    }
    return(n);
  }

private:
  static const int MAX_CANDIDATES = 120;
  static const int MIN_SCORE = 10;
  static const int LDPC_ITERATIONS = 25;

  monitor_t mon = {};
  bool ready = false;
  float *frame = nullptr;
  int fill = 0;
  float dc = 0, dev = 0;
  bool dcSet = false;

  // Median of the waterfall, in its 0.5 dB units: the noise in one 6.25 Hz bin
  int noiseLevel() const
  {
    static uint32_t hist[256];
    memset(hist, 0, sizeof(hist));
    int total = mon.wf.num_blocks * mon.wf.block_stride;
    for(int i = 0 ; i < total ; i++) hist[mon.wf.mag[i]]++;
    uint32_t acc = 0;
    for(int v = 0 ; v < 256 ; v++) if((acc += hist[v]) * 2 >= (uint32_t)total) return(v);
    return(0);
  }

  // Signal level on the decoded tones against the noise per bin, scaled to
  // the 2500 Hz bandwidth WSJT-X uses: 10 log10(2500 / 6.25) = 26 dB
  int8_t snr(const ftx_message_t *msg, const ftx_candidate_t *c, int noise) const
  {
    uint8_t tones[FT8_NN];
    ft8_encode(msg->payload, tones);
    int sum = 0, count = 0;
    for(int s = 0 ; s < FT8_NN ; s++)
    {
      int block = c->time_offset + s;
      int bin = c->freq_offset + tones[s];
      if(block < 0 || block >= mon.wf.num_blocks || bin < 0 || bin >= mon.wf.num_bins) continue;
      int sub = (c->time_sub * mon.wf.freq_osr + c->freq_sub) * mon.wf.num_bins;
      sum += mon.wf.mag[block * mon.wf.block_stride + sub + bin];
      count++;
    }
    if(!count) return(-30);
    // The median of noise power in dB sits about 1.6 dB under its mean
    float db = ((float)sum / count - noise) / 2.0f - 1.6f;
    // The tone bin holds signal plus noise
    float ratio = powf(10.0f, db / 10.0f) - 1.0f;
    float s = ratio > 0.001f ? 10.0f * log10f(ratio) - 26.0f : -30.0f;
    if(s < -30) s = -30;
    if(s > 40) s = 40;
    return((int8_t)lroundf(s));
  }

  // Callsign hash table for messages that carry a hashed callsign
  struct HashEntry { char call[12]; uint32_t hash; };
  static const int HASH_SIZE = 64;
  static HashEntry *hashTable() { static HashEntry t[HASH_SIZE]; return(t); }
  static void hashReset() { memset(hashTable(), 0, sizeof(HashEntry) * HASH_SIZE); }

  static void hashSave(const char *call, uint32_t hash)
  {
    HashEntry *t = hashTable();
    int idx = (((hash >> 12) & 0x3FF) * 23) % HASH_SIZE;
    for(int k = 0 ; k < HASH_SIZE ; k++, idx = (idx + 1) % HASH_SIZE)
    {
      if(!t[idx].call[0] || ((t[idx].hash & 0x3FFFFF) == (hash & 0x3FFFFF) && !strcmp(t[idx].call, call)))
      {
        snprintf(t[idx].call, sizeof(t[idx].call), "%s", call);
        t[idx].hash = hash;
        return;
      }
    }
  }

  static bool hashLookup(ftx_callsign_hash_type_t type, uint32_t hash, char *call)
  {
    HashEntry *t = hashTable();
    uint8_t shift = type == FTX_CALLSIGN_HASH_10_BITS ? 12 : type == FTX_CALLSIGN_HASH_12_BITS ? 10 : 0;
    int idx = (((hash >> (12 - shift)) & 0x3FF) * 23) % HASH_SIZE;
    for(int k = 0 ; k < HASH_SIZE ; k++, idx = (idx + 1) % HASH_SIZE)
    {
      if(!t[idx].call[0]) return(false);
      if(((t[idx].hash & 0x3FFFFF) >> shift) == hash) { strcpy(call, t[idx].call); return(true); }
    }
    return(false);
  }

  ftx_callsign_hash_interface_t hashIf = { hashLookup, hashSave };
};

#endif // FT8_RX_H
