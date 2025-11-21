# Extension UI Integration Architecture Options

## Overview

This document presents multiple architecture options for integrating extensions (like the Navigation extension) with the core UI while maintaining security boundaries, extensibility, and minimal core application changes.

## Design Goals

1. **Security Boundaries**: Extensions cannot compromise core or other extensions
2. **Minimal Core Changes**: Core application requires minimal modification for new extensions
3. **Extensibility**: Easy to add new extensions without core refactoring
4. **UI Integration**: Extensions can provide UI components that integrate seamlessly
5. **Isolation**: Extensions run in sandboxed environments with limited permissions
6. **Communication**: Secure, well-defined APIs for extension-to-core and extension-to-extension communication

---

## Option 1: QML Component Registry with Sandbox Isolation

### Architecture

Extensions register QML components with the core via a secure registry. The core provides a container system that loads extension UI components in isolated QML contexts.

### Key Components

```
Core Application
├── Extension Registry (C++)
│   ├── Component Metadata Store
│   ├── Permission Manager
│   └── Sandbox Factory
├── UI Container (QML)
│   ├── Extension View Loader
│   ├── Navigation Bar
│   └── Extension Switcher
└── API Bridge (WebSocket + Event Bus)
    ├── Permitted API Endpoints
    └── Request Validator
```

### Security Model

- **Process Isolation**: Each extension runs in its own QML context
- **API Whitelist**: Extensions declare required permissions in manifest
- **Capability-Based Access**: Core grants specific capabilities (location, network, storage)
- **Resource Limits**: CPU/memory quotas per extension
- **IPC Only**: Extensions communicate via event bus (no direct memory access)

### Implementation

**Extension Manifest** (manifest.json):
```json
{
  "id": "navigation",
  "ui_components": [
    {
      "id": "navigation_main",
      "qml_file": "qml/NavigationView.qml",
      "slot": "main",
      "icon": "assets/nav_icon.svg",
      "title": "Navigation"
    },
    {
      "id": "navigation_widget",
      "qml_file": "qml/NavigationWidget.qml",
      "slot": "widget",
      "size": "compact"
    }
  ],
  "permissions": {
    "location": { "reason": "GPS positioning for navigation" },
    "network": { "reason": "Download map tiles and traffic data" },
    "filesystem": { "reason": "Cache map data", "paths": ["$CACHE/maps"] }
  },
  "api_endpoints": [
    "navigation.setDestination",
    "navigation.getCurrentRoute",
    "navigation.getETA"
  ]
}
```

**Core Integration** (minimal changes):

```cpp
// src/extensions/extension_manager.hpp
class ExtensionManager {
public:
    // Register extension UI components
    void registerExtensionUI(const QString& extensionId, const QVariantMap& uiMetadata);
    
    // Get available UI components by slot type
    QList<ExtensionUIComponent> getComponentsForSlot(const QString& slotType);
    
    // Create sandboxed QML context for extension
    QQmlContext* createSandboxedContext(const QString& extensionId, QQmlEngine* engine);
    
private:
    QMap<QString, QList<ExtensionUIComponent>> ui_registry_;
    PermissionManager permission_manager_;
};
```

**QML Container** (assets/qml/Main.qml):

```qml
import QtQuick
import QtQuick.Controls
import CrankshaftReborn.UI 1.0

ApplicationWindow {
    id: root
    
    // Navigation bar showing available extensions
    TabBar {
        id: tabBar
        width: parent.width
        
        Repeater {
            model: ExtensionRegistry.mainComponents
            TabButton {
                text: modelData.title
                icon.source: modelData.icon
            }
        }
    }
    
    // Extension view container with sandboxing
    StackLayout {
        currentIndex: tabBar.currentIndex
        anchors.fill: parent
        
        Repeater {
            model: ExtensionRegistry.mainComponents
            
            // Each extension loads in isolated context
            Loader {
                source: modelData.qmlPath
                
                // Sandboxed context with limited APIs
                onLoaded: {
                    item.extensionAPI = ExtensionRegistry.createAPIBridge(modelData.extensionId)
                }
            }
        }
    }
}
```

**Extension QML** (extensions/navigation/qml/NavigationView.qml):

```qml
import QtQuick
import QtQuick.Controls
import QtLocation
import QtPositioning

Item {
    id: navigationView
    
    // API bridge provided by core (sandboxed)
    property var extensionAPI: null
    
    Map {
        anchors.fill: parent
        plugin: Plugin { name: "osm" }
        center: QtPositioning.coordinate(51.5074, 0.1278) // London
        zoomLevel: 12
        
        MapPolyline {
            id: routeLine
            line.color: Theme.accent
            line.width: 5
        }
    }
    
    // Request data via sandboxed API
    Component.onCompleted: {
        if (extensionAPI) {
            extensionAPI.requestPermission("location", function(granted) {
                if (granted) {
                    extensionAPI.subscribeToEvent("location.updated", onLocationUpdate)
                }
            })
        }
    }
    
    function onLocationUpdate(data) {
        // Handle location update from core
        map.center = QtPositioning.coordinate(data.lat, data.lng)
    }
    
    // Extension controls
    Column {
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 10
        
        Button {
            text: "Set Destination"
            onClicked: {
                extensionAPI.call("navigation.setDestination", {
                    lat: 51.5074,
                    lng: 0.1278,
                    name: "London Eye"
                })
            }
        }
    }
}
```

### Pros
- ✅ Strong security isolation via QML contexts
- ✅ Flexible UI component system
- ✅ Minimal core changes (just registry + loader)
- ✅ Extensions can provide multiple UI components (main view, widgets, etc.)
- ✅ Permission-based API access
- ✅ Resource monitoring and limits possible

### Cons
- ❌ QML context isolation has limitations (shared Qt runtime)
- ❌ Requires careful API design for security
- ❌ Performance overhead from context switching

---

## Option 2: WebSocket-Based UI with iframe-style Isolation

### Architecture

Extensions provide web-based UI (QML or WebView) that communicates with core exclusively via WebSocket. Similar to browser extension model.

### Key Components

```
Core Application
├── WebSocket API Server (Port 8080)
│   ├── Authentication & Session Management
│   ├── Permission-Based Routing
│   └── Request Rate Limiting
├── Extension UI Container (QML WebView or QtWebEngine)
│   ├── Isolated WebView per Extension
│   └── postMessage-style API
└── Extension Sandbox
    ├── No Direct File System Access
    ├── No Direct Qt API Access
    └── WebSocket API Only
```

### Security Model

- **Process Isolation**: Extensions run in separate WebView processes
- **Origin-Based Security**: Each extension has unique origin (extension://navigation)
- **Content Security Policy**: Strict CSP for extension content
- **API Token Authentication**: JWT tokens for WebSocket authentication
- **Rate Limiting**: Prevent API abuse
- **Same-Origin Policy**: Extensions cannot access other extension data

### Implementation

**Extension Structure**:
```
extensions/navigation/
├── manifest.json
├── navigation.so (C++ backend)
├── web/
│   ├── index.html
│   ├── app.js
│   └── style.css
└── assets/
```

**WebSocket API** (extensions communicate via WS):

```javascript
// Extension frontend (web/app.js)
class NavigationExtension {
    constructor() {
        this.ws = new WebSocket('ws://localhost:8080/extensions/navigation');
        this.setupAuthentication();
        this.setupEventHandlers();
    }
    
    async setupAuthentication() {
        // Core provides token during extension load
        const token = window.extensionToken;
        await this.send({
            type: 'authenticate',
            token: token
        });
    }
    
    async requestLocation() {
        const response = await this.send({
            type: 'api.call',
            method: 'location.getCurrent',
            params: {}
        });
        return response.data;
    }
    
    async setDestination(lat, lng, name) {
        await this.send({
            type: 'navigation.setDestination',
            data: { lat, lng, name }
        });
    }
    
    send(message) {
        return new Promise((resolve, reject) => {
            const id = Math.random().toString(36);
            message.id = id;
            
            const timeout = setTimeout(() => {
                reject(new Error('Request timeout'));
            }, 5000);
            
            const handler = (event) => {
                const response = JSON.parse(event.data);
                if (response.id === id) {
                    clearTimeout(timeout);
                    this.ws.removeEventListener('message', handler);
                    resolve(response);
                }
            };
            
            this.ws.addEventListener('message', handler);
            this.ws.send(JSON.stringify(message));
        });
    }
}
```

**Core WebSocket Server** (src/core/websocket_server.cpp):

```cpp
void WebSocketServer::handleExtensionMessage(
    const QString& extensionId,
    const QJsonObject& message
) {
    const QString type = message["type"].toString();
    
    // Check permissions
    if (!permission_manager_->hasPermission(extensionId, type)) {
        sendError(extensionId, "Permission denied: " + type);
        return;
    }
    
    // Rate limiting
    if (!rate_limiter_->checkLimit(extensionId)) {
        sendError(extensionId, "Rate limit exceeded");
        return;
    }
    
    // Route to appropriate handler
    if (type == "api.call") {
        handleAPICall(extensionId, message);
    } else if (type.startsWith("navigation.")) {
        routeToExtension("navigation", message);
    }
}
```

**QML Container** (assets/qml/Main.qml):

```qml
import QtQuick
import QtQuick.Controls
import QtWebView

ApplicationWindow {
    TabBar {
        id: tabBar
        // Populated from extension registry
    }
    
    StackLayout {
        currentIndex: tabBar.currentIndex
        
        Repeater {
            model: ExtensionRegistry.webExtensions
            
            WebView {
                // Isolated web view per extension
                url: "http://localhost:8080/extensions/" + modelData.id + "/index.html"
                
                // Inject extension token for authentication
                onLoadingChanged: {
                    if (loadRequest.status === WebView.LoadSucceededStatus) {
                        runJavaScript("window.extensionToken = '" + modelData.token + "';")
                    }
                }
            }
        }
    }
}
```

### Pros
- ✅ Strongest process isolation (WebView/QtWebEngine sandboxing)
- ✅ Well-understood security model (browser-like)
- ✅ Extensions can use web technologies (React, Vue, etc.)
- ✅ Easy to implement rate limiting and quotas
- ✅ Natural separation between frontend and backend
- ✅ Extension UI can be remote (web-hosted)

### Cons
- ❌ Higher resource usage (WebView per extension)
- ❌ WebSocket latency for UI interactions
- ❌ Requires QtWebEngine (larger dependency)
- ❌ Less native-looking UI
- ❌ More complex development (web + native)

---

## Option 3: Separate Process Architecture with D-Bus IPC

### Architecture

Each extension runs as a separate process. Core and extensions communicate via D-Bus for system integration and shared memory for performance-critical data (with strict access control).

### Key Components

```
Core Process (crankshaft-core)
├── D-Bus Service (org.opencardev.crankshaft)
│   ├── Extension Manager Interface
│   ├── Permission Manager Interface
│   └── Event Bus Interface
├── UI Process (QML)
└── Shared Memory Manager (Read-Only for Extensions)

Extension Process (crankshaft-ext-navigation)
├── D-Bus Client
├── Extension Backend (C++/Python)
└── UI Provider (Wayland Surface / QML Component)
```

### Security Model

- **OS-Level Process Isolation**: Full process separation via OS
- **D-Bus Security Policies**: XML policies define allowed method calls
- **Capabilities**: Linux capabilities for hardware access
- **Seccomp Filters**: Restrict system calls per extension
- **Namespace Isolation**: Separate mount/network namespaces
- **AppArmor/SELinux**: Mandatory access control profiles

### Implementation

**D-Bus Service Definition** (org.opencardev.crankshaft.xml):

```xml
<!DOCTYPE node PUBLIC "-//freedesktop//DTD D-BUS Object Introspection 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/introspect.dtd">
<node>
  <interface name="org.opencardev.crankshaft.Extension">
    <method name="RegisterUI">
      <arg name="extensionId" type="s" direction="in"/>
      <arg name="uiMetadata" type="a{sv}" direction="in"/>
      <arg name="success" type="b" direction="out"/>
    </method>
    
    <method name="RequestPermission">
      <arg name="extensionId" type="s" direction="in"/>
      <arg name="permission" type="s" direction="in"/>
      <arg name="granted" type="b" direction="out"/>
    </method>
    
    <signal name="ExtensionEvent">
      <arg name="eventType" type="s"/>
      <arg name="eventData" type="a{sv}"/>
    </signal>
  </interface>
  
  <interface name="org.opencardev.crankshaft.Navigation">
    <method name="SetDestination">
      <arg name="latitude" type="d" direction="in"/>
      <arg name="longitude" type="d" direction="in"/>
      <arg name="name" type="s" direction="in"/>
      <arg name="routeId" type="s" direction="out"/>
    </method>
    
    <method name="GetCurrentPosition">
      <arg name="position" type="(dd)" direction="out"/>
    </method>
    
    <signal name="RouteUpdated">
      <arg name="routeData" type="a{sv}"/>
    </signal>
  </interface>
</node>
```

**D-Bus Security Policy** (/etc/dbus-1/system.d/crankshaft.conf):

```xml
<!DOCTYPE busconfig PUBLIC "-//freedesktop//DTD D-BUS Bus Configuration 1.0//EN"
 "http://www.freedesktop.org/standards/dbus/1.0/busconfig.dtd">
<busconfig>
  <policy user="crankshaft">
    <allow own="org.opencardev.crankshaft"/>
  </policy>
  
  <policy group="crankshaft-extensions">
    <!-- Extensions can call specific methods -->
    <allow send_destination="org.opencardev.crankshaft"
           send_interface="org.opencardev.crankshaft.Extension"
           send_member="RegisterUI"/>
    <allow send_destination="org.opencardev.crankshaft"
           send_interface="org.opencardev.crankshaft.Extension"
           send_member="RequestPermission"/>
    
    <!-- Extensions can receive events -->
    <allow receive_interface="org.opencardev.crankshaft.Extension"
           receive_type="signal"/>
  </policy>
  
  <!-- Navigation extension specific policy -->
  <policy user="crankshaft-ext-navigation">
    <allow send_destination="org.opencardev.crankshaft"
           send_interface="org.opencardev.crankshaft.Navigation"/>
  </policy>
</busconfig>
```

**Extension Process** (extensions/navigation/main.cpp):

```cpp
// Separate process entry point
int main(int argc, char* argv[]) {
    QCoreApplication app(argc, argv);
    
    // Connect to D-Bus
    QDBusConnection bus = QDBusConnection::systemBus();
    if (!bus.isConnected()) {
        qFatal("Cannot connect to D-Bus");
        return 1;
    }
    
    // Create extension backend
    NavigationExtension extension;
    
    // Register with core
    QDBusInterface core(
        "org.opencardev.crankshaft",
        "/org/opencardev/crankshaft",
        "org.opencardev.crankshaft.Extension",
        bus
    );
    
    QVariantMap uiMetadata;
    uiMetadata["qml_component"] = "qrc:/NavigationView.qml";
    uiMetadata["title"] = "Navigation";
    uiMetadata["icon"] = "navigation";
    
    QDBusReply<bool> reply = core.call("RegisterUI", "navigation", uiMetadata);
    if (!reply.value()) {
        qFatal("Failed to register with core");
        return 1;
    }
    
    // Register D-Bus object for navigation service
    NavigationDBusAdaptor adaptor(&extension);
    bus.registerObject(
        "/org/opencardev/crankshaft/navigation",
        &extension
    );
    bus.registerService("org.opencardev.crankshaft.navigation");
    
    // Request permissions
    requestPermissions(core, {"location", "network", "filesystem"});
    
    return app.exec();
}
```

**Core UI Integration** (assets/qml/Main.qml):

```qml
import QtQuick
import QtQuick.Controls
import org.opencardev.crankshaft 1.0

ApplicationWindow {
    ExtensionManager {
        id: extensionManager
    }
    
    TabBar {
        id: tabBar
        Repeater {
            model: extensionManager.registeredExtensions
            TabButton {
                text: modelData.title
                icon.name: modelData.icon
            }
        }
    }
    
    StackLayout {
        currentIndex: tabBar.currentIndex
        
        Repeater {
            model: extensionManager.registeredExtensions
            
            // Extensions provide their UI via D-Bus
            Loader {
                source: modelData.qmlComponent
                
                // UI connects to extension's D-Bus interface
                onLoaded: {
                    item.dbusInterface = Qt.createQmlObject(
                        'import QtQuick; import QtDBus; ' +
                        'DBusInterface { ' +
                        '  service: "org.opencardev.crankshaft.' + modelData.id + '"; ' +
                        '  path: "/org/opencardev/crankshaft/' + modelData.id + '"; ' +
                        '  interface: "org.opencardev.crankshaft.' + modelData.id.charAt(0).toUpperCase() + modelData.id.slice(1) + '"; ' +
                        '}',
                        item
                    );
                }
            }
        }
    }
}
```

**SystemD Service** (systemd/crankshaft-navigation.service):

```ini
[Unit]
Description=Crankshaft Navigation Extension
Requires=crankshaft-core.service
After=crankshaft-core.service dbus.service

[Service]
Type=dbus
BusName=org.opencardev.crankshaft.navigation
User=crankshaft-ext-navigation
Group=crankshaft-extensions
ExecStart=/usr/lib/crankshaft/extensions/navigation/navigation-extension

# Security hardening
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=strict
ProtectHome=true
ReadWritePaths=/var/cache/crankshaft/navigation
CapabilityBoundingSet=CAP_NET_BIND_SERVICE
SystemCallFilter=@system-service
SystemCallFilter=~@privileged @resources

# Resource limits
MemoryLimit=256M
CPUQuota=25%

[Install]
WantedBy=crankshaft.target
```

### Pros
- ✅ Strongest isolation (OS-level process separation)
- ✅ Robust security via D-Bus policies, AppArmor, seccomp
- ✅ Extensions can crash without affecting core
- ✅ Resource limits enforced by systemd
- ✅ Well-integrated with Linux desktop ecosystem
- ✅ Easy to monitor/debug separate processes
- ✅ Extensions can be restarted independently

### Cons
- ❌ Most complex architecture
- ❌ Requires D-Bus configuration and policies
- ❌ Higher IPC overhead than in-process
- ❌ Platform-specific (Linux-centric)
- ❌ More difficult to develop and test
- ❌ Requires system integration (systemd services)

---

## Option 4: Hybrid In-Process with Capability-Based Security

### Architecture

Extensions run in-process but with capability-based security model. Core provides capabilities (objects) that extensions must use for all sensitive operations.

### Key Components

```
Core Application (Single Process)
├── Capability Manager
│   ├── Location Capability
│   ├── Network Capability
│   ├── FileSystem Capability
│   └── UI Capability
├── Extension Sandbox
│   ├── Restricted Qt API Access
│   ├── Capability-Based Access Control
│   └── Memory Isolation (where possible)
└── UI Component Registry
```

### Security Model

- **Capability Objects**: Extensions receive capability objects (not raw APIs)
- **Unforgeable References**: Capabilities cannot be created by extensions
- **Least Privilege**: Extensions only get capabilities they request and are granted
- **Revocable**: Core can revoke capabilities at runtime
- **Auditable**: All capability usage is logged
- **No Global State**: Extensions cannot access global Qt APIs directly

### Implementation

**Capability Interface** (src/core/capability.hpp):

```cpp
namespace openauto {
namespace core {
namespace capabilities {

// Base capability interface
class Capability {
public:
    virtual ~Capability() = default;
    virtual QString id() const = 0;
    virtual bool isValid() const = 0;
};

// Location capability
class LocationCapability : public Capability {
public:
    QString id() const override { return "location"; }
    
    // Controlled access to location data
    virtual QGeoCoordinate getCurrentPosition() = 0;
    virtual void subscribeToUpdates(std::function<void(QGeoCoordinate)> callback) = 0;
    virtual void unsubscribe() = 0;
    
    // Extensions cannot create this - only core can
protected:
    LocationCapability() = default;
};

// Network capability
class NetworkCapability : public Capability {
public:
    QString id() const override { return "network"; }
    
    // Controlled network access
    virtual QNetworkReply* get(const QUrl& url) = 0;
    virtual QNetworkReply* post(const QUrl& url, const QByteArray& data) = 0;
    
    // Extensions cannot access raw QNetworkAccessManager
protected:
    NetworkCapability() = default;
};

// File system capability (scoped to specific directory)
class FileSystemCapability : public Capability {
public:
    QString id() const override { return "filesystem"; }
    
    // Scoped file access
    virtual QFile* openFile(const QString& relativePath, QIODevice::OpenMode mode) = 0;
    virtual QDir scopedDirectory() const = 0;
    virtual QStringList listFiles() = 0;
    
    // Extension cannot access files outside scope
protected:
    FileSystemCapability(const QString& scopePath) : scope_path_(scopePath) {}
    QString scope_path_;
};

// UI capability
class UICapability : public Capability {
public:
    QString id() const override { return "ui"; }
    
    // Register UI components
    virtual void registerMainView(const QString& qmlPath, const QVariantMap& metadata) = 0;
    virtual void registerWidget(const QString& qmlPath, const QVariantMap& metadata) = 0;
    virtual void showNotification(const QString& title, const QString& message) = 0;
    
protected:
    UICapability() = default;
};

}  // namespace capabilities
}  // namespace core
}  // namespace openauto
```

**Capability Manager** (src/core/capability_manager.hpp):

```cpp
class CapabilityManager {
public:
    // Grant capability to extension (based on manifest permissions)
    std::shared_ptr<capabilities::Capability> grantCapability(
        const QString& extensionId,
        const QString& capabilityType,
        const QVariantMap& options = {}
    );
    
    // Revoke capability
    void revokeCapability(const QString& extensionId, const QString& capabilityType);
    
    // Check if extension has capability
    bool hasCapability(const QString& extensionId, const QString& capabilityType);
    
    // Audit log
    void logCapabilityUsage(const QString& extensionId, const QString& capabilityType, const QString& action);
    
private:
    // Factory methods (extensions cannot call these)
    std::shared_ptr<capabilities::LocationCapability> createLocationCapability();
    std::shared_ptr<capabilities::NetworkCapability> createNetworkCapability();
    std::shared_ptr<capabilities::FileSystemCapability> createFileSystemCapability(
        const QString& scopePath
    );
    std::shared_ptr<capabilities::UICapability> createUICapability(const QString& extensionId);
    
    QMap<QString, QMap<QString, std::shared_ptr<capabilities::Capability>>> granted_capabilities_;
    QMutex mutex_;
};
```

**Modified Extension Interface** (src/extensions/extension.hpp):

```cpp
class Extension {
public:
    virtual ~Extension() = default;

    // Lifecycle methods
    virtual bool initialize() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void cleanup() = 0;

    // Metadata
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual ExtensionType type() const = 0;

    // NO DIRECT DEPENDENCIES - Use capabilities instead
    // Removed: setEventBus, setWebSocketServer
    
    // Capability-based access
    void grantCapability(std::shared_ptr<core::capabilities::Capability> capability) {
        capabilities_[capability->id()] = capability;
    }
    
protected:
    // Extensions access capabilities, not raw core services
    template<typename T>
    std::shared_ptr<T> getCapability() {
        for (auto& cap : capabilities_) {
            auto typed = std::dynamic_pointer_cast<T>(cap);
            if (typed) return typed;
        }
        return nullptr;
    }
    
    QMap<QString, std::shared_ptr<core::capabilities::Capability>> capabilities_;
};
```

**Extension Implementation** (extensions/navigation/navigation_extension.cpp):

```cpp
bool NavigationExtension::initialize() {
    qInfo() << "Initializing Navigation extension...";
    
    // Request capabilities from core (core decides if granted)
    // This happens via extension manager based on manifest
    
    // Extensions don't have direct access to Qt APIs
    // They must use capabilities
    
    return true;
}

void NavigationExtension::start() {
    // Use location capability
    auto locationCap = getCapability<core::capabilities::LocationCapability>();
    if (locationCap && locationCap->isValid()) {
        locationCap->subscribeToUpdates([this](QGeoCoordinate coord) {
            onLocationUpdate(coord);
        });
    } else {
        qWarning() << "Location capability not granted";
    }
    
    // Use UI capability to register views
    auto uiCap = getCapability<core::capabilities::UICapability>();
    if (uiCap && uiCap->isValid()) {
        QVariantMap metadata;
        metadata["title"] = "Navigation";
        metadata["icon"] = "navigation";
        uiCap->registerMainView("qrc:/NavigationView.qml", metadata);
    }
    
    // Use network capability for map tiles
    auto networkCap = getCapability<core::capabilities::NetworkCapability>();
    if (networkCap && networkCap->isValid()) {
        downloadMapTile(networkCap);
    }
    
    // Use filesystem capability for caching
    auto fsCap = getCapability<core::capabilities::FileSystemCapability>();
    if (fsCap && fsCap->isValid()) {
        // Can only access files within granted scope (e.g., $CACHE/navigation/)
        auto cacheFile = fsCap->openFile("map_cache.db", QIODevice::ReadWrite);
    }
}

void NavigationExtension::setDestination(double lat, double lng, const QString& name) {
    // Extensions emit events via event capability (not direct event bus access)
    auto eventCap = getCapability<core::capabilities::EventCapability>();
    if (eventCap && eventCap->isValid()) {
        QVariantMap eventData;
        eventData["destination"] = QVariantMap{
            {"lat", lat},
            {"lng", lng},
            {"name", name}
        };
        eventCap->emit("navigation.destination_set", eventData);
    }
}
```

**Extension Manager Integration**:

```cpp
void ExtensionManager::loadExtension(const QString& path) {
    // Load manifest
    auto manifest = ExtensionManifest::fromFile(path + "/manifest.json");
    
    // Load shared library
    QPluginLoader loader(path + "/" + manifest.entryPoint());
    QObject* plugin = loader.instance();
    Extension* extension = qobject_cast<Extension*>(plugin);
    
    // Grant capabilities based on manifest permissions
    for (const auto& permission : manifest.permissions()) {
        if (shouldGrantPermission(extension->id(), permission)) {
            auto capability = capability_manager_->grantCapability(
                extension->id(),
                permission.type,
                permission.options
            );
            extension->grantCapability(capability);
        }
    }
    
    // Initialize extension
    extension->initialize();
    extension->start();
}
```

### Pros
- ✅ Balance of security and simplicity
- ✅ Fine-grained access control
- ✅ No IPC overhead (in-process)
- ✅ Capabilities are auditable and revocable
- ✅ Extensions cannot forge capabilities
- ✅ Easy to extend with new capability types
- ✅ Simpler than process isolation
- ✅ Cross-platform (no D-Bus dependency)

### Cons
- ❌ Not true process isolation (extensions share memory space)
- ❌ C++ extensions can potentially bypass security (pointer manipulation)
- ❌ Requires careful capability API design
- ❌ Extensions can still DoS via CPU/memory exhaustion
- ❌ Shared Qt runtime means extension bugs can crash core

---

## Comparison Matrix

| Feature | Option 1: QML Registry | Option 2: WebSocket | Option 3: D-Bus Process | Option 4: Capabilities |
|---------|----------------------|-------------------|---------------------|---------------------|
| **Security Level** | Medium | High | Very High | Medium-High |
| **Process Isolation** | No | Partial (WebView) | Yes (Full) | No |
| **Performance** | High | Medium | Medium | High |
| **Development Complexity** | Low | Medium | High | Medium |
| **Core Changes Required** | Minimal | Minimal | Moderate | Moderate |
| **Resource Usage** | Low | High | Medium | Low |
| **Extension Crash Impact** | Crashes Core | Isolated | Isolated | Crashes Core |
| **Platform Dependency** | Qt | Qt + WebEngine | Linux (D-Bus) | Qt |
| **Native UI Feel** | Excellent | Poor | Excellent | Excellent |
| **Remote Extensions** | No | Yes | No | No |
| **Suitable For** | Trusted Extensions | Untrusted/Web | Critical Security | Most Cases |

---

## Recommendation

**For your automotive infotainment project, I recommend a hybrid approach:**

### Phase 1: Option 4 (Capability-Based) for MVP
- Simplest to implement initially
- Good security for trusted extensions
- High performance, native UI
- Easy debugging and development

### Phase 2: Option 3 (D-Bus Process) for Production
- Migrate critical extensions to separate processes
- Full OS-level isolation
- Robust against extension crashes
- Production-grade security

### Implementation Strategy

1. **Start with Option 4**: Build capability-based system first
   - Implement core capabilities (Location, Network, FileSystem, UI, Event)
   - Create navigation extension demo using capabilities
   - Validate API design with 2-3 extensions

2. **Add Option 3 selectively**: Migrate to process isolation
   - Keep simple extensions in-process (Option 4)
   - Move complex/untrusted extensions to separate processes (Option 3)
   - Use D-Bus for IPC between processes
   - Maintain same capability API (implementation changes, not interface)

3. **Best of Both Worlds**:
   - Trusted, simple extensions run in-process (low overhead)
   - Complex or untrusted extensions run in separate processes (security)
   - Unified capability API for both models
   - Core application remains minimal

This approach gives you:
- ✅ Quick MVP with Option 4
- ✅ Path to production security with Option 3
- ✅ Flexibility to choose isolation level per extension
- ✅ Minimal core changes (capability API is consistent)
- ✅ Security boundaries that scale with requirements

---

## Next Steps

Choose your preferred option(s), and I'll create:
1. Complete implementation of the chosen architecture
2. Full navigation extension demo with UI
3. Security policy configuration
4. Testing framework
5. Developer documentation

Which option(s) would you like me to implement?
