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

#include "navigation_extension.hpp"
#include "OSRMProvider.hpp"
#include "../../src/core/capabilities/LocationCapability.hpp"
#include "../../src/core/capabilities/NetworkCapability.hpp"
#include "../../src/core/capabilities/FileSystemCapability.hpp"
#include "../../src/core/capabilities/UICapability.hpp"
#include "../../src/core/capabilities/EventCapability.hpp"
#include "../../src/core/config/ConfigManager.hpp"
#include "../../src/core/config/ConfigTypes.hpp"
#include <QDebug>

namespace opencardev::crankshaft {
namespace extensions {
namespace navigation {

bool NavigationExtension::initialize() {
    qInfo() << "Initializing Navigation extension (capability-based)...";
    isNavigating_ = false;
    
    // Initialize routing provider
    routingProvider_ = new OSRMProvider(nullptr);
    QObject::connect(routingProvider_, &RoutingProvider::routeCalculated,
            [this](const Route& route) { handleRouteCalculated(route); });
    QObject::connect(routingProvider_, &RoutingProvider::routeError,
            [this](const QString& error) { handleRouteError(error); });
    
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

void NavigationExtension::registerConfigItems(core::config::ConfigManager* manager) {
    using namespace core::config;
    
    ConfigPage page;
    page.domain = "navigation";
    page.extension = "core";
    page.title = "Navigation Settings";
    page.description = "Configure GPS navigation and routing preferences";
    page.icon = "qrc:/icons/navigation.svg";
    
    // Route Settings Section
    ConfigSection routeSection;
    routeSection.key = "routing";
    routeSection.title = "Route Settings";
    routeSection.description = "Configure route calculation preferences";
    routeSection.complexity = ConfigComplexity::Basic;
    
    ConfigItem routingModeItem;
    routingModeItem.key = "routing_mode";
    routingModeItem.label = "Routing mode";
    routingModeItem.description = "Preferred routing mode for navigation";
    routingModeItem.type = ConfigItemType::Selection;
    routingModeItem.properties["options"] = QStringList{"Fastest", "Shortest", "Eco", "Avoid Highways", "Avoid Tolls"};
    routingModeItem.defaultValue = "Fastest";
    routingModeItem.complexity = ConfigComplexity::Basic;
    routingModeItem.required = false;
    routingModeItem.readOnly = false;
    routeSection.items.append(routingModeItem);
    
    ConfigItem avoidItem;
    avoidItem.key = "avoid_features";
    avoidItem.label = "Avoid features";
    avoidItem.description = "Route features to avoid";
    avoidItem.type = ConfigItemType::MultiSelection;
    avoidItem.properties["options"] = QStringList{"Highways", "Tolls", "Ferries", "Unpaved Roads"};
    avoidItem.defaultValue = QStringList{};
    avoidItem.complexity = ConfigComplexity::Basic;
    avoidItem.required = false;
    avoidItem.readOnly = false;
    routeSection.items.append(avoidItem);
    
    ConfigItem autoRecalculateItem;
    autoRecalculateItem.key = "auto_recalculate";
    autoRecalculateItem.label = "Auto-recalculate route";
    autoRecalculateItem.description = "Automatically recalculate route when deviating";
    autoRecalculateItem.type = ConfigItemType::Boolean;
    autoRecalculateItem.defaultValue = true;
    autoRecalculateItem.complexity = ConfigComplexity::Basic;
    autoRecalculateItem.required = false;
    autoRecalculateItem.readOnly = false;
    routeSection.items.append(autoRecalculateItem);
    
    ConfigItem recalculateThresholdItem;
    recalculateThresholdItem.key = "recalculate_threshold";
    recalculateThresholdItem.label = "Recalculation threshold";
    recalculateThresholdItem.description = "Distance threshold before triggering recalculation";
    recalculateThresholdItem.type = ConfigItemType::Integer;
    recalculateThresholdItem.defaultValue = 100;
    recalculateThresholdItem.properties["minValue"] = 50;
    recalculateThresholdItem.properties["maxValue"] = 500;
    recalculateThresholdItem.unit = "meters";
    recalculateThresholdItem.complexity = ConfigComplexity::Advanced;
    recalculateThresholdItem.required = false;
    recalculateThresholdItem.readOnly = false;
    routeSection.items.append(recalculateThresholdItem);
    
    page.sections.append(routeSection);
    
    // Display Settings Section
    ConfigSection displaySection;
    displaySection.key = "display";
    displaySection.title = "Display Settings";
    displaySection.description = "Configure map display and orientation";
    displaySection.complexity = ConfigComplexity::Basic;
    
    ConfigItem mapOrientationItem;
    mapOrientationItem.key = "map_orientation";
    mapOrientationItem.label = "Map orientation";
    mapOrientationItem.description = "How to orient the map display";
    mapOrientationItem.type = ConfigItemType::Selection;
    mapOrientationItem.properties["options"] = QStringList{"North Up", "Heading Up", "3D"};
    mapOrientationItem.defaultValue = "Heading Up";
    mapOrientationItem.complexity = ConfigComplexity::Basic;
    mapOrientationItem.required = false;
    mapOrientationItem.readOnly = false;
    displaySection.items.append(mapOrientationItem);
    
    ConfigItem showTrafficItem;
    showTrafficItem.key = "show_traffic";
    showTrafficItem.label = "Show traffic";
    showTrafficItem.description = "Display real-time traffic information";
    showTrafficItem.type = ConfigItemType::Boolean;
    showTrafficItem.defaultValue = true;
    showTrafficItem.complexity = ConfigComplexity::Basic;
    showTrafficItem.required = false;
    showTrafficItem.readOnly = false;
    displaySection.items.append(showTrafficItem);
    
    ConfigItem speedLimitItem;
    speedLimitItem.key = "show_speed_limit";
    speedLimitItem.label = "Show speed limit";
    speedLimitItem.description = "Display current speed limit on route";
    speedLimitItem.type = ConfigItemType::Boolean;
    speedLimitItem.defaultValue = true;
    speedLimitItem.complexity = ConfigComplexity::Basic;
    speedLimitItem.required = false;
    speedLimitItem.readOnly = false;
    displaySection.items.append(speedLimitItem);
    
    page.sections.append(displaySection);
    
    // Voice Guidance Section
    ConfigSection voiceSection;
    voiceSection.key = "voice";
    voiceSection.title = "Voice Guidance";
    voiceSection.description = "Configure voice navigation instructions";
    voiceSection.complexity = ConfigComplexity::Basic;
    
    ConfigItem voiceGuidanceItem;
    voiceGuidanceItem.key = "enable_voice";
    voiceGuidanceItem.label = "Enable voice guidance";
    voiceGuidanceItem.description = "Provide turn-by-turn voice instructions";
    voiceGuidanceItem.type = ConfigItemType::Boolean;
    voiceGuidanceItem.defaultValue = true;
    voiceGuidanceItem.complexity = ConfigComplexity::Basic;
    voiceGuidanceItem.required = false;
    voiceGuidanceItem.readOnly = false;
    voiceSection.items.append(voiceGuidanceItem);
    
    ConfigItem voiceVolumeItem;
    voiceVolumeItem.key = "voice_volume";
    voiceVolumeItem.label = "Voice volume";
    voiceVolumeItem.description = "Volume level for voice guidance";
    voiceVolumeItem.type = ConfigItemType::Integer;
    voiceVolumeItem.defaultValue = 80;
    voiceVolumeItem.properties["minValue"] = 0;
    voiceVolumeItem.properties["maxValue"] = 100;
    voiceVolumeItem.unit = "%";
    voiceVolumeItem.complexity = ConfigComplexity::Basic;
    voiceVolumeItem.required = false;
    voiceVolumeItem.readOnly = false;
    voiceSection.items.append(voiceVolumeItem);
    
    ConfigItem voiceLanguageItem;
    voiceLanguageItem.key = "voice_language";
    voiceLanguageItem.label = "Voice language";
    voiceLanguageItem.description = "Language for voice guidance";
    voiceLanguageItem.type = ConfigItemType::Selection;
    voiceLanguageItem.properties["options"] = QStringList{"English (UK)", "English (US)", "French", "German", "Spanish", "Italian"};
    voiceLanguageItem.defaultValue = "English (UK)";
    voiceLanguageItem.complexity = ConfigComplexity::Basic;
    voiceLanguageItem.required = false;
    voiceLanguageItem.readOnly = false;
    voiceSection.items.append(voiceLanguageItem);
    
    page.sections.append(voiceSection);
    
    // Advanced Settings Section
    ConfigSection advancedSection;
    advancedSection.key = "advanced";
    advancedSection.title = "Advanced Settings";
    advancedSection.description = "Configure advanced routing and map options";
    advancedSection.complexity = ConfigComplexity::Advanced;
    
    ConfigItem osrmServerItem;
    osrmServerItem.key = "osrm_server";
    osrmServerItem.label = "OSRM server URL";
    osrmServerItem.description = "URL of the OSRM routing server";
    osrmServerItem.type = ConfigItemType::String;
    osrmServerItem.defaultValue = "http://router.project-osrm.org";
    osrmServerItem.properties["placeholder"] = "http://server:port";
    osrmServerItem.complexity = ConfigComplexity::Expert;
    osrmServerItem.required = false;
    osrmServerItem.readOnly = false;
    advancedSection.items.append(osrmServerItem);
    
    ConfigItem cacheSizeItem;
    cacheSizeItem.key = "map_cache_size";
    cacheSizeItem.label = "Map cache size";
    cacheSizeItem.description = "Maximum size for offline map cache";
    cacheSizeItem.type = ConfigItemType::Integer;
    cacheSizeItem.defaultValue = 500;
    cacheSizeItem.properties["minValue"] = 100;
    cacheSizeItem.properties["maxValue"] = 2000;
    cacheSizeItem.unit = "MB";
    cacheSizeItem.complexity = ConfigComplexity::Advanced;
    cacheSizeItem.required = false;
    cacheSizeItem.readOnly = false;
    advancedSection.items.append(cacheSizeItem);
    
    page.sections.append(advancedSection);
    
    // Register the page
    manager->registerConfigPage(page);
    qInfo() << "Navigation extension registered config items";
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

    // Calculate route using routing provider
    if (routingProvider_ && currentLocation_.isValid()) {
        qDebug() << "Calculating route from" << currentLocation_ << "to" << destination_;
        routingProvider_->calculateRoute(currentLocation_, destination_);
    } else {
        qWarning() << "Cannot calculate route: provider or location invalid";
    }
    
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

void NavigationExtension::handleRouteCalculated(const Route& route) {
    qInfo() << "Route calculated:" << route.steps.size() << "steps";
    
    // Send route to UI via event
    auto eventCap = getCapability<core::capabilities::EventCapability>();
    if (!eventCap) return;
    
    // Convert route coordinates to QVariantList
    QVariantList coordinates;
    for (const QGeoCoordinate& coord : route.coordinates) {
        QVariantMap coordMap;
        coordMap["latitude"] = coord.latitude();
        coordMap["longitude"] = coord.longitude();
        coordinates.append(coordMap);
    }
    
    // Convert route steps to QVariantList
    QVariantList steps;
    for (const RouteStep& step : route.steps) {
        QVariantMap stepMap;
        stepMap["instruction"] = step.instruction;
        stepMap["type"] = step.type;
        stepMap["distance"] = step.distance;
        stepMap["duration"] = step.duration;
        stepMap["latitude"] = step.location.latitude();
        stepMap["longitude"] = step.location.longitude();
        steps.append(stepMap);
    }
    
    QVariantMap routeData;
    routeData["coordinates"] = coordinates;
    routeData["steps"] = steps;
    routeData["totalDistance"] = route.totalDistance;
    routeData["totalDuration"] = route.totalDuration;
    routeData["summary"] = route.summary;
    
    eventCap->emitEvent("navigation.routeCalculated", routeData);
    qDebug() << "Route data sent to UI";
}

void NavigationExtension::handleRouteError(const QString& error) {
    qWarning() << "Route calculation error:" << error;
    
    auto eventCap = getCapability<core::capabilities::EventCapability>();
    if (!eventCap) return;
    
    QVariantMap errorData;
    errorData["error"] = error;
    eventCap->emitEvent("navigation.routeError", errorData);
}

}  // namespace navigation
}  // namespace extensions
}  // namespace opencardev::crankshaft
