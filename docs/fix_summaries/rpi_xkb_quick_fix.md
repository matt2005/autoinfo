# Raspberry Pi build: Qt Quick component not found

- Issue: CMake configure failed with `Failed to find required Qt component "Quick"` and earlier `Could NOT find XKB (missing: XKB_LIBRARY XKB_INCLUDE_DIR)` inside Docker multi-arch RPi builds.
- Root cause: Qt Quick detection depends on XKB development headers; image only had runtime `libxkbcommon0` pulled indirectly, but lacked `libxkbcommon-dev` and `libxkbcommon-x11-dev`.
- Fix: Added `libxkbcommon-dev` and `libxkbcommon-x11-dev` to `Dockerfile.rpi` apt install list.
- Files changed: `Dockerfile.rpi`.
- Expected outcome: CMake now finds XKB, Qt6Quick resolves, RPi matrix builds proceed past configure.
- Notes: Retained existing Qt 6.4.2 Debian packages; if version alignment with CI (6.5.3) becomes necessary, consider building Qt from source or using aqtinstaller in a follow-up.
