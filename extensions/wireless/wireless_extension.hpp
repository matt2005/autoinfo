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

#pragma once

#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusReply>
#include <QList>
#include <QObject>
#include <QTimer>
#include <QVariantMap>
#include "../../src/core/capabilities/WirelessCapability.hpp"
#include "../../src/extensions/extension.hpp"

namespace opencardev {
namespace crankshaft {
namespace extensions {
namespace wireless {

/**
 * Wireless extension using NetworkManager D-Bus interface.
 * Provides WiFi scanning, connection management, and AP mode.
 */
class WirelessExtension : public Extension {
  public:
    WirelessExtension();
    ~WirelessExtension() override;

    // Lifecycle methods
    bool initialize() override;
    void start() override;
    void stop() override;
    void cleanup() override;

    // Metadata
    QString id() const override { return "wireless"; }
    QString name() const override { return "Wireless Manager"; }
    QString version() const override { return "1.0.0"; }
    ExtensionType type() const override { return ExtensionType::UI; }

    // Configuration
    void registerConfigItems(core::config::ConfigManager* manager) override;

  private:
    void setupEventHandlers();
    void setupNetworkManager();
    void scanNetworksInternal();
    void connectToNetworkInternal(const QString& ssid, const QString& password);
    void disconnectInternal();
    void configureAccessPointInternal(const QString& ssid, const QString& password);
    void updateConnectionState();
    void publishNetworkList(
        const QList<core::capabilities::WirelessCapability::NetworkInfo>& networks);
    void publishConnectionStateChanged();

    // NetworkManager D-Bus methods
    QList<QDBusObjectPath> getWirelessDevices();
    QList<QDBusObjectPath> getAccessPoints(const QDBusObjectPath& devicePath);
    QVariantMap getAccessPointProperties(const QDBusObjectPath& apPath);
    QString getActiveConnectionSsid();

    // Event handlers
    void handleScanRequest(const QVariantMap& data);
    void handleConnectRequest(const QVariantMap& data);
    void handleDisconnectRequest(const QVariantMap& data);
    void handleAccessPointRequest(const QVariantMap& data);
    void handleForgetNetworkRequest(const QVariantMap& data);
    void handleToggleWifiRequest(const QVariantMap& data);

    QDBusInterface* nmInterface_;
    QDBusInterface* settingsInterface_;
    QTimer* scanTimer_;
    QTimer* stateMonitorTimer_;

    QList<core::capabilities::WirelessCapability::NetworkInfo> cachedNetworks_;
    QString currentSsid_;
    bool isScanning_;
    bool wifiEnabled_;

    QList<int> eventSubscriptions_;
};

}  // namespace wireless
}  // namespace extensions
}  // namespace crankshaft
}  // namespace opencardev
