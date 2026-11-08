# Third-party FT8 / FFT sources

Used by the RESONIK FT8 page (`Ft8Rx.h`, `Ft8.cpp`). Taken from the
[epiHATR ats-mini fork](https://github.com/epiHATR/ats-mini), which adapted
them for the ESP32-S3 (heap buffers in `monitor.c`, `ft8_common.h` in place
of `common.h`).

* [kgoba/ft8_lib](https://github.com/kgoba/ft8_lib): MIT, see `FT8_LICENSE.txt`
* [mborgerding/kissfft](https://github.com/mborgerding/kissfft): BSD-3-Clause,
  see `KISSFFT_LICENSE.txt`
