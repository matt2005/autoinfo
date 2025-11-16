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

#include "bluetooth_extension.hpp"
#include "../../src/core/event_bus.hpp"
#include <QDebug>

namespace openauto {
namespace extensions {
namespace bluetooth {

bool BluetoothExtension::initialize() {
    qInfo() << "Initializing Bluetooth extension...";
    scanning_ = false;
    connectedDevice_ = nullptr;
    activeCall_ = nullptr;
    setupEventHandlers();
    return true;
}

void BluetoothExtension::start() {
    qInfo() << "Starting Bluetooth extension...";
}

void BluetoothExtension::stop() {
    qInfo() << "Stopping Bluetooth extension...";
    scanning_ = false;
}

void BluetoothExtension::cleanup() {
    qInfo() << "Cleaning up Bluetooth extension...";
    devices_.clear();
    connectedDevice_ = nullptr;
    if (activeCall_) {
        delete activeCall_;
        activeCall_ = nullptr;
    }
}

void BluetoothExtension::setupEventHandlers() {
    if (!event_bus_) {
        qWarning() << "Event bus not available";
        return;
    }

    event_bus_->subscribe("bluetooth.scan", [this](const QVariantMap& data) {
        handleScanCommand(data);
    });

    event_bus_->subscribe("bluetooth.pair", [this](const QVariantMap& data) {
        handlePairCommand(data);
    });

    event_bus_->subscribe("bluetooth.connect", [this](const QVariantMap& data) {
        handleConnectCommand(data);
    });

    event_bus_->subscribe("bluetooth.disconnect", [this](const QVariantMap& data) {
        handleDisconnectCommand(data);
    });

    event_bus_->subscribe("bluetooth.answerCall", [this](const QVariantMap& data) {
        handleAnswerCallCommand(data);
    });

    event_bus_->subscribe("bluetooth.rejectCall", [this](const QVariantMap& data) {
        handleRejectCallCommand(data);
    });

    event_bus_->subscribe("bluetooth.endCall", [this](const QVariantMap& data) {
        handleEndCallCommand(data);
    });

    event_bus_->subscribe("bluetooth.dial", [this](const QVariantMap& data) {
        handleDialCommand(data);
    });
}

void BluetoothExtension::handleScanCommand(const QVariantMap& data) {
    Q_UNUSED(data);
    qDebug() << "Bluetooth scan command received";

    scanning_ = true;

    QVariantMap event;
    event["scanning"] = true;
    event_bus_->publish("bluetooth.scanStarted", event);

    // TODO: Implement actual Bluetooth scanning
    // For now, simulate finding some devices
    qInfo() << "Starting Bluetooth device scan...";

    // Simulate scan completion
    scanning_ = false;
    event["scanning"] = false;
    event_bus_->publish("bluetooth.scanCompleted", event);

    publishDeviceList();
}

void BluetoothExtension::handlePairCommand(const QVariantMap& data) {
    QString deviceAddress = data.value("address").toString();
    qDebug() << "Pair command received for device:" << deviceAddress;

    // TODO: Implement actual Bluetooth pairing
    qInfo() << "Pairing with device:" << deviceAddress;

    QVariantMap event;
    event["address"] = deviceAddress;
    event["paired"] = true;
    event_bus_->publish("bluetooth.paired", event);
}

void BluetoothExtension::handleConnectCommand(const QVariantMap& data) {
    QString deviceAddress = data.value("address").toString();
    qDebug() << "Connect command received for device:" << deviceAddress;

    // TODO: Implement actual Bluetooth connection
    qInfo() << "Connecting to device:" << deviceAddress;

    QVariantMap event;
    event["address"] = deviceAddress;
    event["connected"] = true;
    event_bus_->publish("bluetooth.connected", event);
}

void BluetoothExtension::handleDisconnectCommand(const QVariantMap& data) {
    QString deviceAddress = data.value("address").toString();
    qDebug() << "Disconnect command received for device:" << deviceAddress;

    connectedDevice_ = nullptr;

    QVariantMap event;
    event["address"] = deviceAddress;
    event["connected"] = false;
    event_bus_->publish("bluetooth.disconnected", event);
}

void BluetoothExtension::handleAnswerCallCommand(const QVariantMap& data) {
    Q_UNUSED(data);
    qDebug() << "Answer call command received";

    if (activeCall_) {
        activeCall_->active = true;
        activeCall_->incoming = false;

        qInfo() << "Answering call from:" << activeCall_->number;
        publishCallStatus();
    }
}

void BluetoothExtension::handleRejectCallCommand(const QVariantMap& data) {
    Q_UNUSED(data);
    qDebug() << "Reject call command received";

    if (activeCall_) {
        qInfo() << "Rejecting call from:" << activeCall_->number;

        delete activeCall_;
        activeCall_ = nullptr;

        publishCallStatus();
    }
}

void BluetoothExtension::handleEndCallCommand(const QVariantMap& data) {
    Q_UNUSED(data);
    qDebug() << "End call command received";

    if (activeCall_) {
        qInfo() << "Ending call with:" << activeCall_->number;

        delete activeCall_;
        activeCall_ = nullptr;

        publishCallStatus();
    }
}

void BluetoothExtension::handleDialCommand(const QVariantMap& data) {
    QString number = data.value("number").toString();
    qDebug() << "Dial command received:" << number;

    // TODO: Implement actual phone dialing
    qInfo() << "Dialing number:" << number;

    if (activeCall_) {
        delete activeCall_;
    }

    activeCall_ = new PhoneCall{number, "", false, true};

    publishCallStatus();
}

void BluetoothExtension::publishDeviceList() {
    if (!event_bus_) {
        return;
    }

    QVariantList deviceList;
    for (const auto& device : devices_) {
        QVariantMap deviceMap;
        deviceMap["address"] = device.address;
        deviceMap["name"] = device.name;
        deviceMap["paired"] = device.paired;
        deviceMap["connected"] = device.connected;
        deviceMap["signalStrength"] = device.signalStrength;
        deviceList.append(deviceMap);
    }

    QVariantMap event;
    event["devices"] = deviceList;
    event_bus_->publish("bluetooth.deviceList", event);
}

void BluetoothExtension::publishCallStatus() {
    if (!event_bus_) {
        return;
    }

    QVariantMap event;
    if (activeCall_) {
        event["hasActiveCall"] = true;
        event["number"] = activeCall_->number;
        event["contactName"] = activeCall_->contactName;
        event["incoming"] = activeCall_->incoming;
        event["active"] = activeCall_->active;
    } else {
        event["hasActiveCall"] = false;
    }

    event_bus_->publish("bluetooth.callStatus", event);
}

}  // namespace bluetooth
}  // namespace extensions
}  // namespace openauto
