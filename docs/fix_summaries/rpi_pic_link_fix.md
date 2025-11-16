# Raspberry Pi build: PIC link failure fix

- Summary: RPi arm64 build failed when linking shared extension libraries against static core libraries with "recompile with -fPIC" and "dangerous relocation: unsupported relocation" errors.
- Root cause: Static libraries (`CrankshaftCore`, `CrankshaftExtensions`) were not compiled as position-independent code, but they are linked into shared objects (`libMediaPlayerExtension.so`, `libNavigationExtension.so`). On AArch64, this requires `-fPIC`.
- Change: Enabled CMake global option `CMAKE_POSITION_INDEPENDENT_CODE` in `CMakeLists.txt` so all targets, including static libraries, are compiled with `-fPIC`.
- Files updated: `CMakeLists.txt` (root).
- Expected result: Raspberry Pi matrix builds (arm64/armhf) link successfully for extensions; CI workflow should pass the RPi build steps.
- Notes: This is a safe, portable change; other platforms ignore or benefit from PIC. If any target needs to opt-out explicitly, set `POSITION_INDEPENDENT_CODE OFF` on that target.
