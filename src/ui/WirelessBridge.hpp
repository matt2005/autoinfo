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

#pragma once

#include <QObject>
#include <QString>
#include <QVariant>
#include <QVariantList>
#include "../core/events/event_bus.hpp"

/**
 * WirelessBridge exposes wireless management to QML.
 * Bridges between QML UI and wireless extension via event bus.
 */
class WirelessBridge : public QObject {
    Q_OBJECT

  public:
    static WirelessBridge* instance();
    static void initialise(opencardev::crankshaft::core::EventBus* eventBus);
    static void registerQmlType();

  public slots:
    void scan();
    void connect(const QString& ssid, const QString& password);
    void disconnect();
    void configureAP(const QString& ssid, const QString& password);
    void forget(const QString& ssid);
    void toggleWifi(bool enabled);

  signals:
    void networksUpdated(const QVariantList& networks);
    void connectionStateChanged(const QString& ssid, bool connected);

  private:
    explicit WirelessBridge(QObject* parent = nullptr);
    void subscribeEvents();

    opencardev::crankshaft::core::EventBus* event_bus_ = nullptr;
    QList<int> subscriptions_;

    static WirelessBridge* instance_;
};
