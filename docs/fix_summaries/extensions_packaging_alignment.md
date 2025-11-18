# Extensions build and packaging alignment

Date: 2025-11-18

Summary:

- Built base extensions (media_player, navigation, bluetooth) now produce `.so` names matching their manifests (`entry_point`).
- Install layout unified to:
  - Binaries: `lib/CrankshaftReborn/extensions/<ext>/<entry>.so`
  - Manifests and assets: `share/CrankshaftReborn/extensions/<ext>/...`
- Removed duplicate top-level install of manifest JSONs.
- Runtime search paths updated to discover extensions, QML, and themes from both install (`/usr/share/CrankshaftReborn/...`) and portable (`<appdir>/...`) locations.

Files changed:

- `extensions/*/CMakeLists.txt`: set `OUTPUT_NAME` and adjusted `install()` destinations.
- `CMakeLists.txt`: removed duplicate `install(DIRECTORY extensions ...)` for JSONs.
- `src/extensions/extension_manager.cpp`: added search paths (env/appdir/usr/share/current dir).
- `src/main.cpp`: added QML search paths (env/appdir/usr/share/current dir).
- `src/ui/ThemeManager.cpp`: added theme search paths (appdir/usr/share/current dir).

Notes:

- Extension loader currently validates manifests and discovers extensions. Loading the shared objects will be added later after finalising the extension ABI.
- If packaging for Debian, ensure `qt6-*-dev`/runtime packages (Bluetooth, Multimedia, Positioning) are included as needed.
