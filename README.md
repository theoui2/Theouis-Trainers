# Theoui's Trainers 1.0

Central trainer launcher for Nintendo Switch, by **Theoui**.

Open one entry in Ultrahand, then choose an installed game trainer. The launcher discovers `.ovl` files in `/switch/TheouisTrainers/trainers/`; adding a game does not require a launcher update.

## Contextual shortcut

Open **Theoui's Trainers → Shortcut settings** once, choose a preset, then **Return to Ultrahand**. **L3 + R3** is the default first-setup choice when available. The shortcut opens the trainer associated with the running game; active overlays use it to hide/show. Six button presets and Off are available. Occupied combinations are refused, and existing global shortcuts are preserved.

See [SHORTCUTS.md](SHORTCUTS.md) / [Raccourcis en français](RACCOURCIS_FR.md) for setup, game associations and supported overlay contexts.

## Installation

Atmosphere, a working overlay loader and Ultrahand must already be installed.

1. Download **TheouisTrainers_Loader_v1.0.zip** from this repository's Releases.
2. Copy **the contents of SD/** to the microSD root. The launcher is installed at `/switch/.overlays/Trainers.ovl`.
3. Install the trainers you want from their separate repositories.
4. Open **Theoui's Trainers** in Ultrahand and select a game. Use **Refresh trainers** after adding files while the menu is open.

| Trainer | Repository | Supported game version |
|---|---|---|
| Borderlands 2 | [Simple Trainer BL2 — Switch](https://github.com/theoui2/Simple-Trainer-BL2-Switch) | 1.0.2 |
| Tears of the Kingdom | [Simple Trainer TOTK](https://github.com/theoui2/Simple-Trainer-TOTK-) | 1.4.2 |

This release contains the launcher only. Each game has its own release and update schedule. [Instructions en français](INSTALLATION_FR.md).

## Add a trainer

Place its compiled `.ovl` directly in `/switch/TheouisTrainers/trainers/`. Files in this directory are listed by this launcher, outside Ultrahand's overlay list. Preserve the trainer's filename when updating it.

The display name and version are read from the overlay metadata. An optional `.ini` with the same basename overrides its display name:

```ini
[trainer]
name=Borderlands 2
```

For example: `SimpleTrainerSwitch.ovl` and `SimpleTrainerSwitch.ini`. The version still comes from the compiled overlay. Third-party trainers may require their own additional files.

## Existing installations

The first launch recognizes the old BL2, TOTK and TOTK Lab filenames. It moves an old trainer when the destination is missing, preserves an existing destination, and hides retained legacy root copies and the old Trainers package. Unrelated Ultrahand entries and settings are preserved. Migration errors appear in the menu.

## Build and validation

Set `DEVKITPRO` and add devkitA64 and Switch tools to `PATH`, then run `make -j2`. The reference toolchain is GCC 16.1.0 and libnx 4.12.0. Output: `Trainers.ovl`.

Run `python3 tools/package_release.py` after building to create the launcher-only installation ZIP in `dist/`, with corresponding sources and checksums.

The source includes 28 catalogue/migration scenarios and 9 registry-preservation scenarios. See [VALIDATION.md](VALIDATION.md). Physical-console launch and navigation still require validation.

## Credits

GPL-2.0-or-later; see LICENSE. Includes WerWolv/libtesla and libnx notices. The launcher uses the standard next-overlay mechanism and has no game-memory patching code.
