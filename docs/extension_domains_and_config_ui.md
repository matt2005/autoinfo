# Extension Domains and Configuration UI

## Overview

This document describes the extension domain system and configuration UI implemented in Crankshaft Reborn, which provides better organization, security, and management of extensions.

## Extension Domains

### Domain Types

Extensions are now categorized into domains:

**1. Core Domain (`ExtensionDomain::Core`)**
- Built-in extensions developed and maintained by the OpenCarDev team
- Automatically trusted with full system access
- Auto-loaded on application startup
- Examples: navigation, wireless, media_player, bluetooth

**2. Third-Party Domain (`ExtensionDomain::ThirdParty`)**
- Community-developed extensions
- Subject to capability-based permission system
- Can be enabled/disabled via Config UI
- Support for hot reload during development

**3. Unknown Domain (`ExtensionDomain::Unknown`)**
- Extensions without a specified domain
- Treated conservatively for security

### Domain Declaration

Extensions declare their domain in `manifest.json`:

```json
{
  "id": "my_extension",
  "name": "My Extension",
  "version": "1.0.0",
  "domain": "core",
  ...
}
```

Valid domain values:
- `"core"` - Core domain
- `"thirdparty"`, `"third-party"`, `"third_party"` - Third-party domain

## Simplified Extension Loading

### Automatic Domain-Based Loading

The main application now automatically loads extensions by domain, eliminating the need for manual registration:

**Before (main.cpp):**
```cpp
// Manual registration required for each extension
auto navigationExtension = std::make_shared<NavigationExtension>();
QString navPath = findExtensionPath("navigation");
application.extensionManager()->registerBuiltInExtension(navigationExtension, navPath);

auto wirelessExtension = std::make_shared<WirelessExtension>();
QString wirelessPath = findExtensionPath("wireless");
application.extensionManager()->registerBuiltInExtension(wirelessExtension, wirelessPath);
```

**After (main.cpp):**
```cpp
// Automatic loading by domain
application.extensionManager()->loadAllByDomain(ExtensionDomain::Core);
application.extensionManager()->loadAllByDomain(ExtensionDomain::ThirdParty);
```

### Extension Discovery

Extensions are discovered from multiple search paths:
1. Environment variable: `CRANKSHAFT_EXTENSIONS_PATH`
2. Application directory: `<app_dir>/extensions`
3. Current directory: `./extensions`
4. Build directory: `./build/extensions`
5. System directories: `/usr/share/CrankshaftReborn/extensions`

## Extension Management API

### ExtensionManager New Methods

**Domain-based loading:**
```cpp
void loadAllByDomain(ExtensionDomain domain);
QStringList getLoadedExtensionsByDomain(ExtensionDomain domain) const;
QStringList discoverExtensions(const QString& path, ExtensionDomain domain);
ExtensionDomain getExtensionDomain(const QString& extension_id) const;
```

**Enable/Disable:**
```cpp
bool enableExtension(const QString& extension_id);
bool disableExtension(const QString& extension_id);
bool isExtensionEnabled(const QString& extension_id) const;
QStringList getDisabledExtensions() const;
```

**Hot Reload:**
```cpp
bool reloadExtension(const QString& extension_id);
```

**Configuration:**
```cpp
void setExtensionsDirectory(const QString& dir);
QString getExtensionsDirectory() const;
QStringList getExtensionSearchPaths() const;
```

### Disabled Extensions Persistence

Disabled extensions are saved to:
```
~/.config/CrankshaftReborn/disabled_extensions.conf
```

Format:
```
# Disabled Crankshaft Extensions
extension_id_1
extension_id_2
```

## Configuration UI

### ExtensionManagerBridge

QML bridge providing extension management interface:

**QML Methods:**
```qml
ExtensionManagerBridge.getLoadedExtensions()      // Returns list of loaded extensions
ExtensionManagerBridge.getAvailableExtensions()   // Returns all discovered extensions
ExtensionManagerBridge.getExtensionInfo(id)       // Get detailed info for extension
ExtensionManagerBridge.reloadExtension(id)        // Hot reload extension
ExtensionManagerBridge.enableExtension(id)        // Enable disabled extension
ExtensionManagerBridge.disableExtension(id)       // Disable extension
ExtensionManagerBridge.isExtensionEnabled(id)     // Check if enabled
ExtensionManagerBridge.refreshExtensions()        // Trigger UI refresh
```

**QML Signals:**
```qml
onExtensionLoaded(extensionId)
onExtensionUnloaded(extensionId)
onExtensionError(extensionId, error)
onExtensionsRefreshed()
```

### Config UI Components

#### ConfigView.qml
Main configuration interface with tabbed navigation:
- **Extensions Tab** - Extension management interface
- **System Tab** - System settings (placeholder)
- **Display Tab** - Display settings (placeholder)
- **About Tab** - Application information

#### ExtensionManagerView.qml
Comprehensive extension management UI with:

**Features:**
1. **Extension List**
   - Shows all discovered extensions
   - Visual badges for domain (Core/Third-Party) and load status
   - Color-coded by load state
   - Opacity indicates enabled/disabled state

2. **Filtering**
   - Domain filter: All, Core, Third-Party
   - Status filter: All, Loaded, Not Loaded, Enabled, Disabled
   - Text search: Filter by name, ID, or description

3. **Actions per Extension**
   - **Reload** - Hot reload extension (third-party only)
   - **Enable/Disable** - Toggle extension state
   - **Info** - View detailed extension information

4. **Extension Information Dialog**
   - Displays complete extension metadata
   - Shows dependencies and permissions
   - Platform compatibility information

5. **Error Reporting**
   - Popup dialog for extension errors
   - Shows extension ID and error message

### UI Integration

Access Config UI from main menu or settings:

```qml
import QtQuick
import QtQuick.Controls

ApplicationWindow {
    // ... other components
    
    Button {
        text: "⚙️ Settings"
        onClicked: {
            configLoader.source = "qml/ConfigView.qml"
            configDialog.open()
        }
    }
    
    Dialog {
        id: configDialog
        width: 800
        height: 600
        modal: true
        
        Loader {
            id: configLoader
            anchors.fill: parent
        }
    }
}
```

## Usage Examples

### Disabling a Third-Party Extension

**Via Code:**
```cpp
extensionManager->disableExtension("community_plugin");
// Extension is unloaded and won't load on next startup
```

**Via UI:**
1. Open Config → Extensions tab
2. Find the extension in the list
3. Click "🚫 Disable" button
4. Extension is unloaded immediately
5. Will remain disabled after restart

### Hot Reloading During Development

**Via Code:**
```cpp
extensionManager->reloadExtension("my_extension");
// Extension is unloaded and reloaded with latest code
```

**Via UI:**
1. Modify extension code
2. Open Config → Extensions tab
3. Find your extension
4. Click "🔄 Reload" button
5. Extension reloads without restarting application

### Creating a Third-Party Extension

**manifest.json:**
```json
{
  "id": "my_custom_plugin",
  "name": "My Custom Plugin",
  "version": "1.0.0",
  "description": "Adds custom functionality",
  "author": "Your Name",
  "type": "ui",
  "domain": "thirdparty",
  "dependencies": [],
  "platforms": ["linux"],
  "requirements": {
    "min_core_version": "1.0.0",
    "required_permissions": ["ui", "event"]
  }
}
```

**Installation:**
1. Place extension in `~/.local/share/CrankshaftReborn/extensions/my_custom_plugin/`
2. Restart application or use Config UI to refresh
3. Extension appears in Extensions list
4. Enable if needed
5. Extension loads automatically

## Security Considerations

### Domain-Based Trust Model

**Core Extensions:**
- Full trust by default
- Direct access to all capabilities
- No user confirmation required
- Installed in system directories

**Third-Party Extensions:**
- Sandboxed by capability system
- Explicit permission grants required in manifest
- User can disable at any time
- Installed in user directories

### Capability Enforcement

Extensions must declare required capabilities in manifest:

```json
"requirements": {
  "required_permissions": [
    "ui",
    "event",
    "location",
    "network"
  ]
}
```

CapabilityManager validates and enforces these permissions.

## Development Workflow

### Developing Core Extensions

1. Create extension in `extensions/` directory
2. Set `"domain": "core"` in manifest
3. Extension auto-loads on startup
4. No need to modify main.cpp

### Developing Third-Party Extensions

1. Create extension in development directory
2. Set `"domain": "thirdparty"` in manifest
3. Set `CRANKSHAFT_EXTENSIONS_PATH` environment variable
4. Use Config UI hot reload for rapid iteration
5. Package for distribution

## Migration Guide

### Updating Existing Extensions

**Add domain to manifest:**
```json
{
  "id": "existing_extension",
  "domain": "core",  // Add this line
  ...
}
```

**Remove manual registration from main.cpp:**
```cpp
// Delete these lines:
auto myExtension = std::make_shared<MyExtension>();
extensionManager->registerBuiltInExtension(myExtension, path);
```

Extensions with `"domain": "core"` load automatically.

## Future Enhancements

### Planned Features

1. **Extension Store Integration**
   - Browse and install extensions from UI
   - Automatic updates
   - User ratings and reviews

2. **Enhanced Security**
   - Extension signing and verification
   - Runtime permission prompts
   - Sandboxing for untrusted extensions

3. **Dependency Management**
   - Automatic dependency resolution
   - Version compatibility checking
   - Conflict detection

4. **Extension Configuration**
   - Per-extension settings UI
   - Configuration import/export
   - Profile management

5. **Performance Monitoring**
   - Extension resource usage tracking
   - Performance metrics in UI
   - Automatic problematic extension detection

## Troubleshooting

### Extension Not Loading

**Check:**
1. Domain specified in manifest
2. Extension not in disabled list
3. Dependencies satisfied
4. Manifest syntax valid
5. Extension in search path

**View logs:**
```bash
# Extension manager logs all load attempts
./CrankshaftReborn 2>&1 | grep -i extension
```

### Hot Reload Not Working

**Possible causes:**
1. Extension has running background tasks
2. Extension didn't implement cleanup properly
3. Extension has circular dependencies

**Solution:**
Restart application for full reload.

## Conclusion

The extension domain system and configuration UI provide:
- **Simplified Development** - No manual registration needed
- **Better Organization** - Clear separation of core and third-party
- **Enhanced Security** - Domain-based trust model
- **User Control** - Easy enable/disable/reload via UI
- **Rapid Iteration** - Hot reload for development

This foundation enables a robust extension ecosystem while maintaining security and ease of use.
