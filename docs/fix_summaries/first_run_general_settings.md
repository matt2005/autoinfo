# First Run General Settings Implementation

## Summary
Implemented a first–run experience that automatically opens the General settings page on the initial application launch. This encourages users to configure language and UI preferences immediately. The behaviour is governed by a new configuration flag `enablefirstrun` which is turned off after the first display.

## Changes
- Added config item `enablefirstrun` (boolean, default `true`) under domain `system`, extension `ui`, section `general` in `src/main.cpp`.
- Added programmatic navigation helper `openPage(domain, extension)` to `ConfigScreen.qml`.
- Added first–run logic to `Main.qml` (`Component.onCompleted`) to:
  - Detect `system.ui.general.enablefirstrun`.
  - Navigate to the Settings tab and open the `system/ui` General page.
  - Set the flag to `false` to prevent repeat navigation.
- Improved extension tab icon logic in `Main.qml` to avoid loading non–path icon strings as images (prevents spurious QQuickImage errors).
- Added a VS Code task `Build Translations` to build `.qm` files easily.
- Updated translation build process to copy `.qm` files into the build tree for runtime usage (previous change referenced).

## Rationale
Providing an immediate settings page on first run streamlines initial set–up (language, theme, shortcuts). Using a simple boolean avoids complexity while remaining flexible. Programmatic page navigation required a small API addition to `ConfigScreen.qml`.

## Verification Steps
1. Clean build or remove existing config entry to simulate first run.
2. Launch application.
3. Confirm it opens on the Settings tab (General section).
4. Inspect logs for message: `First run detected: opening General settings page`.
5. Confirm subsequent launches open on Home (flag now false).

## Future Considerations
- Persist user profile driven onboarding (multiple steps) if needed later.
- Extend `enablefirstrun` to trigger a guided tour overlay.
- Add localisation entries once translations for additional languages exist.

## Notes
- Flag name chosen per request: `enablefirstrun`.
- Description strings intentionally left non–translated (internal UI metadata).
- British English used throughout documentation.
