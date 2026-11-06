<img src="docs/resonik-symbol.svg" width="56" alt="RESONIK symbol">

# RESONIK edition of the ATS Mini firmware

An improved firmware for the **ATS Mini** pocket receiver (ESP32-S3 + SI4732): LW, MW, SW and FM, with AM, LSB, USB and CW.
It is built on the open-source [ats-mini](https://github.com/esp32-si4732/ats-mini) firmware and keeps everything it does,
adding features for shortwave listeners and radio amateurs.

> **Latest release: [v2.45-r1](https://github.com/ep3mhb/resonik-ats-mini/releases/latest)** (6 November 2026).

Website, installer and manuals (coming soon): **<https://resonik.mikroutik.com>**

## What v2.45-r1 adds

- Tile menu (selectable; the classic menu stays available)
- CW mode: true CW frequency shown, narrow filter, 10 Hz steps
- IARU Region 1 band plan on the tuning scale, with automatic LSB/USB
- Dual clock (UTC and local time) with named time zones and automatic daylight saving
- Battery time remaining
- Automatic attenuation when strong stations overload the receiver
- The clock survives switching off: automatic time sync at power-on, with a web-time fallback for networks that block NTP

Later: propagation beacon tracker, better EiBi schedule, memory scanner, signal logger, CAT control,
and on V4 boards an audio spectrum and an on-screen CW decoder.

## Which file do I need?

Each release has two variants. Check **Settings → About** on your radio:

| PSRAM shown | Variant |
|---|---|
| 8192K (8 MB, octal) | `ospi` |
| 2048K (2 MB, quad) | `qspi` |

The `lilygo-t-embed` variant is only for the LILYGO T-Embed SI4732.

## Installing

- **From a RESONIK version:** Settings → Update FW.
- **From the official ats-mini firmware:** by USB, once; after that, Settings → Update FW works.
- **By USB:** download `resonik-ats-mini-v2.45-r1-ospi.zip` (or `-qspi`) from the release, unzip, and flash with
  esptool-js / esptool as described in the upstream [flashing guide](https://esp32-si4732.github.io/ats-mini/flash.html):
  `ats-mini.ino.bootloader.bin` at 0x0, `ats-mini.ino.partitions.bin` at 0x8000, `boot_app0.bin` at 0xe000 and
  `ats-mini.ino.bin` at 0x10000. Your settings are kept. A one-click web installer is coming to <https://resonik.mikroutik.com>.

Make a backup first (see the upstream [backup guide](https://esp32-si4732.github.io/ats-mini/recovery.html)).
You can always go back to the official firmware by USB.

## Versions

Release tags look like `v2.45-r1`: the ats-mini version the edition is based on, then the RESONIK release number.
A new upstream release (for example 2.46) is merged in and released as `v2.46-r1`.

## Building

Same as upstream: `make -C ats-mini` with arduino-cli, profile `esp32s3-ospi` or `esp32s3-qspi`
(see [development](https://esp32-si4732.github.io/ats-mini/development.html)). Unit tests: `make -C ats-mini test`.
GitHub Actions builds every push and publishes releases for `v*` tags.

## Credits and licence

This is a fork of [esp32-si4732/ats-mini](https://github.com/esp32-si4732/ats-mini) (MIT licence). The original
authors: PU2CLR (Ricardo Caratti), Volos Projects, Ralph Xavier, Sunnygold, Goshante, G8PTN (Dave), R9UCL (Max Arnold),
Marat Fayzullin, and all ats-mini contributors. The original README is in [README-upstream.md](README-upstream.md).

RESONIK edition by Mohammad Bahrololoum (EP2MHB). Released under the same MIT licence — see [LICENSE](LICENSE).

RESONIK and the RESONIK symbol are the project's name and mark; the licence covers the code, not the name or mark.
