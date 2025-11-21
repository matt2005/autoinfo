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
#include "../../src/core/capabilities/LocationCapability.hpp"
#include "../../src/core/capabilities/NetworkCapability.hpp"
#include "../../src/core/capabilities/FileSystemCapability.hpp"
#include "../../src/core/capabilities/UICapability.hpp"
#include "../../src/core/capabilities/EventCapability.hpp"
#include <QDebug>

namespace openauto {
namespace extensions {
namespace navigation {

bool NavigationExtension::initialize() {
    qInfo() << "Initializing Navigation extension (capability-based)...";
    isNavigating_ = false;
    
    // Check required capabilities
    if (!hasCapability("location")) {
        qWarning() << "Navigation: Location capability not granted!";
    }
    if (!hasCapability("event")) {
        qWarning() << "Navigation: Event capability not granted!";
    }
    if (!hasCapability("ui")) {
        qWarning() << "Navigation: UI capability not granted!";
    }
    
    return true;
}

void NavigationExtension::start() {
    qInfo() << "Starting Navigation extension...";
    
    // Register UI using UI capability
    auto uiCap = getCapability<core::capabilities::UICapability>();
    if (uiCap) {
        QVariantMap metadata;
        metadata["title"] = "Navigation";
        metadata["icon"] = "navigation";
        metadata["description"] = "GPS navigation with real-time traffic";
        uiCap->registerMainView("qrc:/navigation/qml/NavigationView.qml", metadata);
        qInfo() << "Navigation: Registered main view with path: qrc:/navigation/qml/NavigationView.qml";
    }
    
    // Subscribe to events using Event capability
    setupEventHandlers();
    
    // Subscribe to location updates using Location capability
    auto locationCap = getCapability<core::capabilities::LocationCapability>();
    if (locationCap) {
        location_subscription_id_ = locationCap->subscribeToUpdates(
            [this](const QGeoCoordinate& coord) {
                updateCurrentLocation(coord);
            }
        );
        qInfo() << "Navigation: Subscribed to location updates";
    }
}

void NavigationExtension::stop() {
    qInfo() << "Stopping Navigation extension...";
    isNavigating_ = false;
    
    // Unsubscribe from location updates
    auto locationCap = getCapability<core::capabilities::LocationCapability>();
    if (locationCap && location_subscription_id_ >= 0) {
        locationCap->unsubscribe(location_subscription_id_);
        location_subscription_id_ = -1;
    }
}

void NavigationExtension::cleanup() {
    qInfo() << "Cleaning up Navigation extension...";
    currentRoute_.clear();
}

void NavigationExtension::setupEventHandlers() {
    auto eventCap = getCapability<core::capabilities::EventCapability>();
    if (!eventCap) {
        qWarning() << "Navigation: Event capability not available";
        return;
    }

    // Subscribe to navigation commands (our own namespace)
    eventCap->subscribe("navigation.navigateTo", [this](const QVariantMap& data) {
        handleNavigateToCommand(data);
    });

    eventCap->subscribe("navigation.cancel", [this](const QVariantMap& data) {
        handleCancelNavigationCommand(data);
    });

    eventCap->subscribe("navigation.setDestination", [this](const QVariantMap& data) {
        handleSetDestinationCommand(data);
    });

    eventCap->subscribe("navigation.searchLocation", [this](const QVariantMap& data) {
        handleSearchLocationCommand(data);
    });
    
    qInfo() << "Navigation: Event handlers configured";
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

    auto eventCap = getCapability<core::capabilities::EventCapability>();
    if (eventCap) {
        QVariantMap event;
        event["status"] = "cancelled";
        eventCap->emitEvent("status", event);
    }
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

    // Use network capability to search for location
    auto networkCap = getCapability<core::capabilities::NetworkCapability>();
    if (networkCap && networkCap->isOnline()) {
        // Example: Search using Nominatim API
        QString searchUrl = QString("https://nominatim.openstreetmap.org/search?q=%1&format=json&limit=5")
            .arg(QString(QUrl::toPercentEncoding(query)));
        
        QNetworkReply* reply = networkCap->get(QUrl(searchUrl));
        if (reply) {
            QObject::connect(reply, &QNetworkReply::finished, [this, reply]() {
                if (reply->error() == QNetworkReply::NoError) {
                    QByteArray response = reply->readAll();
                    qInfo() << "Search results:" << response;
                    
                    // Parse and emit results via event capability
                    auto eventCap = getCapability<core::capabilities::EventCapability>();
                    if (eventCap) {
                        QVariantMap event;
                        event["results"] = QString::fromUtf8(response);
                        eventCap->emitEvent("searchResults", event);
                    }
                }
                reply->deleteLater();
            });
        }
    } else {
        qWarning() << "Network capability not available for location search";
    }
}

void NavigationExtension::updateCurrentLocation(const QGeoCoordinate& location) {
    currentLocation_ = location;

    if (isNavigating_) {
        publishNavigationUpdate();
    }
}

void NavigationExtension::publishNavigationUpdate() {
    auto eventCap = getCapability<core::capabilities::EventCapability>();
    if (!eventCap) {
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

    eventCap->emitEvent("update", event);
}

}  // namespace navigation
}  // namespace extensions
}  // namespace openauto
