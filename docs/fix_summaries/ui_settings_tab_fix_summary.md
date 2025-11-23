[]: # ```plaintext
[]: #  * Project: Crankshaft
[]: #  * This file is part of Crankshaft project.
[]: #  * Copyright (C) 2025 OpenCarDev Team
[]: #  *
[]: #  *  Crankshaft is free software: you can redistribute it and/or modify
[]: #  *  it under the terms of the GNU General Public License as published by
[]: #  *  the Free Software Foundation; either version 3 of the License, or
[]: #  *  (at your option) any later version.
[]: #  *
[]: #  *  Crankshaft is distributed in the hope that it will be useful,
[]: #  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
[]: #  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
[]: #  *  GNU General Public License for more details.
[]: #  *
[]: #  *  You should have received a copy of the GNU General Public License
[]: #  *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
[]: # ```

# UI Settings Tab + Event Bus Wildcards: Fix Summary

- Date: 2025-11-23
- Author: OpenCarDev Team

## Summary
- Exposed the Settings (Configuration) screen via a proper tab in the header `TabBar`.
- Implemented basic glob-style wildcard delivery in the core `EventBus` to support public media control events (e.g. `*.media.play`).
- Fixed and verified the new integration test for public media controls.

## Root Causes
- The `Settings` tab UI existed but its `TabButton` was mistakenly placed inside the Home page content rather than inside the header `TabBar`, so it could not be selected.
- The `EventBus` only matched exact event names; wildcard subscriptions (like `*.media.play`) were not delivered.

## Changes
- QML: `assets/qml/Main.qml`
  - Added a `TabButton` for Settings inside the header `TabBar` after dynamic extension tabs.
  - Removed the misplaced `TabButton` from the Home page.
- Core: `src/core/events/event_bus.cpp`
  - Added simple glob wildcard matching on publish: supports `*` and `?` in subscription patterns.
- Tests: `tests/integration/test_media_public_controls.cpp`
  - Removed unused `WebSocketServer` stub and included `QElapsedTimer`.

## Validation
- Rebuilt and ran the focused integration test:
  - `ctest -R test_media_public_controls --output-on-failure`
  - Result: 100% passed for the targeted test.
- Manual run under VNC platform (headless) to inspect logs:
  - `QT_DEBUG_PLUGINS=1 QT_QPA_PLATFORM=vnc QT_QPA_VNC_SIZE=1280x800 ./build/CrankshaftReborn`
  - Observed QML load and extension view loading messages.
  - The Settings tab is now part of the header `TabBar` and appears after extension tabs.

## Notes
- Wildcard matching is implemented at publish time using a regex derived from the glob pattern. Exact matches are delivered first; wildcard patterns are delivered subsequently.
- The `ConfigScreen` is the final page in the `StackLayout`. Its index is `1 + ExtensionRegistry.mainComponents.length`, and is now addressable via the new Settings tab.

## Next Steps
- Add a small smoke test for `EventBus` wildcard delivery in unit tests.
- Consider persisting the last-selected tab across sessions.
