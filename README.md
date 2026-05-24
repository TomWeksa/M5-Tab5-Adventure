# Neon Exclusion

Touch-first adventure game prototype for the M5Stack Tab5.

Neon Exclusion is an original cyberpunk/post-apocalyptic survival adventure about running contaminated districts for salvage, rumors, and impossible artifacts. The character does not level up. Capability comes from equipment, consumables, and dangerous artifacts.

## Project Layout

- `platformio.ini` contains the Tab5 build environment.
- `src/main.cpp` contains the current playable prototype.

## Current Prototype

- Touch UI with field and inventory screens.
- Item-driven stats: GRIT, TECH, SCAN, GHOST, FILTER, and STRAIN are derived from equipped gear.
- Runs through unsafe sites with Scout, Scavenge, and Sneak actions.
- Exposure, body health, scrap, day count, travel, retreat, rest, consumables, and loot.
- Inventory taps equip gear or consume doses.

## Design Direction

- No XP levels or class progression.
- Better abilities come from suits, detectors, tools, weapons, artifacts, drugs, maps, contacts, and vehicle modules.
- Artifacts should be powerful but costly, usually adding STRAIN or other narrative risk.
- The world should feel near-apocalyptic: failing infrastructure, black-market clinics, poisoned rain, corporate remnants, cult signals, and frontier scavengers.

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
