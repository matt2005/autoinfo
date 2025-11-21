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

#include <QString>
#include <QVariantMap>
#include <QMap>
#include <memory>
#include "../core/capabilities/Capability.hpp"

namespace openauto {
namespace core {
    namespace capabilities {
        // Forward declaration not needed - included above
    }
}

namespace extensions {

enum class ExtensionType {
    Unknown,
    Service,      // Background services (e.g., Bluetooth, GPS)
    UI,           // UI components (e.g., Media player UI)
    Integration,  // Third-party integrations
    Platform      // Platform-specific features
};

/**
 * Base Extension class using capability-based security.
 * 
 * Extensions do NOT have direct access to core services (EventBus, WebSocket, etc.).
 * Instead, extensions must request capabilities based on their manifest permissions.
 * Capabilities are unforgeable tokens that grant specific operations.
 * 
 * This provides:
 * - Security: Extensions can't access APIs they don't have capabilities for
 * - Auditability: All capability usage is logged
 * - Revocability: Core can revoke capabilities at runtime
 * - Least privilege: Extensions only get what they need
 */
class Extension {
public:
    virtual ~Extension() = default;

    // Lifecycle methods
    virtual bool initialize() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void cleanup() = 0;

    // Metadata
    virtual QString id() const = 0;
    virtual QString name() const = 0;
    virtual QString version() const = 0;
    virtual ExtensionType type() const = 0;

    /**
     * Grant a capability to this extension.
     * Called by CapabilityManager during extension loading.
     * 
     * @param capability Capability object (LocationCapability, NetworkCapability, etc.)
     */
    void grantCapability(std::shared_ptr<core::capabilities::Capability> capability) {
        if (capability) {
            capabilities_[capability->id()] = capability;
        }
    }
    
    /**
     * Check if extension has a specific capability.
     * 
     * @param capabilityId Capability ID ("location", "network", etc.)
     * @return true if extension has valid capability
     */
    bool hasCapability(const QString& capabilityId) const {
        return capabilities_.contains(capabilityId) &&
               capabilities_[capabilityId] &&
               capabilities_[capabilityId]->isValid();
    }

protected:
    /**
     * Get a capability by type.
     * Returns nullptr if capability not granted or invalid.
     * 
     * Usage:
     *   auto locationCap = getCapability<LocationCapability>();
     *   if (locationCap) {
     *       QGeoCoordinate pos = locationCap->getCurrentPosition();
     *   }
     * 
     * @return Capability pointer or nullptr
     */
    template<typename T>
    std::shared_ptr<T> getCapability() const {
        for (const auto& cap : capabilities_) {
            auto typed = std::dynamic_pointer_cast<T>(cap);
            if (typed && typed->isValid()) {
                return typed;
            }
        }
        return nullptr;
    }
    
    /**
     * Get capability by ID.
     * 
     * @param capabilityId Capability ID
     * @return Capability pointer or nullptr
     */
    std::shared_ptr<core::capabilities::Capability> getCapabilityById(
        const QString& capabilityId
    ) const {
        if (capabilities_.contains(capabilityId)) {
            return capabilities_[capabilityId];
        }
        return nullptr;
    }

private:
    // Granted capabilities (capability_id -> capability)
    QMap<QString, std::shared_ptr<core::capabilities::Capability>> capabilities_;
};

}  // namespace extensions
}  // namespace openauto
