<!--
Project: Crankshaft
This file is part of Crankshaft project.
Copyright (C) 2025 OpenCarDev Team

Crankshaft is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 3 of the License, or
(at your option) any later version.

Crankshaft is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
-->

# Extension enable/disable toggles

## Summary

- Added config-driven enable/disable controls for built-in extensions and runtime toggling hook.

## Changes

- `src/extensions/extension_manager.hpp/.cpp`:
  - Added `enableExtension` and `disableExtension` methods to start/stop extension instances without unloading metadata.
  - During built-in registration, respect `system.extensions.manage.<id>` to decide whether to start the extension.
  - Connected to `ConfigManager::configValueChanged` to hot-toggle extensions when `system.extensions.manage` values change.
- `src/main.cpp`:
  - Registered a new config page `system.extensions` with a `manage` section exposing Boolean toggles for core extensions (navigation, bluetooth, media_player, dialer, wireless).
  - Updated `system.ui` page icon to text to avoid missing qrc icon.

## Notes

- Current UI tabs remain after disabling because component IDs are not tracked for unregistration in `UICapability`. A follow-up task can extend `UICapability` to return component IDs and add bulk-unregister in `ExtensionRegistry` for full hot UI removal.

## Testing

- Start the app, open Settings → Extensions, toggle e.g. Wireless off/on.
- Observe logs for "Disabled extension:" / "Enabled extension:" messages.
- On restart, disabled extensions do not start.
