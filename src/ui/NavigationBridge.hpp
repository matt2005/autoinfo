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

#include <QObject>
#include <QString>
#include <QVariant>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include "../core/capabilities/LocationCapability.hpp"
class openauto_capability_manager_forward_decl;

namespace openauto {
namespace core { class CapabilityManager; }
}

/**
 * NavigationBridge exposes persistent navigation-related settings to QML
 * and mediates GPS device selection into the LocationCapability, including
 * mock device modes for development.
 */
class NavigationBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString gpsDevice READ gpsDevice WRITE setGpsDevice NOTIFY gpsDeviceChanged)
public:
    static NavigationBridge* instance();

    static void initialise(openauto::core::CapabilityManager* capabilityManager);
    static void registerQmlType();

    QString gpsDevice() const { return gpsDevice_; }

public slots:
    void setGpsDevice(const QString& device);

signals:
    void gpsDeviceChanged();

private:
    explicit NavigationBridge(QObject* parent = nullptr);
    void load();
    void save();
    void applyToCapability();
    openauto::core::CapabilityManager* capability_manager_ = nullptr;
    QString gpsDevice_ = "Internal";
    QString settingsPath_;
};
