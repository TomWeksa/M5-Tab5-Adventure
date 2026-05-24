# M5 Tab5 Starter

Minimal PlatformIO/Arduino workspace for an M5Stack Tab5.

## Project Layout

- `platformio.ini` contains the Tab5 build environment.
- `src/main.cpp` boots M5Unified, draws a simple screen, and logs touch coordinates over serial.

## Commands

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run --target upload
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" device monitor
```

Shorter `pio` commands work too if PlatformIO is on your `PATH`.

## Notes

- The Tab5 currently builds as `board = esp32-p4-evboard`.
- Build output lives in `.pio/` and is ignored by Git.
