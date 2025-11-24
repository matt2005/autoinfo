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

#include "bluetooth_extension.hpp"
#include <QDebug>
#include <QVariantMap>
#include "../../src/core/config/ConfigManager.hpp"
#include "../../src/core/config/ConfigTypes.hpp"

namespace opencardev::crankshaft {
namespace extensions {
namespace bluetooth {

bool BluetoothExtension::initialize() {
    qInfo() << "Initializing Bluetooth extension (capability-driven)...";

    // Acquire capabilities granted by manager.
    btCap_ = getCapability<core::capabilities::BluetoothCapability>();
    eventCap_ = getCapability<core::capabilities::EventCapability>();

    if (!btCap_) {
        qWarning() << "Bluetooth capability not granted; extension will be disabled.";
        return false;
    }
    if (!eventCap_) {
        qWarning() << "Event capability not granted; cannot communicate.";
        return false;
    }

    scanning_ = false;
    activeCall_ = nullptr;
    currentAdapter_ = btCap_->currentAdapter();

    // Subscribe to device updates from capability
    deviceSubscriptionId_ = btCap_->subscribeDevices(
        [this](const QList<core::capabilities::BluetoothCapability::Device>& list) {
            handleDevicesUpdated(list);
        });

    // Subscribe to command events emitted in our namespace.
    subscribeCommandEvents();
    return true;
}

void BluetoothExtension::start() {
    qInfo() << "Starting Bluetooth extension";
    publishDeviceList();
}

void BluetoothExtension::stop() {
    qInfo() << "Stopping Bluetooth extension";
    if (btCap_ && deviceSubscriptionId_ >= 0) {
        btCap_->unsubscribeDevices(deviceSubscriptionId_);
        deviceSubscriptionId_ = -1;
    }
    scanning_ = false;
}

void BluetoothExtension::cleanup() {
    qInfo() << "Cleaning up Bluetooth extension";
    if (activeCall_) {
        delete activeCall_;
        activeCall_ = nullptr;
    }
    btCap_.reset();
    eventCap_.reset();
}

void BluetoothExtension::registerConfigItems(core::config::ConfigManager* manager) {
    using namespace core::config;

    ConfigPage page;
    page.domain = "connectivity";
    page.extension = "bluetooth";
    page.title = "Bluetooth Settings";
    page.description = "Configure Bluetooth connectivity and pairing options";
    page.icon = "qrc:/icons/bluetooth.svg";

    // Connection Settings Section
    ConfigSection connectionSection;
    connectionSection.key = "connection";
    connectionSection.title = "Connection Settings";
    connectionSection.description = "Manage Bluetooth connection behavior";
    connectionSection.complexity = ConfigComplexity::Basic;

    ConfigItem autoConnectItem;
    autoConnectItem.key = "auto_connect";
    autoConnectItem.label = "Auto-connect to devices";
    autoConnectItem.description = "Automatically connect to known devices when in range";
    autoConnectItem.type = ConfigItemType::Boolean;
    autoConnectItem.defaultValue = true;
    autoConnectItem.complexity = ConfigComplexity::Basic;
    autoConnectItem.required = false;
    autoConnectItem.readOnly = false;
    connectionSection.items.append(autoConnectItem);

    ConfigItem reconnectDelayItem;
    reconnectDelayItem.key = "reconnect_delay";
    reconnectDelayItem.label = "Reconnection delay";
    reconnectDelayItem.description = "Time to wait before attempting reconnection";
    reconnectDelayItem.type = ConfigItemType::Integer;
    reconnectDelayItem.defaultValue = 5;
    reconnectDelayItem.properties["minValue"] = 1;
    reconnectDelayItem.properties["maxValue"] = 60;
    reconnectDelayItem.unit = "seconds";
    reconnectDelayItem.complexity = ConfigComplexity::Advanced;
    reconnectDelayItem.required = false;
    reconnectDelayItem.readOnly = false;
    connectionSection.items.append(reconnectDelayItem);

    ConfigItem visibilityItem;
    visibilityItem.key = "visibility";
    visibilityItem.label = "Visibility mode";
    visibilityItem.description = "Bluetooth visibility mode";
    visibilityItem.type = ConfigItemType::Selection;
    visibilityItem.properties["options"] = QStringList{"Hidden", "Visible", "Discoverable"};
    visibilityItem.defaultValue = "Visible";
    visibilityItem.complexity = ConfigComplexity::Basic;
    visibilityItem.required = false;
    visibilityItem.readOnly = false;
    connectionSection.items.append(visibilityItem);

    page.sections.append(connectionSection);

    // Audio Settings Section
    ConfigSection audioSection;
    audioSection.key = "audio";
    audioSection.title = "Audio Settings";
    audioSection.description = "Configure Bluetooth audio quality and codecs";
    audioSection.complexity = ConfigComplexity::Advanced;

    ConfigItem audioCodecItem;
    audioCodecItem.key = "audio_codec";
    audioCodecItem.label = "Preferred audio codec";
    audioCodecItem.description = "Select the preferred audio codec for Bluetooth audio";
    audioCodecItem.type = ConfigItemType::Selection;
    audioCodecItem.properties["options"] = QStringList{"SBC", "AAC", "aptX", "aptX HD", "LDAC"};
    audioCodecItem.defaultValue = "AAC";
    audioCodecItem.complexity = ConfigComplexity::Advanced;
    audioCodecItem.required = false;
    audioCodecItem.readOnly = false;
    audioSection.items.append(audioCodecItem);

    ConfigItem bitrateItem;
    bitrateItem.key = "bitrate";
    bitrateItem.label = "Audio bitrate";
    bitrateItem.description = "Maximum bitrate for Bluetooth audio streaming";
    bitrateItem.type = ConfigItemType::Integer;
    bitrateItem.defaultValue = 320;
    bitrateItem.properties["minValue"] = 128;
    bitrateItem.properties["maxValue"] = 990;
    bitrateItem.properties["step"] = 16;
    bitrateItem.unit = "kbps";
    bitrateItem.complexity = ConfigComplexity::Expert;
    bitrateItem.required = false;
    bitrateItem.readOnly = false;
    audioSection.items.append(bitrateItem);

    page.sections.append(audioSection);

    // Phone Settings Section
    ConfigSection phoneSection;
    phoneSection.key = "phone";
    phoneSection.title = "Phone Settings";
    phoneSection.description = "Configure hands-free phone functionality";
    phoneSection.complexity = ConfigComplexity::Basic;

    ConfigItem autoAnswerItem;
    autoAnswerItem.key = "auto_answer";
    autoAnswerItem.label = "Auto-answer calls";
    autoAnswerItem.description = "Automatically answer incoming calls after specified delay";
    autoAnswerItem.type = ConfigItemType::Boolean;
    autoAnswerItem.defaultValue = false;
    autoAnswerItem.complexity = ConfigComplexity::Basic;
    autoAnswerItem.required = false;
    autoAnswerItem.readOnly = false;
    phoneSection.items.append(autoAnswerItem);

    ConfigItem autoAnswerDelayItem;
    autoAnswerDelayItem.key = "auto_answer_delay";
    autoAnswerDelayItem.label = "Auto-answer delay";
    autoAnswerDelayItem.description = "Delay before auto-answering incoming calls";
    autoAnswerDelayItem.type = ConfigItemType::Integer;
    autoAnswerDelayItem.defaultValue = 0;
    autoAnswerDelayItem.properties["minValue"] = 0;
    autoAnswerDelayItem.properties["maxValue"] = 10;
    autoAnswerDelayItem.unit = "seconds";
    autoAnswerDelayItem.complexity = ConfigComplexity::Basic;
    autoAnswerDelayItem.required = false;
    autoAnswerDelayItem.readOnly = false;
    phoneSection.items.append(autoAnswerDelayItem);

    page.sections.append(phoneSection);

    // Register the page
    manager->registerConfigPage(page);
    qInfo() << "Bluetooth extension registered config items";
}

void BluetoothExtension::setupEventHandlers() {}

void BluetoothExtension::subscribeCommandEvents() {
    if (!eventCap_)
        return;
    // Use fully-qualified event names matching EventCapability emission model.
    eventCap_->subscribe("bluetooth.scan",
                         [this](const QVariantMap& data) { handleScanCommand(data); });
    eventCap_->subscribe("bluetooth.pair",
                         [this](const QVariantMap& data) { handlePairCommand(data); });
    eventCap_->subscribe("bluetooth.connect",
                         [this](const QVariantMap& data) { handleConnectCommand(data); });
    eventCap_->subscribe("bluetooth.disconnect",
                         [this](const QVariantMap& data) { handleDisconnectCommand(data); });
    eventCap_->subscribe("bluetooth.answerCall",
                         [this](const QVariantMap& data) { handleAnswerCallCommand(data); });
    eventCap_->subscribe("bluetooth.rejectCall",
                         [this](const QVariantMap& data) { handleRejectCallCommand(data); });
    eventCap_->subscribe("bluetooth.endCall",
                         [this](const QVariantMap& data) { handleEndCallCommand(data); });
    eventCap_->subscribe("bluetooth.dial",
                         [this](const QVariantMap& data) { handleDialCommand(data); });

    // Public dial events from any extension (e.g., "dialer.phone.dial")
    eventCap_->subscribe("*.phone.dial",
                         [this](const QVariantMap& data) { handleDialCommand(data); });
}

void BluetoothExtension::handleScanCommand(const QVariantMap& data) {
    if (!btCap_ || !eventCap_)
        return;
    int timeoutMs = data.value("timeoutMs").toInt();
    scanning_ = true;
    QVariantMap started;
    started["scanning"] = true;
    started["timeoutMs"] = timeoutMs;
    eventCap_->emitEvent("scan_started", started);
    btCap_->startDiscovery(timeoutMs);
}

void BluetoothExtension::handlePairCommand(const QVariantMap& data) {
    if (!btCap_ || !eventCap_)
        return;
    QString addr = data.value("address").toString();
    bool ok = btCap_->pairDevice(addr);
    QVariantMap ev;
    ev["address"] = addr;
    ev["paired"] = ok;
    eventCap_->emitEvent("paired", ev);
}

void BluetoothExtension::handleConnectCommand(const QVariantMap& data) {
    if (!btCap_ || !eventCap_)
        return;
    QString addr = data.value("address").toString();
    bool ok = btCap_->connectDevice(addr);
    QVariantMap ev;
    ev["address"] = addr;
    ev["connected"] = ok;
    eventCap_->emitEvent("connected", ev);
}

void BluetoothExtension::handleDisconnectCommand(const QVariantMap& data) {
    if (!btCap_ || !eventCap_)
        return;
    QString addr = data.value("address").toString();
    bool ok = btCap_->disconnectDevice(addr);
    QVariantMap ev;
    ev["address"] = addr;
    ev["connected"] = !ok ? true : false;  // if disconnect succeeded connected=false
    ev["connected"] = false;
    eventCap_->emitEvent("disconnected", ev);
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

void BluetoothExtension::handleDevicesUpdated(
    const QList<core::capabilities::BluetoothCapability::Device>& list) {
    Q_UNUSED(list);  // Publish through publishDeviceList
    publishDeviceList();
}

void BluetoothExtension::publishDeviceList() {
    if (!eventCap_ || !btCap_)
        return;
    QVariantList deviceList;
    for (const auto& d : btCap_->listDevices()) {
        QVariantMap m;
        m["address"] = d.address;
        m["name"] = d.name;
        m["paired"] = d.paired;
        m["connected"] = d.connected;
        m["rssi"] = d.rssi;
        deviceList.append(m);
    }
    QVariantMap ev;
    ev["devices"] = deviceList;
    ev["scanning"] = scanning_;
    eventCap_->emitEvent("devices_updated", ev);
}

void BluetoothExtension::publishCallStatus() {
    if (!eventCap_)
        return;
    QVariantMap ev;
    if (activeCall_) {
        ev["hasActiveCall"] = true;
        ev["number"] = activeCall_->number;
        ev["contactName"] = activeCall_->contactName;
        ev["incoming"] = activeCall_->incoming;
        ev["active"] = activeCall_->active;
    } else {
        ev["hasActiveCall"] = false;
    }
    eventCap_->emitEvent("call_status", ev);
}

}  // namespace bluetooth
}  // namespace extensions
}  // namespace opencardev::crankshaft
