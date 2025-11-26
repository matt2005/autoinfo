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

#include <QString>
#include <QVector>
#include <memory>
#include "../../src/core/capabilities/BluetoothCapability.hpp"
#include "../../src/core/capabilities/EventCapability.hpp"
#include "../../src/extensions/extension.hpp"

namespace opencardev::crankshaft {
namespace extensions {
namespace bluetooth {

struct PhoneCall {
    QString number;
    QString contactName;
    bool incoming;
    bool active;
};

class BluetoothExtension : public Extension {
  public:
    BluetoothExtension() : scanning_(false), activeCall_(false) {}
    ~BluetoothExtension() override = default;

    // Lifecycle methods
    bool initialize() override;
    void start() override;
    void stop() override;
    void cleanup() override;

    // Metadata
    QString id() const override { return "bluetooth"; }
    QString name() const override { return "Bluetooth Manager"; }
    QString version() const override { return "1.0.0"; }
    ExtensionType type() const override { return ExtensionType::Service; }

    // Configuration
    void registerConfigItems(core::config::ConfigManager* manager) override;

  private:
    void setupEventHandlers();
    void handleScanCommand(const QVariantMap& data);
    void handlePairCommand(const QVariantMap& data);
    void handleConnectCommand(const QVariantMap& data);
    void handleDisconnectCommand(const QVariantMap& data);
    void handleAnswerCallCommand(const QVariantMap& data);
    void handleRejectCallCommand(const QVariantMap& data);
    void handleEndCallCommand(const QVariantMap& data);
    void handleDialCommand(const QVariantMap& data);
    void publishDeviceList();
    void publishCallStatus();
    void subscribeCommandEvents();
    void handleDevicesUpdated(const QList<core::capabilities::BluetoothCapability::Device>& list);

    std::shared_ptr<core::capabilities::BluetoothCapability> btCap_;
    std::shared_ptr<core::capabilities::EventCapability> eventCap_;
    int deviceSubscriptionId_ = -1;
    QString currentAdapter_;
    bool scanning_;
    // Legacy simulated call state retained
    core::capabilities::BluetoothCapability::Device connectedDevice_{};
    PhoneCall* activeCall_;
};

}  // namespace bluetooth
}  // namespace extensions
}  // namespace opencardev::crankshaft
