/*
 * Project: OpenAuto
 * This file is part of openauto project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  openauto is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  openauto is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with openauto. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "../../src/extensions/extension.hpp"
#include <QString>
#include <QVector>

namespace openauto {
namespace extensions {
namespace bluetooth {

struct BluetoothDevice {
    QString address;
    QString name;
    bool paired;
    bool connected;
    int signalStrength;
};

struct PhoneCall {
    QString number;
    QString contactName;
    bool incoming;
    bool active;
};

class BluetoothExtension : public Extension {
public:
    BluetoothExtension() = default;
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

    QVector<BluetoothDevice> devices_;
    BluetoothDevice* connectedDevice_;
    PhoneCall* activeCall_;
    bool scanning_;
};

}  // namespace bluetooth
}  // namespace extensions
}  // namespace openauto
