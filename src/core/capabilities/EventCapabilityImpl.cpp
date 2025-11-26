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
#include "EventCapabilityImpl.hpp"
#include <QDebug>
#include "../events/event_bus.hpp"
#include "CapabilityManager.hpp"

using namespace opencardev::crankshaft::core::capabilities;
using opencardev::crankshaft::core::CapabilityManager;
using opencardev::crankshaft::core::EventBus;

EventCapabilityImpl::EventCapabilityImpl(const QString& extension_id, CapabilityManager* manager,
                                         EventBus* event_bus)
    : extension_id_(extension_id),
      manager_(manager),
      event_bus_(event_bus),
      is_valid_(true),
      next_subscription_id_(1) {}

QString EventCapabilityImpl::extensionId() const {
    return extension_id_;
}
bool EventCapabilityImpl::isValid() const {
    return is_valid_;
}
void EventCapabilityImpl::invalidate() {
    is_valid_ = false;
    for (auto subId : subscriptions_.keys()) {
        event_bus_->unsubscribe(subId);
    }
    subscriptions_.clear();
}

bool EventCapabilityImpl::emitEvent(const QString& eventName, const QVariantMap& eventData) {
    if (!is_valid_ || !event_bus_)
        return false;
    QString fullEventName = extension_id_ + "." + eventName;
    manager_->logCapabilityUsage(extension_id_, "event", "emit", fullEventName);
    event_bus_->publish(fullEventName, eventData);
    return true;
}

int EventCapabilityImpl::subscribe(const QString& eventPattern,
                                   std::function<void(const QVariantMap&)> callback) {
    if (!is_valid_ || !event_bus_)
        return -1;
    if (!canSubscribe(eventPattern)) {
        qWarning() << "Extension" << extension_id_ << "denied subscription to" << eventPattern;
        return -1;
    }
    int localId = next_subscription_id_++;
    int busId = event_bus_->subscribe(eventPattern, callback);
    subscriptions_[localId] = busId;
    manager_->logCapabilityUsage(extension_id_, "event", "subscribe", eventPattern);
    return localId;
}

void EventCapabilityImpl::unsubscribe(int subscriptionId) {
    if (!is_valid_ || !event_bus_)
        return;
    if (subscriptions_.contains(subscriptionId)) {
        event_bus_->unsubscribe(subscriptions_[subscriptionId]);
        subscriptions_.remove(subscriptionId);
        manager_->logCapabilityUsage(extension_id_, "event", "unsubscribe",
                                     QString::number(subscriptionId));
    }
}

bool EventCapabilityImpl::canEmit(const QString& eventName) const {
    return eventName.startsWith(extension_id_ + ".");
}

bool EventCapabilityImpl::canSubscribe(const QString& eventPattern) const {
    if (eventPattern.startsWith(extension_id_ + "."))
        return true;
    if (eventPattern.startsWith("core."))
        return true;
    if (eventPattern == "*" || eventPattern.startsWith("*."))
        return true;
    return false;
}
