/*
[]: #  * Project: Crankshaft
[]: #  * This file is part of Crankshaft project.
[]: #  * Copyright (C) 2025 OpenCarDev Team
[]: #  *
[]: #  *  Crankshaft is free software: you can redistribute it and/or modify
[]: #  *  it under the terms of the GNU General Public License as published by
[]: #  *  the Free Software Foundation; either version 3 of the License, or
[]: #  *  (at your option) any later version.
[]: #  *
[]: #  *  Crankshaft is distributed in the hope that it will be useful,
[]: #  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
[]: #  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
[]: #  *  GNU General Public License for more details.
[]: #  *
[]: #  *  You should have received a copy of the GNU General Public License
[]: #  *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
*/

#include "wireless_extension.hpp"
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QDBusObjectPath>
#include <QDebug>
#include "../../src/core/capabilities/EventCapability.hpp"
#include "../../src/core/capabilities/UICapability.hpp"

namespace opencardev {
namespace crankshaft {
namespace extensions {
namespace wireless {

WirelessExtension::WirelessExtension()
    : nmInterface_(nullptr),
      settingsInterface_(nullptr),
      scanTimer_(nullptr),
      stateMonitorTimer_(nullptr),
      isScanning_(false),
      wifiEnabled_(true) {}

WirelessExtension::~WirelessExtension() {
    cleanup();
}

bool WirelessExtension::initialize() {
    qInfo() << "Initializing Wireless extension...";

    // Check required capabilities (but allow initialization to proceed for config registration)
    if (!hasCapability("wireless")) {
        qWarning() << "Wireless: Wireless capability not granted - will operate in limited mode";
    }
    if (!hasCapability("event")) {
        qWarning() << "Wireless: Event capability not granted - event communication disabled";
    }
    if (!hasCapability("ui")) {
        qWarning() << "Wireless: UI capability not granted - UI registration disabled";
    }

    // Only setup NetworkManager if we have the wireless capability
    if (hasCapability("wireless")) {
        setupNetworkManager();
    }

    return true;
}

void WirelessExtension::start() {
    qInfo() << "Starting Wireless extension...";

    // Register UI
    auto uiCap = getCapability<core::capabilities::UICapability>();
    if (uiCap) {
        QVariantMap metadata;
        metadata["title"] = "WiFi";
        metadata["icon"] = "wifi";
        metadata["description"] = "WiFi network management";
        uiCap->registerMainView("qrc:/wireless/qml/WirelessView.qml", metadata);
        qInfo() << "Wireless: Registered main view";
    }

    // Subscribe to events
    setupEventHandlers();

    // Start periodic scanning
    scanTimer_ = new QTimer();
    QObject::connect(scanTimer_, &QTimer::timeout, [this]() { scanNetworksInternal(); });
    scanTimer_->start(15000);  // Scan every 15 seconds

    // Start connection state monitoring
    stateMonitorTimer_ = new QTimer();
    QObject::connect(stateMonitorTimer_, &QTimer::timeout, [this]() { updateConnectionState(); });
    stateMonitorTimer_->start(2000);  // Check every 2 seconds

    // Initial scan
    scanNetworksInternal();
    updateConnectionState();
}

void WirelessExtension::stop() {
    qInfo() << "Stopping Wireless extension...";

    if (scanTimer_) {
        scanTimer_->stop();
        delete scanTimer_;
        scanTimer_ = nullptr;
    }

    if (stateMonitorTimer_) {
        stateMonitorTimer_->stop();
        delete stateMonitorTimer_;
        stateMonitorTimer_ = nullptr;
    }

    // Unsubscribe from events
    auto eventCap = getCapability<core::capabilities::EventCapability>();
    if (eventCap) {
        for (int subId : eventSubscriptions_) {
            eventCap->unsubscribe(subId);
        }
        eventSubscriptions_.clear();
    }
}

void WirelessExtension::cleanup() {
    qInfo() << "Cleaning up Wireless extension...";

    if (nmInterface_) {
        delete nmInterface_;
        nmInterface_ = nullptr;
    }

    if (settingsInterface_) {
        delete settingsInterface_;
        settingsInterface_ = nullptr;
    }
}

void WirelessExtension::registerConfigItems(core::config::ConfigManager* manager) {
    using namespace core::config;

    ConfigPage page;
    page.domain = "connectivity";
    page.extension = "wireless";
    page.title = "WiFi Settings";
    page.description = "Configure wireless network connections and access point mode";
    page.icon = "qrc:/icons/wifi.svg";
    page.complexity = ConfigComplexity::Basic;

    // Network Settings Section
    ConfigSection networkSection;
    networkSection.key = "network";
    networkSection.title = "Network Settings";
    networkSection.description = "WiFi network connection settings";
    networkSection.complexity = ConfigComplexity::Basic;

    ConfigItem autoConnectItem;
    autoConnectItem.key = "auto_connect";
    autoConnectItem.label = "Auto-connect to known networks";
    autoConnectItem.description = "Automatically connect to saved networks when in range";
    autoConnectItem.type = ConfigItemType::Boolean;
    autoConnectItem.defaultValue = true;
    autoConnectItem.complexity = ConfigComplexity::Basic;
    autoConnectItem.required = false;
    autoConnectItem.readOnly = false;
    networkSection.items.append(autoConnectItem);

    ConfigItem scanIntervalItem;
    scanIntervalItem.key = "scan_interval";
    scanIntervalItem.label = "Network scan interval";
    scanIntervalItem.description = "How often to scan for available networks";
    scanIntervalItem.type = ConfigItemType::Integer;
    scanIntervalItem.defaultValue = 15;
    scanIntervalItem.properties["minValue"] = 5;
    scanIntervalItem.properties["maxValue"] = 60;
    scanIntervalItem.unit = "seconds";
    scanIntervalItem.complexity = ConfigComplexity::Advanced;
    scanIntervalItem.required = false;
    scanIntervalItem.readOnly = false;
    networkSection.items.append(scanIntervalItem);

    ConfigItem powerSaveItem;
    powerSaveItem.key = "power_save";
    powerSaveItem.label = "WiFi power saving mode";
    powerSaveItem.description = "Enable power saving to reduce battery consumption";
    powerSaveItem.type = ConfigItemType::Boolean;
    powerSaveItem.defaultValue = true;
    powerSaveItem.complexity = ConfigComplexity::Advanced;
    powerSaveItem.required = false;
    powerSaveItem.readOnly = false;
    networkSection.items.append(powerSaveItem);

    page.sections.append(networkSection);

    // Access Point Settings Section
    ConfigSection apSection;
    apSection.key = "access_point";
    apSection.title = "Access Point Mode";
    apSection.description = "Configure device as a WiFi access point";
    apSection.complexity = ConfigComplexity::Advanced;

    ConfigItem apEnabledItem;
    apEnabledItem.key = "ap_enabled";
    apEnabledItem.label = "Enable access point mode";
    apEnabledItem.description = "Allow other devices to connect to this device";
    apEnabledItem.type = ConfigItemType::Boolean;
    apEnabledItem.defaultValue = false;
    apEnabledItem.complexity = ConfigComplexity::Advanced;
    apEnabledItem.required = false;
    apEnabledItem.readOnly = false;
    apSection.items.append(apEnabledItem);

    ConfigItem apSsidItem;
    apSsidItem.key = "ap_ssid";
    apSsidItem.label = "Access point name (SSID)";
    apSsidItem.description = "Network name visible to other devices";
    apSsidItem.type = ConfigItemType::String;
    apSsidItem.defaultValue = "Crankshaft-AP";
    apSsidItem.complexity = ConfigComplexity::Advanced;
    apSsidItem.required = false;
    apSsidItem.readOnly = false;
    apSection.items.append(apSsidItem);

    ConfigItem apPasswordItem;
    apPasswordItem.key = "ap_password";
    apPasswordItem.label = "Access point password";
    apPasswordItem.description = "Password for access point (minimum 8 characters)";
    apPasswordItem.type = ConfigItemType::String;
    apPasswordItem.defaultValue = "";
    apPasswordItem.properties["minLength"] = 8;
    apPasswordItem.properties["maxLength"] = 63;
    apPasswordItem.properties["secret"] = true;
    apPasswordItem.complexity = ConfigComplexity::Advanced;
    apPasswordItem.required = false;
    apPasswordItem.readOnly = false;
    apSection.items.append(apPasswordItem);

    ConfigItem apChannelItem;
    apChannelItem.key = "ap_channel";
    apChannelItem.label = "WiFi channel";
    apChannelItem.description = "WiFi channel for access point (1-11)";
    apChannelItem.type = ConfigItemType::Integer;
    apChannelItem.defaultValue = 6;
    apChannelItem.properties["minValue"] = 1;
    apChannelItem.properties["maxValue"] = 11;
    apChannelItem.complexity = ConfigComplexity::Expert;
    apChannelItem.required = false;
    apChannelItem.readOnly = false;
    apSection.items.append(apChannelItem);

    page.sections.append(apSection);

    // Security Settings Section
    ConfigSection securitySection;
    securitySection.key = "security";
    securitySection.title = "Security Settings";
    securitySection.description = "Advanced security and encryption options";
    securitySection.complexity = ConfigComplexity::Expert;

    ConfigItem hiddenNetworksItem;
    hiddenNetworksItem.key = "show_hidden";
    hiddenNetworksItem.label = "Show hidden networks";
    hiddenNetworksItem.description = "Display networks that don't broadcast SSID";
    hiddenNetworksItem.type = ConfigItemType::Boolean;
    hiddenNetworksItem.defaultValue = false;
    hiddenNetworksItem.complexity = ConfigComplexity::Expert;
    hiddenNetworksItem.required = false;
    hiddenNetworksItem.readOnly = false;
    securitySection.items.append(hiddenNetworksItem);

    ConfigItem randomMacItem;
    randomMacItem.key = "random_mac";
    randomMacItem.label = "Randomize MAC address";
    randomMacItem.description = "Use random MAC address for improved privacy";
    randomMacItem.type = ConfigItemType::Boolean;
    randomMacItem.defaultValue = false;
    randomMacItem.complexity = ConfigComplexity::Expert;
    randomMacItem.required = false;
    randomMacItem.readOnly = false;
    securitySection.items.append(randomMacItem);

    page.sections.append(securitySection);

    // Register the page
    manager->registerConfigPage(page);
    qInfo() << "Wireless extension registered config items";
}

void WirelessExtension::setupNetworkManager() {
    // Connect to NetworkManager D-Bus service
    nmInterface_ =
        new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager",
                           "org.freedesktop.NetworkManager", QDBusConnection::systemBus());

    if (!nmInterface_->isValid()) {
        qWarning() << "Failed to connect to NetworkManager:" << nmInterface_->lastError().message();
        return;
    }

    settingsInterface_ = new QDBusInterface(
        "org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager/Settings",
        "org.freedesktop.NetworkManager.Settings", QDBusConnection::systemBus());

    qInfo() << "Connected to NetworkManager D-Bus service";
}

void WirelessExtension::setupEventHandlers() {
    auto eventCap = getCapability<core::capabilities::EventCapability>();
    if (!eventCap)
        return;

    // Subscribe to wireless command events
    int subId = eventCap->subscribe("wireless.scan",
                                    [this](const QVariantMap& data) { handleScanRequest(data); });
    eventSubscriptions_.append(subId);

    subId = eventCap->subscribe("wireless.connect",
                                [this](const QVariantMap& data) { handleConnectRequest(data); });
    eventSubscriptions_.append(subId);

    subId = eventCap->subscribe("wireless.disconnect",
                                [this](const QVariantMap& data) { handleDisconnectRequest(data); });
    eventSubscriptions_.append(subId);

    subId = eventCap->subscribe("wireless.access_point", [this](const QVariantMap& data) {
        handleAccessPointRequest(data);
    });
    eventSubscriptions_.append(subId);

    subId = eventCap->subscribe(
        "wireless.forget", [this](const QVariantMap& data) { handleForgetNetworkRequest(data); });
    eventSubscriptions_.append(subId);

    subId = eventCap->subscribe("wireless.toggle",
                                [this](const QVariantMap& data) { handleToggleWifiRequest(data); });
    eventSubscriptions_.append(subId);

    qInfo() << "Wireless: Subscribed to command events";
}

void WirelessExtension::scanNetworksInternal() {
    if (isScanning_ || !wifiEnabled_)
        return;

    isScanning_ = true;
    QList<core::capabilities::WirelessCapability::NetworkInfo> networks;

    // Get wireless devices
    QList<QDBusObjectPath> devices = getWirelessDevices();

    for (const QDBusObjectPath& devicePath : devices) {
        // Request scan on device
        QDBusInterface deviceInterface("org.freedesktop.NetworkManager", devicePath.path(),
                                       "org.freedesktop.NetworkManager.Device.Wireless",
                                       QDBusConnection::systemBus());

        if (deviceInterface.isValid()) {
            QVariantMap scanOptions;
            deviceInterface.asyncCall("RequestScan", scanOptions);
        }

        // Get access points
        QList<QDBusObjectPath> accessPoints = getAccessPoints(devicePath);

        for (const QDBusObjectPath& apPath : accessPoints) {
            QVariantMap apProps = getAccessPointProperties(apPath);

            core::capabilities::WirelessCapability::NetworkInfo info;
            info.ssid = apProps.value("Ssid").toString();
            info.signalStrength = apProps.value("Strength").toInt();
            info.frequency = apProps.value("Frequency").toUInt();

            uint flags = apProps.value("Flags").toUInt();
            uint wpaFlags = apProps.value("WpaFlags").toUInt();
            uint rsnFlags = apProps.value("RsnFlags").toUInt();

            info.isSecure = (wpaFlags != 0 || rsnFlags != 0);
            if (rsnFlags != 0) {
                info.securityType = "WPA3/WPA2";
            } else if (wpaFlags != 0) {
                info.securityType = "WPA2";
            } else if (flags != 0) {
                info.securityType = "WEP";
            } else {
                info.securityType = "Open";
            }

            info.isConnected = (info.ssid == currentSsid_);
            info.bssid = apProps.value("HwAddress").toString();

            if (!info.ssid.isEmpty()) {
                networks.append(info);
            }
        }
    }

    cachedNetworks_ = networks;
    publishNetworkList(networks);
    isScanning_ = false;
}

void WirelessExtension::connectToNetworkInternal(const QString& ssid, const QString& password) {
    qInfo() << "Connecting to network:" << ssid;

    // Create connection settings
    QVariantMap connection;
    connection["id"] = ssid;
    connection["type"] = "802-11-wireless";
    connection["autoconnect"] = true;

    QVariantMap wireless;
    wireless["ssid"] = ssid.toUtf8();
    wireless["mode"] = "infrastructure";

    QVariantMap wirelessSecurity;
    if (!password.isEmpty()) {
        wirelessSecurity["key-mgmt"] = "wpa-psk";
        wirelessSecurity["psk"] = password;
    }

    QVariantMap ipv4;
    ipv4["method"] = "auto";

    QVariantMap ipv6;
    ipv6["method"] = "auto";

    QVariantMap settings;
    settings["connection"] = connection;
    settings["802-11-wireless"] = wireless;
    if (!password.isEmpty()) {
        settings["802-11-wireless-security"] = wirelessSecurity;
    }
    settings["ipv4"] = ipv4;
    settings["ipv6"] = ipv6;

    // Add and activate connection
    if (nmInterface_ && nmInterface_->isValid()) {
        QList<QDBusObjectPath> devices = getWirelessDevices();
        if (!devices.isEmpty()) {
            QDBusReply<QDBusObjectPath> reply = nmInterface_->call(
                "AddAndActivateConnection", QVariant::fromValue(settings),
                QVariant::fromValue(devices.first()), QVariant::fromValue(QDBusObjectPath("/")));

            if (reply.isValid()) {
                qInfo() << "Connection activated successfully";
                currentSsid_ = ssid;
                publishConnectionStateChanged();
            } else {
                qWarning() << "Failed to activate connection:" << reply.error().message();
            }
        }
    }
}

void WirelessExtension::disconnectInternal() {
    qInfo() << "Disconnecting from current network";

    if (nmInterface_ && nmInterface_->isValid()) {
        // Get active connections
        QDBusReply<QList<QDBusObjectPath>> reply = nmInterface_->call("GetActiveConnections");

        if (reply.isValid()) {
            for (const QDBusObjectPath& connPath : reply.value()) {
                QDBusInterface connInterface("org.freedesktop.NetworkManager", connPath.path(),
                                             "org.freedesktop.NetworkManager.Connection.Active",
                                             QDBusConnection::systemBus());

                QString type = connInterface.property("Type").toString();
                if (type == "802-11-wireless") {
                    nmInterface_->asyncCall("DeactivateConnection", QVariant::fromValue(connPath));
                }
            }
        }
    }

    currentSsid_.clear();
    publishConnectionStateChanged();
}

void WirelessExtension::configureAccessPointInternal(const QString& ssid, const QString& password) {
    qInfo() << "Configuring access point:" << ssid;

    if (password.length() < 8) {
        qWarning() << "AP password must be at least 8 characters";
        return;
    }

    // Create AP connection settings
    QVariantMap connection;
    connection["id"] = ssid + " (AP)";
    connection["type"] = "802-11-wireless";
    connection["autoconnect"] = false;

    QVariantMap wireless;
    wireless["ssid"] = ssid.toUtf8();
    wireless["mode"] = "ap";
    wireless["band"] = "bg";

    QVariantMap wirelessSecurity;
    wirelessSecurity["key-mgmt"] = "wpa-psk";
    wirelessSecurity["psk"] = password;

    QVariantMap ipv4;
    ipv4["method"] = "shared";

    QVariantMap settings;
    settings["connection"] = connection;
    settings["802-11-wireless"] = wireless;
    settings["802-11-wireless-security"] = wirelessSecurity;
    settings["ipv4"] = ipv4;

    // Add and activate AP connection
    if (nmInterface_ && nmInterface_->isValid()) {
        QList<QDBusObjectPath> devices = getWirelessDevices();
        if (!devices.isEmpty()) {
            QDBusReply<QDBusObjectPath> reply = nmInterface_->call(
                "AddAndActivateConnection", QVariant::fromValue(settings),
                QVariant::fromValue(devices.first()), QVariant::fromValue(QDBusObjectPath("/")));

            if (reply.isValid()) {
                qInfo() << "Access point configured successfully";
            } else {
                qWarning() << "Failed to configure AP:" << reply.error().message();
            }
        }
    }
}

void WirelessExtension::updateConnectionState() {
    QString activeSsid = getActiveConnectionSsid();

    if (activeSsid != currentSsid_) {
        currentSsid_ = activeSsid;
        publishConnectionStateChanged();
    }
}

void WirelessExtension::publishNetworkList(
    const QList<core::capabilities::WirelessCapability::NetworkInfo>& networks) {
    auto eventCap = getCapability<core::capabilities::EventCapability>();
    if (!eventCap)
        return;

    QVariantMap data;
    QVariantList networkList;

    for (const auto& network : networks) {
        QVariantMap netMap;
        netMap["ssid"] = network.ssid;
        netMap["bssid"] = network.bssid;
        netMap["signalStrength"] = network.signalStrength;
        netMap["isSecure"] = network.isSecure;
        netMap["securityType"] = network.securityType;
        netMap["frequency"] = network.frequency;
        netMap["isConnected"] = network.isConnected;
        networkList.append(netMap);
    }

    data["networks"] = networkList;
    data["count"] = networks.size();

    eventCap->emitEvent("networks_updated", data);
}

void WirelessExtension::publishConnectionStateChanged() {
    auto eventCap = getCapability<core::capabilities::EventCapability>();
    if (!eventCap)
        return;

    QVariantMap data;
    data["ssid"] = currentSsid_;
    data["connected"] = !currentSsid_.isEmpty();

    eventCap->emitEvent("connection_state_changed", data);
}

QList<QDBusObjectPath> WirelessExtension::getWirelessDevices() {
    QList<QDBusObjectPath> wirelessDevices;

    if (!nmInterface_ || !nmInterface_->isValid()) {
        return wirelessDevices;
    }

    QDBusReply<QList<QDBusObjectPath>> reply = nmInterface_->call("GetDevices");

    if (reply.isValid()) {
        for (const QDBusObjectPath& devicePath : reply.value()) {
            QDBusInterface deviceInterface("org.freedesktop.NetworkManager", devicePath.path(),
                                           "org.freedesktop.NetworkManager.Device",
                                           QDBusConnection::systemBus());

            uint deviceType = deviceInterface.property("DeviceType").toUInt();
            if (deviceType == 2) {  // NM_DEVICE_TYPE_WIFI = 2
                wirelessDevices.append(devicePath);
            }
        }
    }

    return wirelessDevices;
}

QList<QDBusObjectPath> WirelessExtension::getAccessPoints(const QDBusObjectPath& devicePath) {
    QList<QDBusObjectPath> accessPoints;

    QDBusInterface deviceInterface("org.freedesktop.NetworkManager", devicePath.path(),
                                   "org.freedesktop.NetworkManager.Device.Wireless",
                                   QDBusConnection::systemBus());

    if (deviceInterface.isValid()) {
        QDBusReply<QList<QDBusObjectPath>> reply = deviceInterface.call("GetAccessPoints");
        if (reply.isValid()) {
            accessPoints = reply.value();
        }
    }

    return accessPoints;
}

QVariantMap WirelessExtension::getAccessPointProperties(const QDBusObjectPath& apPath) {
    QVariantMap properties;

    QDBusInterface apInterface("org.freedesktop.NetworkManager", apPath.path(),
                               "org.freedesktop.DBus.Properties", QDBusConnection::systemBus());

    if (apInterface.isValid()) {
        QDBusReply<QVariantMap> reply =
            apInterface.call("GetAll", "org.freedesktop.NetworkManager.AccessPoint");

        if (reply.isValid()) {
            properties = reply.value();

            // Convert SSID from byte array to string
            if (properties.contains("Ssid")) {
                QByteArray ssidBytes = properties.value("Ssid").toByteArray();
                properties["Ssid"] = QString::fromUtf8(ssidBytes);
            }
        }
    }

    return properties;
}

QString WirelessExtension::getActiveConnectionSsid() {
    if (!nmInterface_ || !nmInterface_->isValid()) {
        return QString();
    }

    QDBusReply<QList<QDBusObjectPath>> reply = nmInterface_->call("GetActiveConnections");

    if (reply.isValid()) {
        for (const QDBusObjectPath& connPath : reply.value()) {
            QDBusInterface connInterface("org.freedesktop.NetworkManager", connPath.path(),
                                         "org.freedesktop.NetworkManager.Connection.Active",
                                         QDBusConnection::systemBus());

            QString type = connInterface.property("Type").toString();
            if (type == "802-11-wireless") {
                QString id = connInterface.property("Id").toString();
                return id;
            }
        }
    }

    return QString();
}

void WirelessExtension::handleScanRequest(const QVariantMap& data) {
    Q_UNUSED(data);
    scanNetworksInternal();
}

void WirelessExtension::handleConnectRequest(const QVariantMap& data) {
    QString ssid = data.value("ssid").toString();
    QString password = data.value("password").toString();

    if (!ssid.isEmpty()) {
        connectToNetworkInternal(ssid, password);
    }
}

void WirelessExtension::handleDisconnectRequest(const QVariantMap& data) {
    Q_UNUSED(data);
    disconnectInternal();
}

void WirelessExtension::handleAccessPointRequest(const QVariantMap& data) {
    QString ssid = data.value("ssid").toString();
    QString password = data.value("password").toString();

    if (!ssid.isEmpty() && password.length() >= 8) {
        configureAccessPointInternal(ssid, password);
    }
}

void WirelessExtension::handleForgetNetworkRequest(const QVariantMap& data) {
    QString ssid = data.value("ssid").toString();

    if (ssid.isEmpty())
        return;

    // Find and delete the connection
    if (settingsInterface_ && settingsInterface_->isValid()) {
        QDBusReply<QList<QDBusObjectPath>> reply = settingsInterface_->call("ListConnections");

        if (reply.isValid()) {
            for (const QDBusObjectPath& connPath : reply.value()) {
                QDBusInterface connInterface("org.freedesktop.NetworkManager", connPath.path(),
                                             "org.freedesktop.NetworkManager.Settings.Connection",
                                             QDBusConnection::systemBus());

                QDBusReply<QVariantMap> settingsReply = connInterface.call("GetSettings");
                if (settingsReply.isValid()) {
                    QVariantMap settings = settingsReply.value();
                    QString connId = settings.value("connection").toMap().value("id").toString();

                    if (connId == ssid || connId == ssid + " (AP)") {
                        connInterface.asyncCall("Delete");
                        qInfo() << "Forgot network:" << ssid;
                    }
                }
            }
        }
    }
}

void WirelessExtension::handleToggleWifiRequest(const QVariantMap& data) {
    bool enabled = data.value("enabled", !wifiEnabled_).toBool();

    if (nmInterface_ && nmInterface_->isValid()) {
        nmInterface_->setProperty("WirelessEnabled", enabled);
        wifiEnabled_ = enabled;
        qInfo() << "WiFi" << (enabled ? "enabled" : "disabled");
    }
}

}  // namespace wireless
}  // namespace extensions
}  // namespace crankshaft
}  // namespace opencardev
