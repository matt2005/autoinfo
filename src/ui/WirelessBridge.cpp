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

#include "WirelessBridge.hpp"
#include <QDebug>
#include <QQmlEngine>

WirelessBridge* WirelessBridge::instance_ = nullptr;

WirelessBridge::WirelessBridge(QObject* parent) : QObject(parent) {}

WirelessBridge* WirelessBridge::instance() {
    return instance_;
}

void WirelessBridge::initialise(opencardev::crankshaft::core::EventBus* eventBus) {
    if (instance_ == nullptr) {
        instance_ = new WirelessBridge();
        instance_->event_bus_ = eventBus;
        instance_->subscribeEvents();
        qInfo() << "WirelessBridge initialized";
    }
}

void WirelessBridge::registerQmlType() {
    qmlRegisterSingletonType<WirelessBridge>(
        "CrankshaftReborn.Wireless", 1, 0, "WirelessBridge",
        [](QQmlEngine* engine, QJSEngine*) -> QObject* {
            if (instance_ == nullptr) {
                qWarning() << "WirelessBridge instance not created yet!";
                return nullptr;
            }
            QQmlEngine::setObjectOwnership(instance_, QQmlEngine::CppOwnership);
            return instance_;
        });
}

void WirelessBridge::subscribeEvents() {
    if (!event_bus_) {
        qWarning() << "WirelessBridge: No event bus available";
        return;
    }

    // Subscribe to networks_updated event from wireless extension
    int subId = event_bus_->subscribe("wireless.networks_updated", [this](const QVariantMap& data) {
        QVariantList networks = data.value("networks").toList();
        emit networksUpdated(networks);
    });
    subscriptions_.append(subId);

    // Subscribe to connection_state_changed event
    subId =
        event_bus_->subscribe("wireless.connection_state_changed", [this](const QVariantMap& data) {
            QString ssid = data.value("ssid").toString();
            bool connected = data.value("connected").toBool();
            emit connectionStateChanged(ssid, connected);
        });
    subscriptions_.append(subId);

    qInfo() << "WirelessBridge: Subscribed to wireless events";
}

void WirelessBridge::scan() {
    if (!event_bus_)
        return;

    QVariantMap data;
    event_bus_->publish("wireless.scan", data);
}

void WirelessBridge::connect(const QString& ssid, const QString& password) {
    if (!event_bus_)
        return;

    QVariantMap data;
    data["ssid"] = ssid;
    data["password"] = password;
    event_bus_->publish("wireless.connect", data);
}

void WirelessBridge::disconnect() {
    if (!event_bus_)
        return;

    QVariantMap data;
    event_bus_->publish("wireless.disconnect", data);
}

void WirelessBridge::configureAP(const QString& ssid, const QString& password) {
    if (!event_bus_)
        return;

    QVariantMap data;
    data["ssid"] = ssid;
    data["password"] = password;
    event_bus_->publish("wireless.access_point", data);
}

void WirelessBridge::forget(const QString& ssid) {
    if (!event_bus_)
        return;

    QVariantMap data;
    data["ssid"] = ssid;
    event_bus_->publish("wireless.forget", data);
}

void WirelessBridge::toggleWifi(bool enabled) {
    if (!event_bus_)
        return;

    QVariantMap data;
    data["enabled"] = enabled;
    event_bus_->publish("wireless.toggle", data);
}
