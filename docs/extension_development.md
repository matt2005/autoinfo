# Extension Development Guide

This guide will help you create extensions for Crankshaft Reborn using Qt6.

## Overview

Extensions are self-contained modules that add functionality to Crankshaft Reborn. They can be written in C/C++, Python, or Node.js and can interact with the core system through the event bus and WebSocket API.

## Extension Types

### Service Extensions
Background services that provide functionality without a UI component.

**Examples**: Bluetooth manager, GPS service, weather service

### UI Extensions
Extensions that provide user interface components.

**Examples**: Media player, navigation, settings panel

### Integration Extensions
Extensions that integrate with third-party services.

**Examples**: Spotify, Android Auto, Apple CarPlay

### Platform Extensions
Platform-specific features and hardware integration.

**Examples**: GPIO controls, CAN bus integration, steering wheel controls

## Creating an Extension

### Step 1: Directory Structure

Create a new directory in the `extensions/` folder:

```
extensions/
└── my_extension/
    ├── manifest.json
    ├── my_extension.cpp
    ├── my_extension.hpp
    ├── CMakeLists.txt
    └── assets/
        └── icon.png
```

### Step 2: Manifest File

Create a `manifest.json` file:

```json
{
  "id": "my_extension",
  "name": "My Extension",
  "version": "1.0.0",
  "description": "Description of my extension",
  "author": "Your Name",
  "type": "service",
  "dependencies": [],
  "platforms": ["linux", "all"],
  "entry_point": "my_extension.so",
  "config_schema": "config_schema.json",
  "requirements": {
    "min_core_version": "1.0.0",
    "required_permissions": [
      "network",
      "filesystem"
    ]
  },
  "metadata": {
    "category": "utilities",
    "icon": "assets/icon.png"
  }
}
```

### Step 3: Implement the Extension

#### C++ Extension

```cpp
#include "../../src/extensions/extension.hpp"

namespace openauto {
namespace extensions {
namespace myext {

class MyExtension : public Extension {
public:
    MyExtension() = default;
    ~MyExtension() override = default;

    bool initialize() override {
        setupEventHandlers();
        return true;
    }

    void start() override {
        // Start the extension
    }

    void stop() override {
        // Stop the extension
    }

    void cleanup() override {
        // Clean up resources
    }

    QString id() const override { return "my_extension"; }
    QString name() const override { return "My Extension"; }
    QString version() const override { return "1.0.0"; }
    ExtensionType type() const override { return ExtensionType::Service; }

private:
    void setupEventHandlers() {
        event_bus_->subscribe("my.event", [this](const QVariantMap& data) {
            handleEvent(data);
        });
    }

    void handleEvent(const QVariantMap& data) {
        // Handle the event
    }
};

}  // namespace myext
}  // namespace extensions
}  // namespace openauto
```

### Step 4: Build Configuration

Create a `CMakeLists.txt` file for C++ extensions:

```cmake
cmake_minimum_required(VERSION 3.16)

project(MyExtension VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core)

add_library(my_extension SHARED
    my_extension.cpp
    my_extension.hpp
)

target_link_libraries(my_extension
    PRIVATE
        Qt6::Core
        CrankshaftCore
        CrankshaftExtensions
)

install(TARGETS my_extension
    LIBRARY DESTINATION share/CrankshaftReborn/extensions/my_extension
)

install(FILES manifest.json
    DESTINATION share/CrankshaftReborn/extensions/my_extension
)
```

## Event Bus API

### Subscribing to Events

```cpp
int subscription_id = event_bus_->subscribe("event.name", 
    [](const QVariantMap& data) {
        // Handle event
    });
```

### Publishing Events

```cpp
QVariantMap data;
data["key"] = "value";
event_bus_->publish("event.name", data);
```

### Unsubscribing

```cpp
event_bus_->unsubscribe(subscription_id);
```

## WebSocket API

### Sending Messages

```cpp
ws_server_->broadcast(QString("Message to all clients"));

// Or to a specific client
ws_server_->sendToClient(client, QString("Message"));
```

### Receiving Messages

```cpp
connect(ws_server_, &WebSocketServer::messageReceived,
    [](QWebSocket* client, const QString& message) {
        // Handle message
    });
```

## Permissions

Extensions must declare required permissions in their manifest:

- `network`: Network access
- `filesystem`: File system access
- `audio`: Audio device access
- `video`: Video device access
- `bluetooth`: Bluetooth access
- `location`: GPS/location access
- `contacts`: Contact list access
- `phone`: Phone functionality
- `system`: System-level operations

## Best Practices

1. **Error Handling**: Always handle errors gracefully
2. **Resource Management**: Clean up resources in `cleanup()`
3. **Thread Safety**: Use thread-safe operations
4. **Logging**: Use Qt's logging framework
5. **Dependencies**: Minimise dependencies
6. **Documentation**: Document your extension's API
7. **Versioning**: Follow semantic versioning
8. **Security**: Validate all input
9. **Performance**: Optimise for Raspberry Pi
10. **Accessibility**: Consider accessibility in UI extensions

## Examples

See the base extensions for complete examples:

- `extensions/media_player/`: UI extension with audio/video playback
- `extensions/navigation/`: UI extension with GPS integration
- `extensions/bluetooth/`: Service extension for Bluetooth connectivity

## Support

- Check the [FAQ](faq.md)
- Ask in [GitHub Discussions](https://github.com/opencardev/crankshaft_reborn/discussions)
- Open an [issue](https://github.com/opencardev/crankshaft_reborn/issues) for bugs
