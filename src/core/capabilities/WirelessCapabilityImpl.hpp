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
#pragma once

#include <QMap>
#include <QTimer>
#include "WirelessCapability.hpp"

namespace opencardev::crankshaft::core::capabilities {

class WirelessCapabilityImpl : public WirelessCapability {
  public:
    explicit WirelessCapabilityImpl(const QString& extensionId) : extension_id_(extensionId) {}
    ~WirelessCapabilityImpl() override = default;

    // --- WirelessCapability interface stubs ---
    void scanNetworks(std::function<void(const QList<NetworkInfo>&)> callback) override {
        // Stub: return empty list immediately
        callback(QList<NetworkInfo>());
    }

    void connectToNetwork(
        const QString& ssid, const QString& password,
        std::function<void(bool success, const QString& error)> callback) override {
        Q_UNUSED(ssid);
        Q_UNUSED(password);
        // Stub: always fail (no backend yet)
        callback(false, QStringLiteral("Wireless backend not implemented"));
    }

    void disconnect() override { /* Stub */ }

    ConnectionState getConnectionState() const override { return ConnectionState::Disconnected; }

    NetworkInfo getCurrentNetwork() const override { return NetworkInfo(); }

    int subscribeToStateChanges(
        std::function<void(ConnectionState state, const QString& ssid)> callback) override {
        // Store callback; return subscription id
        const int id = ++last_sub_id_;
        state_callbacks_[id] = callback;
        return id;
    }

    void unsubscribe(int subscriptionId) override { state_callbacks_.remove(subscriptionId); }

    QList<QString> getSavedNetworks() const override { return QList<QString>(); }

    void forgetNetwork(const QString& ssid) override { Q_UNUSED(ssid); }

    void setEnabled(bool enabled) override { enabled_ = enabled; }

    bool isEnabled() const override { return enabled_; }

    void configureAccessPoint(
        const QString& ssid, const QString& password,
        std::function<void(bool success, const QString& error)> callback) override {
        Q_UNUSED(ssid);
        Q_UNUSED(password);
        callback(false, QStringLiteral("Access point configuration not implemented"));
    }

    // Base capability conformance
    QString id() const override { return QStringLiteral("wireless"); }
    bool isValid() const override { return is_valid_; }
    QString extensionId() const override { return extension_id_; }

  private:
    QString extension_id_;
    bool is_valid_ = true;
    bool enabled_ = true;
    int last_sub_id_ = 0;
    QMap<int, std::function<void(ConnectionState, const QString&)>> state_callbacks_;
};

// Factory helper used by CapabilityManager
inline std::shared_ptr<WirelessCapability> createWirelessCapabilityInstance(
    const QString& extensionId) {
    return std::static_pointer_cast<WirelessCapability>(
        std::make_shared<WirelessCapabilityImpl>(extensionId));
}

}  // namespace opencardev::crankshaft::core::capabilities
