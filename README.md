<img src="docs/resonik-symbol.svg" width="56" alt="RESONIK symbol">

# RESONIK edition of the ATS Mini firmware

[![GitHub downloads](https://img.shields.io/github/downloads/ep3mhb/resonik-ats-mini/total?label=GitHub%20downloads&color=108AB1)](https://github.com/ep3mhb/resonik-ats-mini/releases)
[![Web installs](https://img.shields.io/endpoint?url=https%3A%2F%2Fresonik.mikroutik.com%2Fapi%2Fbadge%3Ftype%3Dweb)](https://resonik.mikroutik.com)
[![Total downloads](https://img.shields.io/endpoint?url=https%3A%2F%2Fresonik.mikroutik.com%2Fapi%2Fbadge%3Ftype%3Dtotal)](https://resonik.mikroutik.com)

An improved firmware for the **ATS Mini** pocket receiver (ESP32-S3 + SI4732): LW, MW, SW and FM, with AM, LSB, USB and CW.
It is built on the open-source [ats-mini](https://github.com/esp32-si4732/ats-mini) firmware and keeps everything it does,
adding features for shortwave listeners and radio amateurs.

> **Latest release: [v2.46-r5](https://github.com/ep3mhb/resonik-ats-mini/releases/latest)** (R5, 24 November 2026).

Website, browser installer and manual: **<https://resonik.mikroutik.com>**

What is coming in each release: [roadmap (PDF)](website/resonik-roadmap.pdf) · [نقشه‌ی راه فارسی (PDF)](website/resonik-roadmap-fa.pdf), source in `docs/roadmap/`.

## What v2.46-r5 adds

- **Digital page** (Menu → Ham & SWL → Digital): RTTY (amateur and DWD weather), NAVTEX on eleven channels, PSK31 and
  WEFAX weather charts, decoded on the radio; the whole text is kept, to read back on the radio or save (needs the V4B board)
- **Decodes tab in Web Control:** live waterfall, FT8/FT4 decodes with country and flag, a map of the stations heard,
  DX alarm, Save log, the decoded text and weather charts, a NAVTEX timetable; start and tune the decoders from the browser
- **Band View** (also in the browser) and **Auto-Store**; **Freq Entry**, a number pad for the frequency
- **Setup wizard** at first start, **backup and restore** of all settings, a **Status page** that grades the radio's health,
  a **new version notice**, and the battery as icon, volts or percent
- **Decoder Audio:** 3 kHz fast or 6 kHz full
- From ats-mini: **CAT TS-480 over USB, Wi-Fi and Bluetooth**, the antenna **Capacitor**, **Enc. Accel.** (knob speed-up),
  Wi-Fi and Bluetooth power in dBm
- The Scanner tile is now **Auto Scan**; every page of the radio's web portal has the same frame, with links to the site,
  the manual and GitHub
- Manuals: the R5 chapters and an appendix, **Learning the digital modes** (English and Persian PDF, web manual)

## Also included (from v2.46-r4, R4.1)

- Brought up to upstream ats-mini **v2.46**: mute and squelch now stay as they were across band and mode changes
- **CW decoder page:** the sound no longer drops when the page opens; it plays at least at the FT8 page's level

## And from v2.45-r4

- **Web Control:** the whole radio in a browser on your phone or computer: frequency, S-meter, RDS, every setting,
  memories, bands and modes, a band scope (click a signal to tune), a live world map of the NCDXF/IARU beacons,
  and Listen (experimental, V4B)
- **Wi-Fi on the radio:** one Wi-Fi page with the IP address, Mode, Power, and Join a network with an on-screen keyboard
- **Seek in SSB and CW**, on the radio and in the browser
- The classic menu has the same six groups as the tile menu; faster band and mode changes; dark radio web pages
- Beacons: Off or Names only; betas update to the release over Wi-Fi

## And from v2.45-r3

- FT8 and FT4 decoder on a full-screen page with waterfall; it decodes even without a clock, finding the slot timing from the stations heard (needs the V4B board, speaker line on GPIO11)
- CW (Morse) decoder page, and a CW strip on the normal screen
- Band scanner (the Auto Scan tile): the knob sets the direction; holds on signals, 5 s per station on AM and FM broadcasts
- Automatic band plan: mode, filter and step follow the segment you tune into; Plan Area All regions, Region 1, 2, 3, Iran or UAE
- Frequency lock, Wi-Fi power (in dBm from v2.46-r5) and processor speed (Battery, Balanced, Fast)
- Fixes: no freezes on slow networks after Wi-Fi connects, seek stays inside the band, smoother SSB tuning

## And from v2.45-r2

- A new default menu: six categories of large coloured tiles (Tuning, Audio, Memory, Ham & SWL, Display, System),
  settings shown full screen; the classic menu stays available
- NCDXF/IARU beacon tracker: names the beacon on the air, or follows one beacon across the five bands
- Memory scan, stopping on signals
- Signal logger to a CSV file, downloadable from the radio's web page
- Propagation data (solar flux, A and K index, band conditions) from hamqsl.com
- Kenwood TS-480 CAT control over USB, and from v2.46-r5 over Wi-Fi (TCP) and Bluetooth (flrig, Hamlib, WSJT-X receive, logging programs)
- EiBi schedules with days of the week, language and target area
- System → Knob: 20 or 30 clicks per turn, for the two kinds of encoder

## And from v2.45-r1

- CW mode: true CW frequency shown, narrow filter, 10 Hz steps
- IARU Region 1 band plan on the tuning scale, with automatic LSB/USB
- Dual clock (UTC and local time) with named time zones and automatic daylight saving
- Battery time remaining
- Automatic attenuation when strong stations overload the receiver
- The clock survives switching off: automatic time sync at power-on, with a web-time fallback for networks that block NTP

Coming next in R6: a propagation map, EiBi stations on the air now, SSTV pictures, the FT2 decoder and more (see the roadmap).

## Which file do I need?

Each release has two variants. Check **Settings → About** on your radio (on RESONIK r2: **Menu → System → About**):

| PSRAM shown | Variant |
|---|---|
| 8192K (8 MB, octal) | `ospi` |
| 2048K (2 MB, quad) | `qspi` |

The `lilygo-t-embed` variant is only for the LILYGO T-Embed SI4732.

## Installing

- **In the browser:** <https://resonik.mikroutik.com>, Install button (Chrome or Edge on a computer).
- **From a RESONIK version:** Menu → System → Update FW (on r1: Settings → Update FW).
- **From the official ats-mini firmware:** by USB, once; after that, Settings → Update FW works.
- **By USB:** download `resonik-ats-mini-v2.46-r5-ospi.zip` (or `-qspi`) from the release, unzip, and flash with
  esptool-js / esptool as described in the upstream [flashing guide](https://esp32-si4732.github.io/ats-mini/flash.html):
  `ats-mini.ino.bootloader.bin` at 0x0, `ats-mini.ino.partitions.bin` at 0x8000, `boot_app0.bin` at 0xe000 and
  `ats-mini.ino.bin` at 0x10000. Your settings are kept.

Make a backup first (see the upstream [backup guide](https://esp32-si4732.github.io/ats-mini/recovery.html)).
You can always go back to the official firmware by USB.

## Versions

Release tags look like `v2.45-r1`: the ats-mini version the edition is based on, then the RESONIK release number.
A new upstream release (for example 2.46) is merged in and released as `v2.46-r1`.

## Building

Same as upstream: `make -C ats-mini` with arduino-cli, profile `esp32s3-ospi` or `esp32s3-qspi`
(see [development](https://esp32-si4732.github.io/ats-mini/development.html)). Unit tests: `make -C ats-mini test`.
GitHub Actions builds every push; running Build Firmware with a version such as `v2.45-r2` tags the branch and publishes the release.

## Credits and licence

This is a fork of [esp32-si4732/ats-mini](https://github.com/esp32-si4732/ats-mini) (MIT licence). The original
authors: PU2CLR (Ricardo Caratti), Volos Projects, Ralph Xavier, Sunnygold, Goshante, G8PTN (Dave), R9UCL (Max Arnold),
Marat Fayzullin, and all ats-mini contributors. The original README is in [README-upstream.md](README-upstream.md).

RESONIK edition by Mohammad Bahrololoum (EP2MHB). Released under the same MIT licence — see [LICENSE](LICENSE).

RESONIK and the RESONIK symbol are the project's name and mark; the licence covers the code, not the name or mark.
