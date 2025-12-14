# Electronic paper driver (Pico RP2040)

Hardware driver and simple graphics canvas, suupported displays:
-  2.13" e-paper display

**Short description:** This repository contains a small graphics canvas and driver glue for driving e-paper (EPD) panel from a Raspberry Pi Pico / RP2040 using the Pico SDK. It provides drawing primitives, text rendering (multiple fonts), and a device-specific initialization sequence.

## Contents
- `main.c` — example/demo app that exercises the canvas and display (select `TEST_CASE` to try different demos).
- `src/canvas` — drawing primitives and font wrappers (`canvas.h`, `canvas.c`, fonts/).
- `src/common/epd.h` — hardware abstraction for the EPD.
- `src/epd_2_13` — device-specific init sequence and pin defaults for the 2.13" panel.

## Quick Start (build & flash)

Requirements:
- Pico SDK installed and configured.
- `cmake` and `ninja` available (the workspace includes VS Code tasks for build/flash).

Example build (powershell):

```powershell
mkdir build
cd build
cmake .. -G Ninja
ninja
```

Flash example using `picotool` (or use provided VS Code tasks):

```powershell
picotool.exe load ..\build\pico_epd_driver.uf2 -fx
```

## Usage
- Edit `main.c` to pick a `TEST_CASE` or to add custom drawing calls using the `canvas` and `epd` APIs.
- Typical flow: `epd_create()` -> `epd_init()` -> draw into `canvas` buffer via `canvas` functions -> `epd_display()` -> `epd_sleep()`.

## License
See `LICENSE` in the repository root.

If you want, I can also add more examples, doxygen generation, or a developer guide.
