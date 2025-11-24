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

#include <QJSEngine>
#include <QObject>
#include <QQmlEngine>
#include <QVariantMap>

namespace opencardev::crankshaft {
namespace core {
class EventBus;
}
namespace ui {

/**
 * EventBridge exposes a minimal, generic event publish interface to QML.
 *
 * It is a QML singleton (CrankshaftReborn.Events/EventBridge), initialised
 * with the core EventBus in main.cpp. This is intended for UI interactions
 * where lightweight, cross-extension signals are useful.
 *
 * Security: This bridge provides raw publish(). In future we can restrict
 * by caller/namespace if needed. For now, use public topics e.g. "*.phone.dial".
 */
class EventBridge : public QObject {
    Q_OBJECT
  public:
    explicit EventBridge(QObject* parent = nullptr);
    ~EventBridge() override = default;

    static void registerQmlType();
    static QObject* qmlInstance(QQmlEngine* engine, QJSEngine* scriptEngine);

    // Wire the global event bus (call once at startup)
    static void initialise(core::EventBus* bus);

    // Publish an event to the bus (topic can be exact or wildcard-friendly)
    Q_INVOKABLE void publish(const QString& topic, const QVariantMap& data = QVariantMap());

    // Helper for common case: emit namespaced event name = extId + "." + name
    Q_INVOKABLE void emitNamespaced(const QString& extensionId, const QString& name,
                                    const QVariantMap& data = QVariantMap());

  private:
    static EventBridge* instance_;
    static core::EventBus* event_bus_;
};

}  // namespace ui
}  // namespace opencardev::crankshaft
