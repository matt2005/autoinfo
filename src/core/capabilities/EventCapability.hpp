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

#include "Capability.hpp"
#include <QString>
#include <QVariantMap>
#include <functional>

namespace opencardev::crankshaft {
namespace core {
namespace capabilities {

/**
 * Event capability for pub/sub communication.
 * 
 * Extensions with this capability can:
 * - Emit events to the event bus
 * - Subscribe to events from other extensions or core
 * - Unsubscribe from events
 * 
 * All events are scoped to prevent unauthorized access.
 * Extensions can only emit events in their own namespace
 * (e.g., "navigation.*") unless granted broader permissions.
 */
class EventCapability : public Capability {
public:
    virtual ~EventCapability() = default;
    
    QString id() const override { return "event"; }
    
    /**
     * Emit an event to the event bus.
     * Event name is automatically prefixed with extension namespace.
     * 
     * @param eventName Event name (e.g., "destination_set")
     * @param eventData Event data payload
     * @return true if event emitted successfully
     */
    virtual bool emitEvent(const QString& eventName, const QVariantMap& eventData) = 0;
    
    /**
     * Subscribe to events matching a pattern.
     * Patterns can include wildcards: "location.*", "*.updated"
     * 
     * Extensions can only subscribe to:
     * - Their own events (e.g., "navigation.*")
     * - Public core events (e.g., "core.*")
     * - Events from extensions they depend on (if permitted)
     * 
     * @param eventPattern Event pattern to match
     * @param callback Function to call when event received (receives event data only)
     * @return Subscription ID for unsubscribe
     */
    virtual int subscribe(
        const QString& eventPattern,
        std::function<void(const QVariantMap& eventData)> callback
    ) = 0;
    
    /**
     * Unsubscribe from events.
     * 
     * @param subscriptionId ID returned from subscribe
     */
    virtual void unsubscribe(int subscriptionId) = 0;
    
    /**
     * Check if extension can emit event with given name.
     * Useful for checking permissions before attempting emit.
     * 
     * @param eventName Event name to check
     * @return true if extension has permission to emit this event
     */
    virtual bool canEmit(const QString& eventName) const = 0;
    
    /**
     * Check if extension can subscribe to event pattern.
     * 
     * @param eventPattern Event pattern to check
     * @return true if extension has permission to subscribe
     */
    virtual bool canSubscribe(const QString& eventPattern) const = 0;

protected:
    EventCapability() = default;
};

}  // namespace capabilities
}  // namespace core
}  // namespace opencardev::crankshaft
