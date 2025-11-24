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

#include <QList>
#include <QString>
#include <QVariantMap>
#include <functional>
#include "Capability.hpp"

namespace opencardev::crankshaft {
namespace core {
namespace capabilities {

/**
 * Wireless capability for WiFi management.
 *
 * Extensions with this capability can:
 * - Scan for available WiFi networks
 * - Connect to WiFi networks
 * - Disconnect from networks
 * - Get connection status and signal strength
 * - Configure access point mode
 *
 * Extensions cannot directly access system network interfaces.
 * All operations are logged and subject to security policies.
 */
class WirelessCapability : public Capability {
  public:
    virtual ~WirelessCapability() = default;

    QString id() const override { return "wireless"; }

    /**
     * WiFi network information.
     */
    struct NetworkInfo {
        QString ssid;
        QString bssid;
        int signalStrength;  // 0-100
        bool isSecure;
        QString securityType;  // WPA2, WPA3, WEP, Open
        int frequency;         // MHz
        bool isConnected;
    };

    /**
     * Connection state.
     */
    enum class ConnectionState { Disconnected, Connecting, Connected, Disconnecting, Failed };

    /**
     * Scan for available WiFi networks.
     *
     * @param callback Called with list of networks when scan completes
     */
    virtual void scanNetworks(std::function<void(const QList<NetworkInfo>&)> callback) = 0;

    /**
     * Connect to a WiFi network.
     *
     * @param ssid Network SSID
     * @param password Network password (empty for open networks)
     * @param callback Called with success status
     */
    virtual void connectToNetwork(
        const QString& ssid, const QString& password,
        std::function<void(bool success, const QString& error)> callback) = 0;

    /**
     * Disconnect from current WiFi network.
     */
    virtual void disconnect() = 0;

    /**
     * Get current connection state.
     */
    virtual ConnectionState getConnectionState() const = 0;

    /**
     * Get currently connected network info.
     * Returns empty NetworkInfo if not connected.
     */
    virtual NetworkInfo getCurrentNetwork() const = 0;

    /**
     * Subscribe to connection state changes.
     *
     * @param callback Called when connection state changes
     * @return Subscription ID for unsubscribe
     */
    virtual int subscribeToStateChanges(
        std::function<void(ConnectionState state, const QString& ssid)> callback) = 0;

    /**
     * Unsubscribe from state change notifications.
     *
     * @param subscriptionId ID returned from subscribeToStateChanges
     */
    virtual void unsubscribe(int subscriptionId) = 0;

    /**
     * Get list of saved/known networks.
     */
    virtual QList<QString> getSavedNetworks() const = 0;

    /**
     * Remove a saved network.
     *
     * @param ssid Network SSID to forget
     */
    virtual void forgetNetwork(const QString& ssid) = 0;

    /**
     * Enable/disable WiFi radio.
     *
     * @param enabled true to enable, false to disable
     */
    virtual void setEnabled(bool enabled) = 0;

    /**
     * Check if WiFi is currently enabled.
     */
    virtual bool isEnabled() const = 0;

    /**
     * Configure WiFi access point mode (hotspot).
     *
     * @param ssid AP SSID
     * @param password AP password (min 8 chars)
     * @param callback Called with success status
     */
    virtual void configureAccessPoint(
        const QString& ssid, const QString& password,
        std::function<void(bool success, const QString& error)> callback) = 0;
};

}  // namespace capabilities
}  // namespace core
}  // namespace opencardev::crankshaft
