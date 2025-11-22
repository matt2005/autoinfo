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
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include <QString>
#include <QList>
#include <QPointer>
#include "../core/application/application.hpp"
#include "../core/events/event_bus.hpp"

namespace opencardev::crankshaft { namespace core { class Application; } }

/**
 * BluetoothBridge exposes BluetoothExtension events into QML and converts
 * UI actions into event bus publications expected by the extension.
 */
class BluetoothBridge : public QObject {
    Q_OBJECT
public:
    static BluetoothBridge* instance();
    static void initialise(opencardev::crankshaft::core::Application* app);
    static void registerQmlType();

    Q_INVOKABLE void scan(int timeoutMs);
    Q_INVOKABLE void pair(const QString& address);
    Q_INVOKABLE void connectDevice(const QString& address);
    Q_INVOKABLE void disconnectDevice(const QString& address);
    Q_INVOKABLE void dial(const QString& number);
    Q_INVOKABLE void answerCall();
    Q_INVOKABLE void rejectCall();
    Q_INVOKABLE void endCall();

signals:
    void devicesUpdated(const QVariantList& devices, bool scanning);
    void scanStarted(int timeoutMs);
    void paired(const QString& address, bool paired);
    void connected(const QString& address, bool connected);
    void disconnected(const QString& address);
    void callStatus(bool hasActiveCall, const QString& number, const QString& contactName, bool incoming, bool active);

private:
    explicit BluetoothBridge(QObject* parent = nullptr);
    void subscribeEvents();
    void publish(const QString& name, const QVariantMap& data = {});

    static BluetoothBridge* s_instance_;
    opencardev::crankshaft::core::Application* app_ = nullptr;
    QList<int> subscriptions_;
};
