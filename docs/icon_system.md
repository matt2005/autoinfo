# Icon System & Registry

The Crankshaft icon system provides a unified way to reference and load icons across the core application and extensions using a simple logical name (`mdi:<icon>`). A curated subset of Material Design Icons (MDI) is bundled for common automotive / media use cases.

## Goals
- Consistent icon naming (stable contract for extensions)
- Theme-friendly monochrome SVGs (colour applied in QML via `color`)
- Low footprint (avoid bundling thousands of unused icons)
- Easy future expansion via update script

## Registry
The `IconRegistry` singleton is exposed to QML as `IconRegistry` and offers:
- `iconUrl(name: string): url` – returns a `qrc:/` URL for the icon or empty if missing
- `exists(name: string): bool` – checks availability
- `listAvailable(): string[]` – enumerates bundled icon names

Names may be provided as:
- `mdi:home`
- `home`
- `home.svg`

All normalised to a canonical form internally.

## QML Usage
```qml
Image {
    source: IconRegistry.iconUrl("mdi:home")
    width: 24; height: 24
    fillMode: Image.PreserveAspectFit
    color: Theme.text // monochrome tinting
}
```

Buttons:
```qml
Button {
    contentItem: Row {
        spacing: 6
        Image { source: IconRegistry.iconUrl("cog"); width: 18; height: 18; color: Theme.accent }
        Text { text: qsTr("Settings") }
    }
}
```

## C++ Usage
```cpp
#include "ui/IconRegistry.hpp"
QUrl playUrl = opencardev::crankshaft::ui::IconRegistry::instance()->iconUrl("play");
```

## Adding Icons
1. Append icon names to `ICONS` list in `scripts/update_mdi_icons.py`.
2. Run: `python scripts/update_mdi_icons.py` (WSL or host Python 3).
3. Ensure new SVGs appear under `assets/icons/mdi/`.
4. Add them to `assets/icons/icons.qrc` if not automatically matched.
5. Rebuild (`cmake --build build`).

## Theming
Icons are monochrome and inherit colour from the QML `Image.color` property, enabling dynamic light/dark mode adaptation without multiple assets.

## Licensing
Bundled icons derive from Material Design Icons (Apache License 2.0). This is compatible with GPLv3 distribution. When expanding set ensure Apache 2.0 header or attribution is retained in commit messages or a central NOTICE section if large sets are added.

## Fallback Strategy
If `iconUrl()` returns an empty URL:
- Provide a placeholder glyph or omit image element.
- Extensions should degrade gracefully.

## Internationalisation
Icon names are **not** translated. Visible labels accompanying icons must use `qsTr()`.

## Future Enhancements
- Automatic dark/light adaptive multi-tone variants.
- Extension-provided icon packs (manifest-declared, merged at runtime).
- Caching of rasterised icons for performance on low-power GPUs.

---
British English terminology maintained throughout.
