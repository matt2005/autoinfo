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

#include "BluetoothBridge.hpp"
#include <QQmlEngine>
#include <QJSEngine>
#include <QDebug>
#include "../core/application/application.hpp"

BluetoothBridge* BluetoothBridge::s_instance_ = nullptr;

BluetoothBridge::BluetoothBridge(QObject* parent) : QObject(parent) {}

BluetoothBridge* BluetoothBridge::instance() {
    if (!s_instance_) {
        s_instance_ = new BluetoothBridge();
    }
    return s_instance_;
}

void BluetoothBridge::initialise(opencardev::crankshaft::core::Application* app) {
    instance()->app_ = app;
    instance()->subscribeEvents();
}

void BluetoothBridge::registerQmlType() {
    qmlRegisterSingletonType<BluetoothBridge>("CrankshaftReborn.Bluetooth", 1, 0, "BluetoothBridge",
        [](QQmlEngine* engine, QJSEngine* scriptEngine) -> QObject* {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return BluetoothBridge::instance();
        }
    );
}

void BluetoothBridge::publish(const QString& name, const QVariantMap& data) {
    if (!app_ || !app_->eventBus()) return;
    QString full = QStringLiteral("bluetooth.") + name;
    app_->eventBus()->publish(full, data);
}

void BluetoothBridge::scan(int timeoutMs) {
    QVariantMap data;
    data["timeoutMs"] = timeoutMs;
    publish("scan", data);
}

void BluetoothBridge::pair(const QString& address) {
    QVariantMap data;
    data["address"] = address;
    publish("pair", data);
}

void BluetoothBridge::connectDevice(const QString& address) {
    QVariantMap data;
    data["address"] = address;
    publish("connect", data);
}

void BluetoothBridge::disconnectDevice(const QString& address) {
    QVariantMap data;
    data["address"] = address;
    publish("disconnect", data);
}

void BluetoothBridge::dial(const QString& number) {
    QVariantMap data;
    data["number"] = number;
    publish("dial", data);
}

void BluetoothBridge::answerCall() { publish("answerCall", QVariantMap()); }
void BluetoothBridge::rejectCall() { publish("rejectCall", QVariantMap()); }
void BluetoothBridge::endCall() { publish("endCall", QVariantMap()); }

void BluetoothBridge::subscribeEvents() {
    if (!app_ || !app_->eventBus()) return;
    auto bus = app_->eventBus();
    // Subscribe to events emitted by BluetoothExtension via EventCapability
    subscriptions_.append(bus->subscribe("bluetooth.devices_updated", [this](const QVariantMap& data){
        QVariantList list = data.value("devices").toList();
        bool scanning = data.value("scanning").toBool();
        emit devicesUpdated(list, scanning);
    }));
    subscriptions_.append(bus->subscribe("bluetooth.scan_started", [this](const QVariantMap& data){
        emit scanStarted(data.value("timeoutMs").toInt());
    }));
    subscriptions_.append(bus->subscribe("bluetooth.paired", [this](const QVariantMap& data){
        emit paired(data.value("address").toString(), data.value("paired").toBool());
    }));
    subscriptions_.append(bus->subscribe("bluetooth.connected", [this](const QVariantMap& data){
        emit connected(data.value("address").toString(), data.value("connected").toBool());
    }));
    subscriptions_.append(bus->subscribe("bluetooth.disconnected", [this](const QVariantMap& data){
        emit disconnected(data.value("address").toString());
    }));
    subscriptions_.append(bus->subscribe("bluetooth.call_status", [this](const QVariantMap& data){
        emit callStatus(data.value("hasActiveCall").toBool(),
                        data.value("number").toString(),
                        data.value("contactName").toString(),
                        data.value("incoming").toBool(),
                        data.value("active").toBool());
    }));
}
