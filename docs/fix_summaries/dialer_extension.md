# Dialler Extension (UI)

Date: 2025-11-23

Summary

- Implemented a new `dialer` UI extension that provides a simple dialler keypad and number entry. It registers a main view tab via `UICapability`.

Details

- Location: `extensions/dialer/`
- Build: Added subdirectory to `extensions/CMakeLists.txt`; outputs `dialer.so` with manifest copy and QML deploy to `build/extensions/dialer`.
- UI: QML view at `qrc:/dialer/qml/DialerView.qml` with keypad, backspace, call, and clear actions.
- Capabilities: Requests `ui`, `event`, `phone`, and `contacts` in manifest; uses `UICapability` today and prepares for event integration.
- Integration plan: The call button currently logs to console. In a subsequent change, we will bridge from QML to `EventCapability` to emit `bluetooth.dial` with the entered number, enabling the Bluetooth extension to place calls.

Verification

- Built and launched under VNC; extension discovery now reports 4 extensions. Dialler view registers as a main component and appears as a tab.

Next steps

- Add a minimal QML-to-Event bridge to publish `bluetooth.dial` events from the dialler UI.
- Optional: recent calls list (persist via `ConfigManager`), contact lookup using `contacts` capability once implemented.
