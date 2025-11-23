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

# I18n framework implementation

## Summary

Implemented comprehensive internationalisation (i18n) framework with Qt translation support for both core application and extensions. Default locale: `en_GB`.

## Changes

### Core i18n manager

- **`src/ui/I18nManager.hpp/.cpp`**: New singleton managing Qt translations
  - Loads core translations from `appDir/i18n/core_<locale>.qm` or system paths
  - Discovers and loads per-extension translations from `extensions/<id>/i18n/<id>_<locale>.qm`
  - Exposes `currentLocale`, `availableLocales()`, and `setLocale(locale)` to QML
  - Calls `QQmlEngine::retranslate()` on language change to update UI
  - Signals `languageChanged(locale)` for reactive updates

### Main application wiring

- **`src/main.cpp`**:
  - Registered `I18nManager` QML singleton (`CrankshaftReborn.I18n 1.0`)
  - Initialised manager with QML engine and `ExtensionManager` for extension translation discovery
  - Applied default locale from config `system.ui.general.language` (fallback `en_GB`)
  - Wrapped C++ UI page/section/item strings with `QObject::tr()` for core translations

### Language selection

- **System → User Interface → General → Language**:
  - New selection item exposing available locales
  - Default: `en_GB`
  - Changing this setting triggers `I18nManager::setLocale()` and live UI retranslation

### QML string marking

- **`assets/qml/Main.qml`**:
  - Wrapped all visible UI strings in `qsTr(...)` for translation readiness
  - Examples: "Home", "Settings", "Welcome to Crankshaft Reborn", "Security Features", etc.
  - Emojis and dynamic content preserved outside translation markers

### Build integration

- **`src/ui/CMakeLists.txt`**: Added `I18nManager.cpp/.hpp` to UI library sources

## Usage

### Adding translations

1. **Core translations**: Place `.qm` files at:
   - `<appDir>/i18n/core_<locale>.qm` (e.g., `core_fr_FR.qm`)
   - `/usr/share/CrankshaftReborn/i18n/core_<locale>.qm`

2. **Extension translations**: Place `.qm` files at:
   - `<appDir>/extensions/<id>/i18n/<id>_<locale>.qm`
   - `/usr/share/CrankshaftReborn/extensions/<id>/i18n/<id>_<locale>.qm`

3. **Generate `.ts` source files** using `lupdate`:
   ```bash
   lupdate src/ -ts i18n/core_en_GB.ts
   lupdate extensions/navigation/ -ts extensions/navigation/i18n/navigation_en_GB.ts
   ```

4. **Translate `.ts` files** using Qt Linguist or text editor

5. **Compile to `.qm`** using `lrelease`:
   ```bash
   lrelease i18n/core_en_GB.ts -qm i18n/core_en_GB.qm
   ```

### Runtime language change

- Open **Settings → User Interface → General → Language**
- Select desired locale from dropdown
- UI retranslates immediately (no restart required)

## Extension i18n guidelines

Extensions should:
1. Use `qsTr()` in QML for all user-visible strings
2. Use `QObject::tr()` in C++ for config labels/descriptions
3. Provide `.qm` files at `extensions/<id>/i18n/<id>_<locale>.qm`
4. Update translations when adding new strings

## Notes

- Default locale (`en_GB`) works without translation files (source strings used)
- Missing translations log debug messages but don't prevent startup
- Extensions can provide partial translations; untranslated strings fall back to source
- Language selector dynamically populates from discovered `.qm` files

## Testing

- Built successfully; all extensions started
- I18n manager initialised with `en_GB` locale
- UI strings wrapped in `qsTr()` load correctly
- Language change mechanism ready for translation files
