# Dialler → Bluetooth Event Bridge

- Summary: Added a QML-accessible event bridge and wired the Dialler UI to emit public dial events consumed by the Bluetooth extension.
- Date: 2025-11-23

## What Changed
- Added `EventBridge` QML singleton (`CrankshaftReborn.Events/EventBridge`) initialised with the core `EventBus`.
- Updated `DialerView.qml` to publish `dialer.phone.dial` with `{ number }` payload.
- Extended Bluetooth extension to subscribe to public dial events via `"*.phone.dial"` alongside existing `bluetooth.dial`.
- Registered and initialised `EventBridge` in `main.cpp`.
- Updated `src/ui/CMakeLists.txt` to compile the new bridge.

## Rationale
- EventCapability namespacing prefixes emitted events with the extension id. For cross-extension dial requests, a public pattern (`*.phone.dial`) keeps decoupling and mirrors the media events approach.
- A small QML bridge avoids entangling UI with capability internals and keeps emitting trivial events simple.

## Files
- `src/ui/EventBridge.hpp/.cpp`: New QML singleton wrapper for the core event bus.
- `src/ui/CMakeLists.txt`: Added EventBridge sources/headers.
- `src/main.cpp`: Registers and initialises EventBridge.
- `extensions/bluetooth/bluetooth_extension.cpp`: Subscribes to `"*.phone.dial"`.
- `extensions/dialer/qml/DialerView.qml`: Emits `dialer.phone.dial` on Call.

## How to Test
1. Build and run with VNC (as before).
2. Open the Dialler tab, enter a number, press Call.
3. Observe logs in the console: Bluetooth should log `Dial command received: <number>` and publish updated `call_status`.

## Follow-ups
- Consider adding permission checks or per-extension emit helpers to restrict QML publishes where needed.
- Add a small integration test that publishes `dialer.phone.dial` and asserts Bluetooth handles the event.
