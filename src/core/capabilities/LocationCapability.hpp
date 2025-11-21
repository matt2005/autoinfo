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
#include <QGeoCoordinate>
#include <functional>

namespace openauto {
namespace core {
namespace capabilities {

/**
 * Location capability for GPS/positioning access.
 * 
 * Extensions with this capability can:
 * - Get current GPS position
 * - Subscribe to location updates
 * - Get location accuracy and metadata
 * 
 * Extensions cannot directly access Qt positioning APIs.
 */
class LocationCapability : public Capability {
public:
    virtual ~LocationCapability() = default;
    
    QString id() const override { return "location"; }

    // GPS device modes supported by the location capability.
    // Internal/USB/Hat use the underlying platform position source.
    // MockStatic provides a fixed coordinate for development.
    // MockIP resolves approximate location from public IP (network required).
    enum class DeviceMode {
        Internal,
        USB,
        Hat,
        MockStatic,
        MockIP
    };
    
    /**
     * Get the current GPS position.
     * Returns invalid coordinate if location unavailable.
     */
    virtual QGeoCoordinate getCurrentPosition() const = 0;
    
    /**
     * Subscribe to location updates.
     * Callback is invoked whenever position changes.
     * 
     * @param callback Function to call with new position
     * @return Subscription ID for unsubscribe
     */
    virtual int subscribeToUpdates(std::function<void(const QGeoCoordinate&)> callback) = 0;
    
    /**
     * Unsubscribe from location updates.
     * 
     * @param subscriptionId ID returned from subscribeToUpdates
     */
    virtual void unsubscribe(int subscriptionId) = 0;
    
    /**
     * Get location accuracy in metres.
     */
    virtual double getAccuracy() const = 0;
    
    /**
     * Check if location service is available.
     */
    virtual bool isAvailable() const = 0;

    // Set the active GPS device mode. Implementations should switch
    // between real position source and mock providers accordingly.
    virtual void setDeviceMode(DeviceMode mode) = 0;

    // Get current active device mode.
    virtual DeviceMode deviceMode() const = 0;

protected:
    LocationCapability() = default;
};

}  // namespace capabilities
}  // namespace core
}  // namespace openauto
