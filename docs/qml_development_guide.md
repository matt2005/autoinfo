# QML Development Guide for Crankshaft

## Overview
This guide helps you set up a productive QML / Qt Quick development environment for Crankshaft (an automotive infotainment system) and introduces core concepts through a beginner tutorial. All instructions use British English and assume development on Linux or WSL with Qt 6.

## 1. Environment Setup

### 1.1 Install Required Packages (WSL / Debian-based)
```bash
sudo apt-get update
sudo apt-get install -y build-essential cmake ninja-build qt6-base-dev qt6-declarative-dev \
    qt6-tools-dev qt6-tools-dev-tools qt6-positioning-dev qt6-connectivity-dev qt6-websockets-dev \
    qt6-multimedia-dev clang-format qmllint qmlscene
```

### 1.2 Optional Tools
- **Qt Creator**: Integrated UI designer, QML profiler, live preview.
- **VS Code Extensions**:
  - `Qt QML` language support.
  - `C++` tools (clangd / CMake Tools).
  - `QML Formatter` for consistent styling.
- **qmlformat**: Reformat QML files.
- **qmllint**: Static analysis for QML (syntax + bindings).
- **qmlscene**: Launch and iterate quickly for an individual QML file.

### 1.3 Repository Tasks
Use provided VS Code tasks:
- `Configure CMake (Debug)` – generate build system.
- `Build (Debug)` – build C++ and QML resources.
- `Build Translations` – compile `.qm` files for i18n.
- `Run Application (VNC Debug)` – start with VNC platform for headless dev.

### 1.4 Live Iteration Workflow
1. Modify a QML file.
2. If not using qrc resources, the main app can hot-reload when restarted.
3. For isolated experimentation: `qmlscene assets/qml/ConfigScreen.qml`.
4. Use `console.log()` for debug output (visible in terminal).
5. Profiling: `qtcreator` -> Analyse -> QML Profiler (attach to running process or start within).

## 2. Project QML Structure
- Core UI QML: `assets/qml/` (Main window, config UI components, theme).
- Extension Views: Registered via `UICapability` (e.g., `qrc:/navigation/qml/NavigationView.qml`).
- Each extension should bundle its own QML under its directory (e.g., `extensions/bluetooth/assets/`).
- Resources use `.qrc` files with prefixes (`/bluetooth`, `/wireless`) and `alias` for logical paths.

## 3. Internationalisation (i18n)
- Strings wrapped with `qsTr("...")` become translatable.
- Build translations: `Build Translations` task or `cmake --build build --target translations`.
- Runtime loads `.qm` from `build/i18n/` (development) or install paths.
- Default locale: `en_GB`.

## 4. Theming
- Theme values exposed via `ThemeManager` singleton.
- Use semantic colours: `Theme.text`, `Theme.background`, `Theme.accent`.
- Avoid hard-coded colours; improves dark/light mode compatibility.

## 5. Capability-based Extension UI
- Extensions request `ui` permission, then call `UICapability::registerMainView(path, metadata)`.
- Tabs auto-populate from `ExtensionRegistry.mainComponents`.
- Metadata: `title`, `icon`, `description`.
- Resource path formats:
  - QRC: `qrc:/bluetooth/qml/BluetoothView.qml`
  - File: `/path/to/build/extensions/dialer/qml/DialerView.qml`

## 6. Quality and Static Analysis
Run:
```bash
qmllint assets/qml/Main.qml
qmlformat -i assets/qml/Main.qml
```
Guidelines:
- Keep component files short (<300 lines) by extracting subcomponents.
- Use `id` attributes consistently for referencing.
- Prefer `Layout` components (RowLayout/ColumnLayout/GridLayout) over manual anchors where appropriate.
- Use property binding instead of imperative JavaScript where possible.

## 7. Debugging Techniques
- Enable verbose QML logging: `QT_LOGGING_RULES="qt.qml.binding.removal=true"`.
- Check plugin loading issues by setting `QT_DEBUG_PLUGINS=1`.
- Use `Loader` `status` and `onStatusChanged` handlers for extension view diagnostics.
- Use `console.warn()` and `console.error()` for severity.

## 8. Beginner Tutorial: Creating a Simple Extension View
Goal: Create a minimal extension that displays current time and switches theme.

### 8.1 Create Extension Skeleton
Directory: `extensions/clock/`
Files:
- `manifest.json` (declare permissions: `ui`, `event`).
- `resources.qrc` with prefix `/clock` and alias `qml/ClockView.qml`.
- `ClockView.qml` UI file.
- C++: `clock_extension.hpp/.cpp` implementing minimal start logic.

### 8.2 Sample `ClockView.qml`
```qml
import QtQuick
import QtQuick.Controls

Item {
    width: 300; height: 120
    Timer { interval: 1000; running: true; repeat: true; onTriggered: timeText.text = new Date().toLocaleTimeString() }
    Text { id: timeText; text: "--:--"; font.pixelSize: 32; anchors.centerIn: parent }
    Button { text: "Toggle Theme"; anchors.bottom: parent.bottom; anchors.horizontalCenter: parent.horizontalCenter; onClicked: Theme.toggleTheme() }
}
```

### 8.3 Register View from Extension
```cpp
auto uiCap = getCapability<core::capabilities::UICapability>();
if (uiCap) {
    QVariantMap meta; meta["title"] = "Clock"; meta["icon"] = "placeholder"; meta["description"] = "Simple clock";
    uiCap->registerMainView("qrc:/clock/qml/ClockView.qml", meta);
}
```

### 8.4 Build & Run
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j
QT_QPA_PLATFORM=vnc ./build/CrankshaftReborn
```
Click the new tab labelled "Clock".

### 8.5 Add Translation
Wrap visible strings with `qsTr()` and rebuild `translations` target.

## 9. Common Pitfalls
| Issue | Cause | Resolution |
|-------|-------|-----------|
| `No such file or directory (qrc:/...)` | Missing `Q_INIT_RESOURCE` for static lib | Ensure resource included & (if static) initialise in `start()` |
| Undefined colour binding | Wrong Theme property name | Use documented names (`textSecondary`, `accent`) |
| Extension tab icon broken | Icon string not a path | Provide valid qrc/file path or rely on text-only tab |
| Translations not loading | LinguistTools not installed | Install `qt6-tools-dev` and rebuild translations |

## 10. Next Steps
- Explore QML performance profiling (FrameRate, Scene Graph live profiling).
- Integrate interactive maps (QtLocation) with capability gating.
- Add unit tests for JavaScript logic using `qmltestrunner`.

## 11. References
- Qt Documentation: https://doc.qt.io/
- QML Tutorial (Official): https://doc.qt.io/qt-6/qmltutorial.html
- Qt Quick Controls: https://doc.qt.io/qt-6/qtquickcontrols-index.html

---
Feel free to extend this guide with project-specific patterns or submit improvements via pull requests.
