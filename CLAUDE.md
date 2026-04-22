# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Firmware (v9.02) for the Waveshare Pico-Green-Clock, a hardware LED matrix clock based on the Raspberry Pi Pico/Pico W. Written in C11/C++17 with ARM assembly, targeting the RP2040 microcontroller.

## Build System

Requires the [Pico SDK](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) to be set up externally. The file `pico_sdk_import.cmake` is **not** in the repo — create a symlink:

```bash
ln -s /home/pi/pico/pico-sdk/external/pico_sdk_import.cmake .
```

### Build for Pico W (NTP support — default)

```bash
mkdir build && cd build
export PICO_SDK_PATH=../../pico-sdk
cmake ..
make -j4
```

Output: `build/Pico-Green-Clock.uf2` — flash by copying to the Pico in BOOTSEL mode.

### Build for original Pico (no NTP)

Replace `CMakeLists.txt` with the contents of `CMakeLists.txt.Pico` before running cmake. The repo keeps three CMake variants: the active `CMakeLists.txt` (currently the Pico W version), `CMakeLists.txt.PicoW` (reference copy), and `CMakeLists.txt.Pico` (original Pico, no Wi-Fi).

There is no test suite and no linter configuration.

## Architecture

### Dual-core design

- **Core 0**: All main clock logic — display rendering, button handling, alarm/chime processing, NTP, flash storage, callback timers.
- **Core 1**: DHT22 temperature/humidity sensor polling (offloaded to avoid timing interference from Core 0 callbacks).
- Inter-core communication uses two custom circular buffers (`memorex.cpp`), not the SDK FIFOs.

### Key files

| File | Purpose |
|------|---------|
| `Pico-Green-Clock.c` | Entire main firmware (~18,900 lines). All clock logic, display, UI, NTP, flash save/restore. |
| `Ds3231.c/h` | DS3231 RTC driver over I2C (i2c1, SDA=GPIO6, SCL=GPIO7, address 0x68). |
| `picow_ntp_client.c/h` | NTP time sync via lwIP on Pico W. |
| `CalendarEventsGeneric.cpp` | User-customizable list of dated messages that scroll on the display. Edit this to personalize events. |
| `RemindersGeneric.cpp` | Reminders feature (partially implemented in v9.02). |
| `define.h` | All GPIO pin assignments, I2C addresses, display buffer macros, and shared structs (`TIME_RTC`, `AlarmMode`). |
| `bitmap.h` | 5×7 pixel font bitmaps for the scrolling text renderer. |
| `lwipopts.h` | lwIP stack configuration for Wi-Fi/NTP. |
| `memorex.cpp` | Circular buffer implementation for inter-core communication, plus IR remote timing/codes. |
| `debug.h` | `DEBUG_*` bitmask flags (per-subsystem UART debug gating, OR-combined into a single debug word). |

**Unusual build pattern:** the `.cpp` files are *not* compiled separately — CMake only builds `Pico-Green-Clock.c`, `picow_ntp_client.c`, and `Ds3231.c`. The `.cpp` files are `#include`d as source into `Pico-Green-Clock.c` at specific lines (calendar at ~line 888, reminders at ~1011, remote at ~4962) via the `CALENDAR_FILENAME` / `REMINDER_FILENAME` / `REMOTE_FILENAME` macros. Users swap in their own variants (e.g. `CalendarEventsAndre.cpp`, a different remote's timing file) by redefining those macros. Developer-only files `Credentials.cpp` (Wi-Fi creds, `#include`d at line 2296) and `test_dst_status.cpp` (line 14255) are referenced but not in the repo — create them locally if needed.

### Display subsystem

The LED matrix is driven by a SM1606SC controller via bit-banged CLK/SDI/LE/OE signals. A `DisplayBuffer[]` byte array holds the current frame. Indicator LEDs (day-of-week, alarm, chime, etc.) are toggled via the macros defined at the bottom of `define.h`. A 5×7 variable-width font supports scrolling arbitrary text strings.

### Persistent storage

Clock configuration is saved to Pico flash (not the DS3231 battery-backed RAM). A CRC16 checksum (polynomial 0x1021) validates the stored struct on read-back. This includes all 9 alarms, display settings, DST zone, timezone offset, and language.

### Sound subsystem

Two separate circular sound queues exist — one for the integrated active buzzer (GPIO14, fixed frequency), one for an optional passive piezo (GPIO19, PWM-driven for variable frequency/jingles). Both are driven from a repeating timer callback.

## Compile-time configuration

All options are `#define`s near the top of `Pico-Green-Clock.c` (~line 280):

| Define | Effect |
|--------|--------|
| `FIRMWARE_VERSION` | Version string baked into the build (currently `"9.02"`). |
| `RELEASE_VERSION` / `DEVELOPER_VERSION` | Mutually exclusive. Release is lean; Developer enables USB CDC debug and requires a terminal connection to start. |
| `PICO_W` | Enable Pico W Wi-Fi and NTP sync. Without it, NTP is automatically disabled. |
| `DST_COUNTRY` | Compile-time DST region (e.g. `DST_NORTH_AMERICA`). Countries are listed in the User Guide PDF. |
| `DEFAULT_LANGUAGE` | `ENGLISH`, `FRENCH`, `GERMAN`, `SPANISH`, `CZECH` |
| `NETWORK_NAME` / `NETWORK_PASSWORD` | Wi-Fi credentials. Flash the firmware once, let it save to flash, then blank out the literals and reflash before committing. |
| `SOUND_DISABLED` | Silence both buzzer queues globally. |
| `QUICK_START` | Skip LED matrix and device tests on power-up |
| `DHT22_SUPPORT` | Enable DHT22 external temp/humidity sensor (GPIO8) |
| `BME280_SUPPORT` | Enable BME280 temp/humidity/pressure sensor (same I2C as DS3231) |
| `IR_SUPPORT` | Enable VS1838B infrared remote control (GPIO9) |
| `PASSIVE_PIEZO_SUPPORT` | Enable passive piezo for jingles/variable tones (GPIO19) |

## Hardware pin map

Defined in `define.h`. Key assignments:

- `SET_BUTTON` = GPIO2, `UP_BUTTON` = GPIO17, `DOWN_BUTTON` = GPIO15
- I2C: SDA = GPIO6, SCL = GPIO7
- LED matrix: CLK=10, SDI=11, LE=12, OE=13
- Active buzzer: GPIO14 | Passive piezo: GPIO19
- ADC light sensor: GPIO26 | ADC Vcc: GPIO29
