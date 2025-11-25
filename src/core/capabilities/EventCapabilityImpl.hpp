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

#include "EventCapability.hpp"
#include <QMap>

namespace opencardev::crankshaft::core { class CapabilityManager; class EventBus; }

namespace opencardev::crankshaft::core::capabilities {

class EventCapabilityImpl : public EventCapability {
  public:
    EventCapabilityImpl(const QString& extension_id, core::CapabilityManager* manager, core::EventBus* event_bus);
    QString extensionId() const override;
    bool isValid() const override;
    void invalidate();
    bool emitEvent(const QString& eventName, const QVariantMap& eventData) override;
    int subscribe(const QString& eventPattern, std::function<void(const QVariantMap&)> callback) override;
    void unsubscribe(int subscriptionId) override;
    bool canEmit(const QString& eventName) const override;
    bool canSubscribe(const QString& eventPattern) const override;
  private:
    QString extension_id_;
    core::CapabilityManager* manager_;
    core::EventBus* event_bus_;
    bool is_valid_;
    QMap<int, int> subscriptions_; // local ID -> bus ID
    int next_subscription_id_;
};

inline std::shared_ptr<EventCapability> createEventCapabilityInstance(const QString& extensionId, core::CapabilityManager* mgr, core::EventBus* bus) {
    return std::static_pointer_cast<EventCapability>(std::make_shared<EventCapabilityImpl>(extensionId, mgr, bus));
}

} // namespace opencardev::crankshaft::core::capabilities
