# Extension Loading and Configuration UI Fix

## Issue
Configuration UI and other extensions (bluetooth, media_player) were not working. Only the navigation extension was loading.

## Root Causes

### 1. Extensions Not Registered as Built-in
- Bluetooth and media_player extensions were only being discovered via manifest scanning
- They were never instantiated because dynamic plugin loading is not yet implemented
- Only navigation was registered as a built-in extension in `main.cpp`

### 2. Missing Manifest Files in Build Directory
- Extension discovery requires `manifest.json` in `build/extensions/[name]/` directories
- Bluetooth and media_player CMakeLists.txt files were missing POST_BUILD commands to copy manifests
- Navigation had the correct POST_BUILD commands as a reference

### 3. Missing Event Capability Permission
- Both bluetooth and media_player extensions require EventCapability to function
- Their manifest files didn't include "event" in `required_permissions`
- Without event capability, extensions fail to initialize

### 4. Missing Link Dependencies
- Main executable (`CrankshaftReborn`) only linked to `NavigationExtension`
- Bluetooth and media_player libraries weren't linked, causing undefined reference errors

### 5. ThemeManager Property Names
- ConfigPageView.qml referenced `ThemeManager.secondaryTextColor` (doesn't exist)
- Correct property name is `ThemeManager.textSecondaryColor`

## Solutions Implemented

### 1. Added POST_BUILD Commands
Updated `extensions/bluetooth/CMakeLists.txt` and `extensions/media_player/CMakeLists.txt`:
```cmake
set_target_properties(BluetoothExtension PROPERTIES 
    OUTPUT_NAME bluetooth
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/extensions/bluetooth
)

add_custom_command(TARGET BluetoothExtension POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/manifest.json
        ${CMAKE_BINARY_DIR}/extensions/bluetooth/manifest.json
    COMMENT "Copying bluetooth manifest to build directory"
)
```

### 2. Registered Extensions as Built-in
Updated `src/main.cpp`:
- Added includes for bluetooth and media_player extension headers
- Created extension instances before `application.initialize()`
- Called `registerBuiltInExtension()` for all three extensions

```cpp
#include "../extensions/bluetooth/bluetooth_extension.hpp"
#include "../extensions/media_player/media_player_extension.hpp"

// Create instances
auto bluetoothExtension = std::make_shared<opencardev::crankshaft::extensions::bluetooth::BluetoothExtension>();
auto mediaPlayerExtension = std::make_shared<opencardev::crankshaft::extensions::media::MediaPlayerExtension>();

// Register after ExtensionRegistry created
application.extensionManager()->registerBuiltInExtension(bluetoothExtension, btExtensionPath);
application.extensionManager()->registerBuiltInExtension(mediaPlayerExtension, mpExtensionPath);
```

### 3. Added Event Capability to Manifests
Updated `extensions/bluetooth/manifest.json` and `extensions/media_player/manifest.json`:
```json
"required_permissions": [
    "bluetooth",
    "contacts",
    "phone",
    "event"  // Added
]
```

### 4. Added Link Dependencies
Updated `CMakeLists.txt`:
```cmake
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        ...
        NavigationExtension
        BluetoothExtension      # Added
        MediaPlayerExtension    # Added
)
```

### 5. Fixed ThemeManager Property Names
Updated `assets/qml/ConfigPageView.qml`:
- Changed `ThemeManager.secondaryTextColor` → `ThemeManager.textSecondaryColor` (3 occurrences)

## Verification

After fixes, all three extensions successfully:
1. ✅ Discovered from manifest files
2. ✅ Loaded and instantiated
3. ✅ Granted required capabilities (event, bluetooth, filesystem, network, etc.)
4. ✅ Initialized successfully
5. ✅ Registered configuration pages:
   - `navigation.core`
   - `connectivity.bluetooth`
   - `media.player`

### Runtime Output
```
Extension loaded successfully: "navigation"
Extension loaded successfully: "bluetooth"
Extension loaded successfully: "media_player"
...
Registered config page: "navigation.core"
Registered config page: "connectivity.bluetooth"
Registered config page: "media.player"
...
Built-in extension started: "navigation"
Built-in extension started: "bluetooth"
Built-in extension started: "media_player"
```

## Files Modified

1. `extensions/bluetooth/CMakeLists.txt` - Added POST_BUILD manifest copy
2. `extensions/media_player/CMakeLists.txt` - Added POST_BUILD manifest copy
3. `extensions/bluetooth/manifest.json` - Added "event" permission
4. `extensions/media_player/manifest.json` - Added "event" permission
5. `src/main.cpp` - Added bluetooth and media_player registration
6. `CMakeLists.txt` - Added link dependencies
7. `assets/qml/ConfigPageView.qml` - Fixed ThemeManager property names

## Notes

- Dynamic plugin loading is not yet implemented - all extensions must be registered as built-in
- Unknown capabilities (contacts, phone, audio) are logged but don't prevent initialization
- Media player event subscriptions are denied (needs event pattern registration) but extension still runs
- Bluetooth operates in mock mode when no hardware is available

## Related Documentation

- Extension architecture: `docs/extension_development.md`
- UI theme system: `docs/ui_theme_system.md`
- Capability-based security (in source comments)
