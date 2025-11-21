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

#include "RoutingProvider.hpp"
#include <QNetworkAccessManager>
#include <QNetworkReply>

namespace openauto {
namespace extensions {
namespace navigation {

/**
 * @brief OSRM (Open Source Routing Machine) routing provider
 * 
 * Uses public OSRM API for route calculation.
 * API: https://router.project-osrm.org/
 */
class OSRMProvider : public RoutingProvider {
    Q_OBJECT
    
public:
    explicit OSRMProvider(QObject* parent = nullptr);
    ~OSRMProvider() override = default;
    
    void calculateRoute(
        const QGeoCoordinate& start,
        const QGeoCoordinate& end,
        const QString& profile = "car") override;
    
    QString name() const override { return "OSRM"; }
    QString description() const override { return "Open Source Routing Machine (public API)"; }
    bool requiresApiKey() const override { return false; }
    
private slots:
    void handleRouteResponse();
    
private:
    QNetworkAccessManager* networkManager_;
    QString baseUrl_;
    
    QString parseStepType(const QString& maneuver);
    QString parseInstruction(const QJsonObject& step);
};

} // namespace navigation
} // namespace extensions
} // namespace openauto
