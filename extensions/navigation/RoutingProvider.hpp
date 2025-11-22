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

#include <QObject>
#include <QString>
#include <QGeoCoordinate>
#include <QVariantMap>
#include <QVariantList>

namespace opencardev::crankshaft {
namespace extensions {
namespace navigation {

struct RouteStep {
    QString instruction;
    QString type;  // "turn-left", "turn-right", "straight", "roundabout", etc.
    QGeoCoordinate location;
    double distance;  // meters
    double duration;  // seconds
};

struct Route {
    QList<RouteStep> steps;
    QList<QGeoCoordinate> coordinates;
    double totalDistance;  // meters
    double totalDuration;  // seconds
    QString summary;
};

/**
 * @brief Abstract base class for routing providers
 * 
 * Provides interface for calculating routes between coordinates
 * and generating turn-by-turn navigation instructions.
 */
class RoutingProvider : public QObject {
    Q_OBJECT
    
public:
    explicit RoutingProvider(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~RoutingProvider() = default;
    
    /**
     * @brief Calculate route between two points
     * @param start Starting coordinate
     * @param end Destination coordinate
     * @param profile Routing profile (car, bike, foot)
     */
    virtual void calculateRoute(
        const QGeoCoordinate& start,
        const QGeoCoordinate& end,
        const QString& profile = "car") = 0;
    
    /**
     * @brief Get provider name
     */
    virtual QString name() const = 0;
    
    /**
     * @brief Get provider description
     */
    virtual QString description() const = 0;
    
    /**
     * @brief Check if provider requires API key
     */
    virtual bool requiresApiKey() const = 0;
    
signals:
    /**
     * @brief Emitted when route calculation completes
     * @param route Calculated route with steps and coordinates
     */
    void routeCalculated(const Route& route);
    
    /**
     * @brief Emitted when route calculation fails
     * @param error Error message
     */
    void routeError(const QString& error);
};

} // namespace navigation
} // namespace extensions
} // namespace openauto
