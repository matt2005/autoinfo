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

#include "EventBridge.hpp"
#include <QDebug>
#include "../core/events/event_bus.hpp"

namespace opencardev::crankshaft {
namespace ui {

EventBridge* EventBridge::instance_ = nullptr;
core::EventBus* EventBridge::event_bus_ = nullptr;

EventBridge::EventBridge(QObject* parent) : QObject(parent) {
    if (!instance_)
        instance_ = this;
}

void EventBridge::registerQmlType() {
    qmlRegisterSingletonType<EventBridge>("CrankshaftReborn.Events", 1, 0, "EventBridge",
                                          EventBridge::qmlInstance);
}

QObject* EventBridge::qmlInstance(QQmlEngine* engine, QJSEngine* scriptEngine) {
    Q_UNUSED(engine);
    Q_UNUSED(scriptEngine);
    if (!instance_)
        instance_ = new EventBridge();
    return instance_;
}

void EventBridge::initialise(core::EventBus* bus) {
    event_bus_ = bus;
}

void EventBridge::publish(const QString& topic, const QVariantMap& data) {
    if (!event_bus_) {
        qWarning() << "EventBridge: EventBus not initialised; cannot publish";
        return;
    }
    event_bus_->publish(topic, data);
}

void EventBridge::emitNamespaced(const QString& extensionId, const QString& name,
                                 const QVariantMap& data) {
    if (!event_bus_) {
        qWarning() << "EventBridge: EventBus not initialised; cannot emit";
        return;
    }
    const QString full = extensionId.isEmpty() ? name : (extensionId + "." + name);
    event_bus_->publish(full, data);
}

}  // namespace ui
}  // namespace opencardev::crankshaft
