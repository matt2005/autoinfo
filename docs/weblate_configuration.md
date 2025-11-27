# Weblate Configuration (Example)

This document outlines an example configuration for integrating Weblate with the Crankshaft project. It maps each translation component (core and extensions) to a Weblate component for collaborative localisation.

## Repository

- Type: GitHub
- URL: `https://github.com/matt2005/autoinfo.git`
- Branch: `main`
- Push method: Weblate commits directly to feature branches or via pull requests (recommended: PR model)

## Component Mapping

| Component Name        | Directory / File Pattern                              | File Format | Base Locale | Purpose                             |
|-----------------------|--------------------------------------------------------|-------------|-------------|-------------------------------------|
| Core UI               | `i18n/core_*.ts`                                       | Qt TS       | en_GB       | Core application interface strings. |
| Bluetooth Extension   | `extensions/bluetooth/i18n/bluetooth_*.ts`             | Qt TS       | en_GB       | Bluetooth related UI strings.       |
| Dialler Extension     | `extensions/dialer/i18n/dialer_*.ts`                   | Qt TS       | en_GB       | Dialler views and labels.           |
| Media Player Extension| `extensions/media_player/i18n/media_player_*.ts`       | Qt TS       | en_GB       | Media player controls & metadata.   |
| Navigation Extension  | `extensions/navigation/i18n/navigation_*.ts`           | Qt TS       | en_GB       | Navigation UI & status messages.    |
| Wireless Extension    | `extensions/wireless/i18n/wireless_*.ts`               | Qt TS       | en_GB       | Wireless networks UI.               |

## Suggested Weblate Project Settings

- Translation license: Same as project (GPLv3)
- Language code standard: Use BCP-47 (e.g., `en-GB`, `fr`, `de`)
- Add new empty strings automatically: Enabled
- Remove obsolete strings: Enabled (Qt TS supports `-no-obsolete` during extraction)
- Notifications: Enable for component updates and merge conflicts
- Quality Checks: Placeholders consistency (`%1`, `%n`), untranslated strings, duplicate translations

## Automation Flow

1. Developer adds/changes strings in QML/C++ with `qsTr()` / `tr()`.
2. CI or local script runs `lupdate` (via `scripts/update_translations.sh extract`).
3. Updated `.ts` files are pushed to the repository.
4. Weblate syncs changes and translators work via Web UI.
5. Translators submit translations; Weblate pushes commits back.
6. CI compiles `.qm` via `scripts/update_translations.sh compile` and bundles in releases.

## Weblate Component Example (projects.yaml fragment)

```yaml
projects:
  - name: Crankshaft
    slug: crankshaft
    repo: 'https://github.com/matt2005/autoinfo.git'
    branch: main
    components:
      - name: Core UI
        slug: core-ui
        filemask: 'i18n/core_*.ts'
        template: 'i18n/core_en_GB.ts'
        new_base: 'i18n/core_en_GB.ts'
        language_regex: '.*'
      - name: Bluetooth Extension
        slug: bluetooth
        filemask: 'extensions/bluetooth/i18n/bluetooth_*.ts'
        template: 'extensions/bluetooth/i18n/bluetooth_en_GB.ts'
      - name: Dialler Extension
        slug: dialer
        filemask: 'extensions/dialer/i18n/dialer_*.ts'
        template: 'extensions/dialer/i18n/dialer_en_GB.ts'
      - name: Media Player Extension
        slug: media-player
        filemask: 'extensions/media_player/i18n/media_player_*.ts'
        template: 'extensions/media_player/i18n/media_player_en_GB.ts'
      - name: Navigation Extension
        slug: navigation
        filemask: 'extensions/navigation/i18n/navigation_*.ts'
        template: 'extensions/navigation/i18n/navigation_en_GB.ts'
      - name: Wireless Extension
        slug: wireless
        filemask: 'extensions/wireless/i18n/wireless_*.ts'
        template: 'extensions/wireless/i18n/wireless_en_GB.ts'
```

## Translator Guidelines

- Preserve placeholders: `%1`, `%2`, `%n`.
- Keep punctuation and capitalisation consistent with UI style.
- Avoid length inflation that risks truncation in constrained layouts.
- Prefer sentence case for UI labels (unless context requires otherwise).
- Use British English (`en_GB`) as canonical source language.

## Next Steps

- Run `./scripts/update_translations.sh all` to generate per-extension `.ts` stubs.
- Commit new `.ts` files and enable Weblate project sync.
- Add `compile_translations` target artefacts to release packaging.

This configuration is an example and can be adapted as the number of extensions grows.
