# Crankshaft Reborn - Project Structure and Coding Standards

## Namespace Convention

**Primary Namespace:** `opencardev::crankshaft`

All code in this project uses the nested namespace `opencardev::crankshaft`. This clearly identifies the project as part of the OpenCarDev organization and the Crankshaft product line.

Example:
```cpp
namespace opencardev::crankshaft {
namespace core {
    class Application {
        // ...
    };
}  // namespace core
}  // namespace opencardev::crankshaft
```

## Directory Structure

### Source Code Organization (`src/`)

The source code is organized into logical modules:

```
src/
├── core/                       # Core system components
│   ├── application/            # Application initialization and lifecycle
│   │   ├── application.hpp
│   │   └── application.cpp
│   ├── capabilities/           # Capability-based security system
│   │   ├── CapabilityManager.hpp
│   │   ├── CapabilityManager.cpp
│   │   ├── Capability.hpp
│   │   ├── *Capability.hpp     # Individual capability interfaces
│   │   └── *Capability.cpp     # Individual capability implementations
│   ├── config/                 # Configuration management
│   │   ├── ConfigManager.hpp
│   │   ├── ConfigManager.cpp
│   │   ├── ConfigTypes.hpp
│   │   └── ConfigTypes.cpp
│   ├── events/                 # Event bus system
│   │   ├── event_bus.hpp
│   │   └── event_bus.cpp
│   ├── network/                # Network communication
│   │   ├── websocket_server.hpp
│   │   └── websocket_server.cpp
│   └── ui/                     # UI abstractions and interfaces
│       └── UIRegistrar.hpp
├── ui/                         # UI implementation and QML bridges
│   ├── Theme.hpp/cpp
│   ├── ThemeManager.hpp/cpp
│   ├── ExtensionRegistry.hpp/cpp
│   ├── *Bridge.hpp/cpp         # QML bridges for various subsystems
│   └── UIRegistrarImpl.hpp/cpp
├── extensions/                 # Extension framework
│   ├── extension.hpp           # Base extension interface
│   ├── extension_manager.hpp/cpp
│   └── extension_manifest.hpp/cpp
├── api/                        # Public API definitions
└── main.cpp                    # Application entry point
```

### Extensions (`extensions/`)

Extensions mirror the core structure but are self-contained modules:

```
extensions/
├── CMakeLists.txt              # Master extension build config
├── <extension_name>/
│   ├── manifest.json           # Extension metadata
│   ├── CMakeLists.txt          # Extension-specific build
│   ├── <extension_name>_extension.hpp
│   ├── <extension_name>_extension.cpp
│   ├── qml/                    # QML UI components (if needed)
│   │   ├── qmldir
│   │   └── *.qml
│   ├── assets/                 # Extension-specific assets
│   └── resources.qrc           # Qt resource file (if needed)
```

## Include Path Conventions

### Within Core Modules

Files within the same core subfolder use simple includes:
```cpp
// In src/core/config/ConfigManager.hpp
#include "ConfigTypes.hpp"
```

Files referencing other core modules use relative paths:
```cpp
// In src/core/application/application.hpp
#include "../events/event_bus.hpp"
#include "../capabilities/CapabilityManager.hpp"
#include "../config/ConfigManager.hpp"
```

### From UI or Extensions to Core

Use relative paths from src:
```cpp
// In src/ui/NavigationBridge.cpp
#include "../core/capabilities/CapabilityManager.hpp"
#include "../core/events/event_bus.hpp"
```

### From main.cpp

Use paths relative to src:
```cpp
// In src/main.cpp
#include "core/application/application.hpp"
#include "ui/ThemeManager.hpp"
#include "extensions/extension_manager.hpp"
```

## CMakeLists.txt Organization

### Core (`src/core/CMakeLists.txt`)

Lists all source and header files with their subfolder paths:
```cmake
set(CORE_SOURCES
    application/application.cpp
    events/event_bus.cpp
    network/websocket_server.cpp
    capabilities/CapabilityManager.cpp
    config/ConfigManager.cpp
    # ...
)

set(CORE_HEADERS
    application/application.hpp
    events/event_bus.hpp
    # ...
)
```

### Extensions (`extensions/CMakeLists.txt` and `extensions/<name>/CMakeLists.txt`)

Each extension has its own CMakeLists.txt that is included by the master extensions CMakeLists.txt.

## Coding Style Guidelines

### File Headers

All new files must include the Crankshaft GPL header:

```cpp
/*
 * Project: Crankshaft
 * This file is part of Crankshaft project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  Crankshaft is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Crankshaft is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
 */
```

### Namespace Usage

- Always use C++17 nested namespace syntax: `namespace opencardev::crankshaft {`
- Close namespaces with comments: `}  // namespace opencardev::crankshaft`
- For nested sub-namespaces, use additional nesting:
  ```cpp
  namespace opencardev::crankshaft {
  namespace core {
      // ...
  }  // namespace core
  }  // namespace opencardev::crankshaft
  ```

### Include Guards

Use `#pragma once` for header guards.

### Forward Declarations

Forward declarations must use the full namespace:
```cpp
namespace opencardev::crankshaft::core {
    class ConfigManager;
}
```

### Documentation

- Use British English for all documentation and comments
- Document public APIs with clear descriptions
- Include usage examples where appropriate
- Update relevant documentation when making structural changes

## Benefits of This Structure

1. **Clear Separation of Concerns**: Each module has a dedicated folder making it easy to locate related files
2. **Scalability**: New subsystems can be added without cluttering existing directories
3. **Maintainability**: Related files are grouped together, reducing cognitive load
4. **Build System Clarity**: CMake files directly reflect the directory structure
5. **Extension Isolation**: Extensions are completely separate from core, promoting modularity

## Migration Notes

When moving files to the new structure:
1. Update all `#include` directives to reflect new paths
2. Update CMakeLists.txt to use subfolder paths
3. Test builds after each logical group of changes
4. Update documentation to reflect new locations

## See Also

- [Extension Development Guide](extension_development.md)
- [Contributing Guidelines](CONTRIBUTING.md)
- [UI Theme System](ui_theme_system.md)
