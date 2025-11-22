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

#include "BluetoothCapability.hpp"
#include "Capability.hpp"
#include "CapabilityManager.hpp"
#include <QBluetoothLocalDevice>
#include <QBluetoothDeviceDiscoveryAgent>
#include <QBluetoothDeviceInfo>
#include <QTimer>
#include <QDateTime>
#include <QDebug>

namespace opencardev::crankshaft {
namespace core {
namespace capabilities {

// Concrete Bluetooth capability implementation (internal use only).
class BluetoothCapabilityImpl : public BluetoothCapability {
public:
    BluetoothCapabilityImpl(const QString& extensionId, CapabilityManager* manager)
        : extension_id_(extensionId), manager_(manager), is_valid_(true), next_sub_id_(1), discovery_agent_(nullptr), discovery_timer_(nullptr) {
        // Attempt to initialise local device (may be absent in WSL or container).
        local_device_ = std::make_unique<QBluetoothLocalDevice>();
        if (!local_device_->isValid()) {
            qWarning() << "Bluetooth local device not valid - will operate in mock mode";
            // Mock mode will fabricate an adapter and devices on demand.
        }
        current_adapter_ = local_device_->isValid() ? local_device_->address().toString() : QStringLiteral("mock-adapter");
    }

    ~BluetoothCapabilityImpl() override {
        if (discovery_agent_) {
            discovery_agent_->stop();
            delete discovery_agent_;
        }
        delete discovery_timer_;
    }

    QString extensionId() const override { return extension_id_; }
    bool isValid() const override { return is_valid_; }

    void invalidate() { is_valid_ = false; }

    QStringList listAdapters() const override {
        if (!is_valid_) return {};
        if (local_device_ && local_device_->isValid()) {
            return { local_device_->address().toString() };
        }
        return { QStringLiteral("mock-adapter") };
    }

    QString currentAdapter() const override { return current_adapter_; }

    bool selectAdapter(const QString& adapterId) override {
        if (!is_valid_) return false;
        if (adapterId.isEmpty()) return false;
        // In future we might validate against listAdapters(); for now always accept.
        current_adapter_ = adapterId;
        manager_->logCapabilityUsage(extension_id_, "bluetooth", "selectAdapter", adapterId);
        return true;
    }

    QList<Device> listDevices() const override { return devices_.values(); }

    bool startDiscovery(int timeoutMs) override {
        if (!is_valid_) return false;
        if (!discovery_agent_) {
            discovery_agent_ = new QBluetoothDeviceDiscoveryAgent();
            QObject::connect(discovery_agent_, &QBluetoothDeviceDiscoveryAgent::deviceDiscovered,
                             [this](const QBluetoothDeviceInfo& info) { onDeviceDiscovered(info); });
            QObject::connect(discovery_agent_, &QBluetoothDeviceDiscoveryAgent::finished,
                             [this]() { onDiscoveryFinished(); });
            QObject::connect(discovery_agent_, &QBluetoothDeviceDiscoveryAgent::errorOccurred,
                             [this](QBluetoothDeviceDiscoveryAgent::Error error) {
                                 qWarning() << "Bluetooth discovery error" << error;
                                 onDiscoveryFinished();
                             });
        }
        devices_.clear();
        discovery_agent_->start(QBluetoothDeviceDiscoveryAgent::ClassicMethod);
        manager_->logCapabilityUsage(extension_id_, "bluetooth", "startDiscovery", QString::number(timeoutMs));
        if (timeoutMs <= 0) timeoutMs = 10000; // default 10s
        if (!discovery_timer_) discovery_timer_ = new QTimer();
        discovery_timer_->stop();
        discovery_timer_->setSingleShot(true);
        QObject::connect(discovery_timer_, &QTimer::timeout, [this]() { stopDiscovery(); });
        discovery_timer_->start(timeoutMs);
        return true;
    }

    void stopDiscovery() override {
        if (!is_valid_) return;
        if (discovery_agent_ && discovery_agent_->isActive()) {
            discovery_agent_->stop();
        }
        if (discovery_timer_) discovery_timer_->stop();
        manager_->logCapabilityUsage(extension_id_, "bluetooth", "stopDiscovery", QString());
        notifySubscribers();
    }

    bool pairDevice(const QString& address) override {
        if (!is_valid_) return false;
        if (devices_.contains(address)) {
            auto dev = devices_[address];
            dev.paired = true; // Simulated pairing success
            devices_[address] = dev;
            manager_->logCapabilityUsage(extension_id_, "bluetooth", "pairDevice", address);
            notifySubscribers();
            if (local_device_ && local_device_->isValid()) {
                QBluetoothAddress addr(address);
                local_device_->requestPairing(addr, QBluetoothLocalDevice::AuthorizedPaired);
            }
            return true;
        }
        return false;
    }

    bool connectDevice(const QString& address) override {
        if (!is_valid_) return false;
        if (devices_.contains(address)) {
            auto dev = devices_[address];
            dev.connected = true; // Logical connection
            devices_[address] = dev;
            manager_->logCapabilityUsage(extension_id_, "bluetooth", "connectDevice", address);
            notifySubscribers();
            return true;
        }
        return false;
    }

    bool disconnectDevice(const QString& address) override {
        if (!is_valid_) return false;
        if (devices_.contains(address)) {
            auto dev = devices_[address];
            dev.connected = false;
            devices_[address] = dev;
            manager_->logCapabilityUsage(extension_id_, "bluetooth", "disconnectDevice", address);
            notifySubscribers();
            return true;
        }
        return false;
    }

    int subscribeDevices(std::function<void(const QList<Device>&)> callback) override {
        if (!is_valid_) return -1;
        int id = next_sub_id_++;
        // Use vector of pairs for subscriptions to avoid allocator debug issue with QMap<int,...> under some environments.
        subscriptions_.append(QPair<int, std::function<void(const QList<Device>&)>>(id, callback));
        callback(listDevices());
        manager_->logCapabilityUsage(extension_id_, "bluetooth", "subscribeDevices", QString::number(id));
        return id;
    }

    void unsubscribeDevices(int subscriptionId) override {
        for (int i = 0; i < subscriptions_.size(); ++i) {
            if (subscriptions_[i].first == subscriptionId) {
                subscriptions_.remove(i);
                break;
            }
        }
        manager_->logCapabilityUsage(extension_id_, "bluetooth", "unsubscribeDevices", QString::number(subscriptionId));
    }

private:
    void onDeviceDiscovered(const QBluetoothDeviceInfo& info) {
        Device dev;
        dev.name = info.name();
        dev.address = info.address().toString();
        dev.paired = local_device_ && local_device_->isValid() ? (local_device_->pairingStatus(info.address()) != QBluetoothLocalDevice::Unpaired) : false;
        dev.connected = false; // We do not auto-connect here.
        dev.rssi = info.rssi();
        devices_[dev.address] = dev;
        notifySubscribers();
    }

    void onDiscoveryFinished() {
        manager_->logCapabilityUsage(extension_id_, "bluetooth", "discoveryFinished", QString());
        notifySubscribers();
    }

    void notifySubscribers() {
        QList<Device> list = listDevices();
        for (const auto &entry : subscriptions_) {
            entry.second(list);
        }
    }

    QString extension_id_;
    CapabilityManager* manager_;
    bool is_valid_;
    std::unique_ptr<QBluetoothLocalDevice> local_device_;
    QString current_adapter_;
    QBluetoothDeviceDiscoveryAgent* discovery_agent_;
    QTimer* discovery_timer_;
    QHash<QString, Device> devices_;
    QVector<QPair<int, std::function<void(const QList<Device>&)>>> subscriptions_;
    int next_sub_id_;
};

// Factory helper used by CapabilityManager.
std::shared_ptr<BluetoothCapability> createBluetoothCapabilityInstance(const QString& extensionId, ::opencardev::crankshaft::core::CapabilityManager* manager) {
    return std::make_shared<BluetoothCapabilityImpl>(extensionId, manager);
}

} // namespace capabilities
} // namespace core
} // namespace opencardev::crankshaft
