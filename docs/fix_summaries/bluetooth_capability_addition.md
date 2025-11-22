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

# Bluetooth Capability & Dependency Resolution Summary

## Overview
Implemented a new `BluetoothCapability` providing adapter enumeration, device discovery with timeout, pairing and logical connect/disconnect operations. Added mock fallback when BlueZ is not available (e.g., WSL / container) to allow UI development without hardware.

Added capability integration to `CapabilityManager` and refactored the existing Bluetooth extension to use capability-based security instead of direct `EventBus` access. Introduced `BluetoothBridge` to expose commands and events to QML and created a new `BluetoothView.qml` interface.

Implemented robust extension dependency resolution in `ExtensionManager`:
- Loads all manifests first.
- Performs topological sort (Kahn's algorithm).
- Detects and reports cycles.
- Reports missing dependencies (not loaded / not running).
- Requires dependencies to be running, not merely discovered.

## Key Changes
- Added `src/core/capabilities/BluetoothCapability.hpp/.cpp` and factory integration.
- Extended `CapabilityManager` to recognise `bluetooth` permission.
- Refactored `extensions/bluetooth/bluetooth_extension.*` to subscribe/emit via `EventCapability`.
- Added `src/ui/BluetoothBridge.*` and registered singleton in `main.cpp`.
- Created `assets/qml/BluetoothView.qml` with device list, discovery, and call controls.
- Updated `README.md` with Bluetooth dependencies, installation guidance, and dependency system description.
- Updated `docs/extension_development.md` to reflect capability-based API and dependency semantics.
- Added unit tests for dependency resolution (`test_extension_dependencies.cpp`).

## Rationale
- Enforces least privilege and auditability for Bluetooth operations.
- Provides deterministic extension load order and clear diagnostics for dependency issues.
- Enables UI integration without leaking raw internal APIs.

## Follow-Up / Future Work
- Implement adapter selection logic when multiple adapters are present.
- Add profile-specific capabilities (A2DP/HFP) for fine-grained control.
- Enhance Bluetooth UI with connection status feedback and error handling.
- Add automated tests for Bluetooth capability mock vs real mode.
- Persist previously paired devices and auto-connect preferences.

## Testing Notes
Dependency tests pass (`test_extension_dependencies`). Bluetooth runtime requires BlueZ; in mock mode the view still operates but with synthetic adapter and devices list initially empty until discovery triggers.

## Security Considerations
- Capability-based approach prevents arbitrary event emission outside namespace.
- Pairing/connection actions logged through CapabilityManager usage logging.
- Future hardening: permission-based adapter filtering; separate capabilities for sensitive operations (contacts, phone).

## Packages Required (Debian/RPi OS)
```
sudo apt-get install -y bluez bluez-tools libbluetooth-dev qt6-connectivity-dev
```

## Commands Quick Reference
```
# Start discovery (10s)
BluetoothBridge.scan(10000)
# Pair device
BluetoothBridge.pair("AA:BB:CC:DD:EE:FF")
# Connect device
BluetoothBridge.connectDevice("AA:BB:CC:DD:EE:FF")
```

---
Generated on 2025-11-22.
