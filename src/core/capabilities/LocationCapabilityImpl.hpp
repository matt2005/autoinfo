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

#include <QGeoCoordinate>
#include <QMap>
#include <QTimer>
#include <QtPositioning/QGeoPositionInfo>
#include <QtPositioning/QGeoPositionInfoSource>
#include <functional>
#include "LocationCapability.hpp"

namespace opencardev::crankshaft::core {
class CapabilityManager;  // fwd
}

namespace opencardev::crankshaft::core::capabilities {

class LocationCapabilityImpl : public LocationCapability {
  public:
    LocationCapabilityImpl(const QString& extension_id, core::CapabilityManager* manager);
    ~LocationCapabilityImpl() override;

    QString extensionId() const override;
    bool isValid() const override;
    QGeoCoordinate getCurrentPosition() const override;
    int subscribeToUpdates(std::function<void(const QGeoCoordinate&)> callback) override;
    void unsubscribe(int subscriptionId) override;
    double getAccuracy() const override;
    bool isAvailable() const override;
    void setDeviceMode(DeviceMode mode) override;
    DeviceMode deviceMode() const override;

    void invalidate();

  private:
    void ensurePositionSource();
    void onPositionUpdated(const QGeoPositionInfo& info);
    void ensureMockTimer();

    QString extension_id_;
    core::CapabilityManager* manager_;
    bool is_valid_;
    QGeoPositionInfoSource* position_source_;
    QMap<int, std::function<void(const QGeoCoordinate&)>> subscriptions_;
    int next_subscription_id_;
    DeviceMode device_mode_;
    QTimer* mock_timer_;
    QGeoCoordinate mock_coordinate_;
};

// Factory helper
inline std::shared_ptr<LocationCapability> createLocationCapabilityInstance(
    const QString& extensionId, core::CapabilityManager* mgr) {
    return std::static_pointer_cast<LocationCapability>(
        std::make_shared<LocationCapabilityImpl>(extensionId, mgr));
}

}  // namespace opencardev::crankshaft::core::capabilities
