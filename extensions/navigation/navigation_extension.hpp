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

#include "../../src/extensions/extension.hpp"
#include "RoutingProvider.hpp"
#include <QGeoCoordinate>
#include <QString>
#include <QVector>

namespace openauto {
namespace extensions {
namespace navigation {

struct RoutePoint {
    QGeoCoordinate coordinate;
    QString instruction;
    double distance;
    int estimatedTime;
};

class NavigationExtension : public Extension {
public:
    NavigationExtension() = default;
    ~NavigationExtension() override = default;

    // Lifecycle methods
    bool initialize() override;
    void start() override;
    void stop() override;
    void cleanup() override;

    // Metadata
    QString id() const override { return "navigation"; }
    QString name() const override { return "Navigation"; }
    QString version() const override { return "1.0.0"; }
    ExtensionType type() const override { return ExtensionType::UI; }

private:
    void setupEventHandlers();
    void handleNavigateToCommand(const QVariantMap& data);
    void handleCancelNavigationCommand(const QVariantMap& data);
    void handleSetDestinationCommand(const QVariantMap& data);
    void handleSearchLocationCommand(const QVariantMap& data);
    void updateCurrentLocation(const QGeoCoordinate& location);
    void publishNavigationUpdate();
    void handleRouteCalculated(const Route& route);
    void handleRouteError(const QString& error);

    QGeoCoordinate currentLocation_;
    QGeoCoordinate destination_;
    QVector<RoutePoint> currentRoute_;
    bool isNavigating_;
    int location_subscription_id_ = -1;
    RoutingProvider* routingProvider_ = nullptr;
};

}  // namespace navigation
}  // namespace extensions
}  // namespace openauto
