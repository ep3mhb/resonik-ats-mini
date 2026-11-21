#ifndef KENWOOD_EXTRA_H
#define KENWOOD_EXTRA_H

// RESONIK: more of the TS-480 than the upstream CAT (ParserKenwood.cpp,
// RemoteKenwood.cpp) answers, kept from the RESONIK CAT of R1 to R4:
//  - MD3 and MD7 (CW, CW-R) select the CW mode; MD6 and MD9 (FSK, FSK-R)
//    select USB, where the data programs listen;
//  - fixed answers to what flrig, OmniRig and logging programs ask a TS-480
//    for when they start, so that they do not wait for a time-out.
// Pure logic, so it runs in the host tests (tests/TestKenwoodExtra.cpp).

#include <stddef.h>
#include <stdint.h>

enum class KenwoodExtra : uint8_t { None, ModeCw, ModeUsb, Reply };

// frame is one command with its ';'. With Reply, text is what to send back.
inline KenwoodExtra kenwoodExtra(const uint8_t *frame, size_t size, const char *&text)
{
  text = nullptr;
  if(size < 3 || frame[size - 1] != ';') return(KenwoodExtra::None);
  auto upper = [](uint8_t c) { return((char)(c >= 'a' && c <= 'z' ? c - 32 : c)); };
  const char a = upper(frame[0]), b = upper(frame[1]);
  const size_t n = size - 3;   // Characters between the two letters and ';'

  if(a == 'M' && b == 'D' && n == 1)
  {
    if(frame[2] == '3' || frame[2] == '7') return(KenwoodExtra::ModeCw);
    if(frame[2] == '6' || frame[2] == '9') return(KenwoodExtra::ModeUsb);
    return(KenwoodExtra::None);
  }

  if(n) return(KenwoodExtra::None);
  static const struct { char a, b; const char *text; } fixed[] =
  {
    { 'R', 'S', "RS0;" },    // Not in a menu
    { 'K', 'S', "KS020;" },  // Keying speed
    { 'S', 'H', "SH00;" },   // Filter high edge
    { 'S', 'L', "SL00;" },   // Filter low edge
    { 'R', 'T', "RT0;" },    // RIT off
    { 'X', 'T', "XT0;" },    // XIT off
    { 'R', 'F', "RF0;" },
  };
  for(const auto &f : fixed)
    if(f.a == a && f.b == b) { text = f.text; return(KenwoodExtra::Reply); }
  return(KenwoodExtra::None);
}

#endif // KENWOOD_EXTRA_H
