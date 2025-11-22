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

#include "Capability.hpp"
#include <QString>
#include <QStringList>
#include <QList>
#include <functional>
#include <memory>

namespace opencardev::crankshaft {
namespace core {
namespace capabilities {

/**
 * Bluetooth capability providing controlled access to Bluetooth adapters
 * and devices. This abstracts underlying BlueZ / QtBluetooth APIs and
 * exposes a constrained surface for extensions.
 *
 * Extensions with this capability can:
 *  - Enumerate adapters
 *  - Select active adapter
 *  - Discover nearby devices (with timeout)
 *  - Query discovered devices
 *  - Request pairing (authorised)
 *  - Mark logical connection state (high-level, not profile specific)
 *
 * Real profile negotiation (A2DP/HFP/etc.) remains internal or exposed
 * via separate specialised capabilities in future iterations.
 */
class BluetoothCapability : public Capability {
public:
    struct Device {
        QString name;
        QString address;
        bool paired;
        bool connected;
        int rssi; // -1 if unknown
    };

    virtual ~BluetoothCapability() = default;

    QString id() const override { return "bluetooth"; }

    // List available adapters (addresses or symbolic names).
    virtual QStringList listAdapters() const = 0;
    // Currently selected adapter (empty if none).
    virtual QString currentAdapter() const = 0;
    // Select adapter by address or name.
    virtual bool selectAdapter(const QString& adapterId) = 0;

    // List known devices (from last discovery session or cached).
    virtual QList<Device> listDevices() const = 0;

    // Begin discovery for timeoutMs milliseconds (<=0 means default 10s).
    virtual bool startDiscovery(int timeoutMs) = 0;
    // Stop discovery early.
    virtual void stopDiscovery() = 0;

    // Request pairing of a device.
    virtual bool pairDevice(const QString& address) = 0;
    // Mark logical connect of a device (not low-level profile establishment).
    virtual bool connectDevice(const QString& address) = 0;
    // Mark logical disconnect of a device.
    virtual bool disconnectDevice(const QString& address) = 0;

    // Subscribe to device list updates. Returns subscription id.
    virtual int subscribeDevices(std::function<void(const QList<Device>&)> callback) = 0;
    // Unsubscribe from device updates.
    virtual void unsubscribeDevices(int subscriptionId) = 0;

protected:
    BluetoothCapability() = default;
};

// Factory declaration provided for CapabilityManager. Implemented in BluetoothCapability.cpp
// Forward declaration to avoid circular include with CapabilityManager.
namespace openauto { namespace core { class CapabilityManager; } }


} // namespace capabilities
} // namespace core
} // namespace openauto
