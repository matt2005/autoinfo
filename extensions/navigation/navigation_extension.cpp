/*
 * Project: OpenAuto
 * This file is part of openauto project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  openauto is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  openauto is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with openauto. If not, see <http://www.gnu.org/licenses/>.
 */

#include "navigation_extension.hpp"
#include "../../src/core/event_bus.hpp"
#include <QDebug>

namespace openauto {
namespace extensions {
namespace navigation {

bool NavigationExtension::initialize() {
    qInfo() << "Initializing Navigation extension...";
    isNavigating_ = false;
    setupEventHandlers();
    return true;
}

void NavigationExtension::start() {
    qInfo() << "Starting Navigation extension...";
}

void NavigationExtension::stop() {
    qInfo() << "Stopping Navigation extension...";
    isNavigating_ = false;
}

void NavigationExtension::cleanup() {
    qInfo() << "Cleaning up Navigation extension...";
    currentRoute_.clear();
}

void NavigationExtension::setupEventHandlers() {
    if (!event_bus_) {
        qWarning() << "Event bus not available";
        return;
    }

    event_bus_->subscribe("navigation.navigateTo", [this](const QVariantMap& data) {
        handleNavigateToCommand(data);
    });

    event_bus_->subscribe("navigation.cancel", [this](const QVariantMap& data) {
        handleCancelNavigationCommand(data);
    });

    event_bus_->subscribe("navigation.setDestination", [this](const QVariantMap& data) {
        handleSetDestinationCommand(data);
    });

    event_bus_->subscribe("navigation.searchLocation", [this](const QVariantMap& data) {
        handleSearchLocationCommand(data);
    });

    event_bus_->subscribe("location.updated", [this](const QVariantMap& data) {
        double latitude = data.value("latitude").toDouble();
        double longitude = data.value("longitude").toDouble();
        updateCurrentLocation(QGeoCoordinate(latitude, longitude));
    });
}

void NavigationExtension::handleNavigateToCommand(const QVariantMap& data) {
    qDebug() << "Navigate to command received:" << data;

    double latitude = data.value("latitude").toDouble();
    double longitude = data.value("longitude").toDouble();
    QString address = data.value("address").toString();

    destination_ = QGeoCoordinate(latitude, longitude);
    isNavigating_ = true;

    qInfo() << "Starting navigation to:" << address
            << "(" << latitude << "," << longitude << ")";

    // TODO: Calculate route
    publishNavigationUpdate();
}

void NavigationExtension::handleCancelNavigationCommand(const QVariantMap& data) {
    Q_UNUSED(data);
    qDebug() << "Cancel navigation command received";

    isNavigating_ = false;
    currentRoute_.clear();

    QVariantMap event;
    event["status"] = "cancelled";
    event_bus_->publish("navigation.status", event);
}

void NavigationExtension::handleSetDestinationCommand(const QVariantMap& data) {
    qDebug() << "Set destination command received:" << data;

    double latitude = data.value("latitude").toDouble();
    double longitude = data.value("longitude").toDouble();

    destination_ = QGeoCoordinate(latitude, longitude);
}

void NavigationExtension::handleSearchLocationCommand(const QVariantMap& data) {
    qDebug() << "Search location command received:" << data;
    QString query = data.value("query").toString();

    // TODO: Implement location search
    qInfo() << "Searching for location:" << query;
}

void NavigationExtension::updateCurrentLocation(const QGeoCoordinate& location) {
    currentLocation_ = location;

    if (isNavigating_) {
        publishNavigationUpdate();
    }
}

void NavigationExtension::publishNavigationUpdate() {
    if (!event_bus_) {
        return;
    }

    QVariantMap event;
    event["latitude"] = currentLocation_.latitude();
    event["longitude"] = currentLocation_.longitude();
    event["destLatitude"] = destination_.latitude();
    event["destLongitude"] = destination_.longitude();
    event["isNavigating"] = isNavigating_;

    if (isNavigating_ && currentLocation_.isValid() && destination_.isValid()) {
        double distance = currentLocation_.distanceTo(destination_);
        event["distanceRemaining"] = distance;

        // Rough ETA calculation (assumes 50 km/h average speed)
        int etaSeconds = static_cast<int>(distance / 13.89);  // 50 km/h = 13.89 m/s
        event["etaSeconds"] = etaSeconds;
    }

    event_bus_->publish("navigation.update", event);
}

}  // namespace navigation
}  // namespace extensions
}  // namespace openauto
