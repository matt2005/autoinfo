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

#include "NavigationBridge.hpp"
#include "../core/CapabilityManager.hpp"
#include <QQmlEngine>
#include <QJSEngine>

static NavigationBridge* s_instance = nullptr;

NavigationBridge::NavigationBridge(QObject* parent) : QObject(parent) {
    // Determine settings path (relative config folder preferred)
    QString base = QDir::currentPath() + "/config";
    if (!QDir(base).exists()) {
        QDir().mkpath(base);
    }
    settingsPath_ = base + "/navigation_settings.json";
    load();
}

NavigationBridge* NavigationBridge::instance() {
    if (!s_instance) {
        s_instance = new NavigationBridge();
    }
    return s_instance;
}

void NavigationBridge::initialise(openauto::core::CapabilityManager* capabilityManager) {
    instance()->capability_manager_ = capabilityManager;
    instance()->applyToCapability();
}

void NavigationBridge::registerQmlType() {
    qmlRegisterSingletonType<NavigationBridge>("CrankshaftReborn.Navigation", 1, 0, "NavigationBridge",
        [](QQmlEngine* engine, QJSEngine* scriptEngine) -> QObject* {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return NavigationBridge::instance();
        }
    );
}

void NavigationBridge::load() {
    QFile f(settingsPath_);
    if (f.exists() && f.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(f.readAll());
        if (doc.isObject()) {
            gpsDevice_ = doc.object().value("gpsDevice").toString(gpsDevice_);
        }
    }
}

void NavigationBridge::save() {
    QJsonObject obj;
    obj["gpsDevice"] = gpsDevice_;
    QFile f(settingsPath_);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
        f.close();
    }
}

void NavigationBridge::applyToCapability() {
    if (!capability_manager_) return;
    auto locCap = capability_manager_->getLocationCapability("navigation");
    if (!locCap) return;
    using DM = openauto::core::capabilities::LocationCapability::DeviceMode;
    DM mode = DM::Internal;
    if (gpsDevice_ == "USB Receiver") mode = DM::USB;
    else if (gpsDevice_ == "GNSS Hat") mode = DM::Hat;
    else if (gpsDevice_.startsWith("Mock") && gpsDevice_.contains("Static")) mode = DM::MockStatic;
    else if (gpsDevice_.startsWith("Mock") && gpsDevice_.contains("IP")) mode = DM::MockIP;
    locCap->setDeviceMode(mode);
    qInfo() << "NavigationBridge applied GPS device:" << gpsDevice_;
}

void NavigationBridge::setGpsDevice(const QString& device) {
    if (device == gpsDevice_) return;
    gpsDevice_ = device;
    save();
    applyToCapability();
    emit gpsDeviceChanged();
}
