# Shortcuts Help Overlay and Tab Cycling

Date: 2025-11-23

Summary: Added keyboard shortcuts to cycle tabs left/right and introduced an on-screen “Shortcuts help” overlay that displays the current key mappings. Shortcuts remain configurable via the `system.ui` settings page.

Changes
- Added `assets/qml/ShortcutsHelpOverlay.qml` overlay component (exported via `assets/qml/qmldir`).
- Extended `assets/qml/GlobalShortcutHandler.qml` with new actions:
  - `cycle_left` → cycles to previous tab
  - `cycle_right` → cycles to next tab
  - `show_help` → toggles on-screen shortcuts help overlay
- Wired overlay and actions in `assets/qml/Main.qml`:
  - Displays overlay with current mappings
  - Implements tab cycling with wrap-around
- Registered default keys in `src/main.cpp` under `system.ui.shortcuts`:
  - `open_settings`: `S`
  - `toggle_theme`: `T`
  - `go_home`: `H`
  - `cycle_left`: `A`
  - `cycle_right`: `D`
  - `show_help`: `?`

Notes
- The help overlay closes when pressing `Esc` or clicking outside the panel.
- Keys are treated as simple single-character bindings (case-insensitive). Modifier-aware bindings can be considered later.
- Bluetooth/Media Player still log unknown capability requests that are currently not implemented; non-fatal.

Verification
- Built and launched under VNC; QML loaded without errors and overlay component initialised. Manual key testing recommended:
  - Press `?` to toggle the help overlay
  - Press `A` / `D` to cycle tabs left / right
  - Press `S`, `T`, `H` for settings/theme/home actions respectively
