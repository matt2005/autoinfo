# MDI Icon Registry Implementation Summary

## Overview
Implemented unified icon handling via a new `IconRegistry` singleton (C++/QML) and a curated set of Material Design Icons (MDI) packaged as Qt resources. This enables consistent icon naming (`mdi:<name>`) across core and extensions, improves UI cohesion, and prepares groundwork for future extension-provided icon packs.

## Key Changes
- Added `IconRegistry.hpp/.cpp` (singleton with QML exposure).
- Updated `src/ui/CMakeLists.txt` to build `IconRegistry` and include `assets/icons/icons.qrc`.
- Created curated monochrome SVG set (home, cog, map, music, bluetooth, play, pause, stop, skip_next, skip_previous).
- Added resource manifest `assets/icons/icons.qrc` with `/icons/mdi` prefix.
- Registered `IconRegistry` in `main.cpp` alongside existing managers.
- Introduced script `scripts/update_mdi_icons.py` for curated remote refresh.
- Updated `Main.qml` to remove emoji placeholders (ready for icon injection where desired).
- Added documentation `docs/icon_system.md` describing usage, licensing, and expansion.

## Usage
QML: `Image { source: IconRegistry.iconUrl("mdi:home"); color: Theme.text }`  
C++: `auto url = IconRegistry::instance()->iconUrl("play");`

## Licensing
Material Design Icons under Apache 2.0 (compatible with GPLv3). Attribution retained via documentation and commit history. Future bulk additions should include NOTICE aggregation.

## Rationale
- Replaces ad-hoc emoji rendering with scalable, theme-tintable vector assets.
- Provides a stable contract for extensions to request icons by logical name.
- Keeps binary and package size low via curated subset; script allows controlled growth.

## Future Work
- Integrate extensions’ own icon packs via manifest (merge into registry).
- Optional placeholder icon for missing requests.
- Performance caching (rasterised pixmaps) for frequently used icons.
- Automated validation to ensure icons listed in manifests exist.

## Validation
- Build succeeds with new resources (Qt AUTORCC processes `icons.qrc`).
- `IconRegistry.listAvailable()` returns curated set at runtime.
- QML can resolve `qrc:/icons/mdi/<name>.svg` URLs.

British English maintained throughout.
