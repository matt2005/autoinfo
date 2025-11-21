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
#include <memory>

namespace openauto {
namespace core {
namespace capabilities {

/**
 * Base capability interface for extension security model.
 * 
 * Capabilities are unforgeable tokens that grant extensions specific
 * permissions. Extensions cannot create capabilities - only the core
 * CapabilityManager can grant them based on manifest permissions.
 * 
 * This follows the capability-based security model where possession
 * of a capability object is proof of authorization to use it.
 */
class Capability {
public:
    virtual ~Capability() = default;
    
    /**
     * Get the capability identifier (e.g., "location", "network").
     */
    virtual QString id() const = 0;
    
    /**
     * Check if this capability is still valid.
     * Returns false if capability has been revoked by core.
     */
    virtual bool isValid() const = 0;
    
    /**
     * Get the extension ID that owns this capability.
     */
    virtual QString extensionId() const = 0;

protected:
    // Only CapabilityManager can construct capabilities
    Capability() = default;
    
    // Non-copyable and non-moveable to prevent forgery
    Capability(const Capability&) = delete;
    Capability& operator=(const Capability&) = delete;
    Capability(Capability&&) = delete;
    Capability& operator=(Capability&&) = delete;
};

}  // namespace capabilities
}  // namespace core
}  // namespace openauto
