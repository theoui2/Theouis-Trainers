# Contextual trainer shortcut

## First setup

Install the updated launcher and game trainers. Open **Theoui's Trainers** once from Ultrahand, then **Shortcut settings**. The first setup selects **L3 + R3** when it is available. Choose a different preset or **Off**, then use **Return to Ultrahand** to reload Ultrahand's shortcut map. No console reboot is required for this map refresh.

| Preset | Buttons |
|---|---|
| Off | Disable the trainer shortcut |
| L3 + R3 | Press both sticks |
| L + R + L3 | L, R and left-stick click |
| L + R + R3 | L, R and right-stick click |
| ZL + ZR + L3 | ZL, ZR and left-stick click |
| ZL + ZR + R3 | ZL, ZR and right-stick click |
| L + R + D-Up | L, R and D-pad Up |

When the overlay is hidden, the shortcut identifies the running application. BL2 opens its BL2 trainer; TOTK opens its TOTK trainer. Reopening the current trainer retains its current menu and session state. Press the shortcut with our overlay already visible to hide it. Release the chord before pressing it again.

If no game/trainer matches, the launcher shows its normal list with an explanation. Duplicate game associations or an incomplete catalogue are refused for automatic routing; manual selection remains available. Existing game/build/player checks still decide whether gameplay options can be used.

## Settings and other overlays

The preset is stored in `/config/theouis-trainers/config.ini`. Installation ZIPs do not replace this file. Ultrahand receives one launcher mode (`--theoui-auto`) in the `Trainers.ovl` section of `overlays.ini`; manual opening still shows the catalogue. The global Ultrahand/Tesla shortcut, other overlays, favourites, priorities and unrelated launch settings are retained.

An occupied combination is refused with a reason. The previous preset remains selected. If Ultrahand's **Launch Combos** setting is disabled, enable it in Ultrahand before selecting an active preset. Off is always available.

This integration listens in Ultrahand and in these updated Theoui overlays. Other overlays need their own compatible Ultrahand launch-combo support to forward the shortcut while they are loaded. Return to Ultrahand when using an older third-party overlay that does not provide it.

## Add another game

Use a same-basename `.ini` beside the game's `.ovl` in `/switch/TheouisTrainers/trainers/`:

```ini
[trainer]
name=Borderlands 2
title_id=010096F00FF22000
```

TOTK uses `title_id=0100F2C0115B6000`. A future game can declare its own 16-digit hexadecimal application ID without a launcher rebuild. The displayed version still comes from the overlay metadata. A matching ID associates a file with a game; the trainer itself remains responsible for version compatibility.

## Technical scope

Input polling compares a cached key mask; it performs no SD or game-memory scan each frame. Application IDs are queried via the process-manager services only when routing is requested, with a second PID check. Auto-launch rechecks the process before switching. The trainer gesture is handled on the overlay UI thread, and a held chord cannot recursively launch overlays.

The Ultrahand mode format was checked against its official source and libultrahand commit `2e4df5430045217093dd77751cd9e16c92fe9144`:
- https://github.com/ppkantorski/libultrahand/blob/2e4df5430045217093dd77751cd9e16c92fe9144/libtesla/source/tesla.cpp
- https://github.com/ppkantorski/Ultrahand-Overlay

Reference integration: Ultrahand 2.5.3 with launch combos enabled. Host tests and Switch compilation do not replace physical-console validation of the initial launch, reopening, game changes and input behaviour.
