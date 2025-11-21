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

#include "OSRMProvider.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QDebug>

namespace openauto {
namespace extensions {
namespace navigation {

OSRMProvider::OSRMProvider(QObject* parent)
    : RoutingProvider(parent)
    , networkManager_(new QNetworkAccessManager(this))
    , baseUrl_("https://router.project-osrm.org")
{
}

void OSRMProvider::calculateRoute(
    const QGeoCoordinate& start,
    const QGeoCoordinate& end,
    const QString& profile)
{
    // Build OSRM request URL
    // Format: /route/v1/{profile}/{coordinates}?steps=true&geometries=geojson
    QString url = QString("%1/route/v1/%2/%3,%4;%5,%6")
        .arg(baseUrl_)
        .arg(profile)
        .arg(start.longitude())
        .arg(start.latitude())
        .arg(end.longitude())
        .arg(end.latitude());
    
    QUrl requestUrl(url);
    QUrlQuery query;
    query.addQueryItem("steps", "true");
    query.addQueryItem("geometries", "geojson");
    query.addQueryItem("overview", "full");
    query.addQueryItem("annotations", "true");
    requestUrl.setQuery(query);
    
    qDebug() << "OSRM: Requesting route:" << requestUrl.toString();
    
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Crankshaft/1.0");
    
    QNetworkReply* reply = networkManager_->get(request);
    connect(reply, &QNetworkReply::finished, this, &OSRMProvider::handleRouteResponse);
}

void OSRMProvider::handleRouteResponse()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        QString error = QString("OSRM request failed: %1").arg(reply->errorString());
        qWarning() << error;
        emit routeError(error);
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isObject()) {
        emit routeError("Invalid OSRM response format");
        return;
    }
    
    QJsonObject root = doc.object();
    QString code = root["code"].toString();
    
    if (code != "Ok") {
        QString message = root["message"].toString();
        emit routeError(QString("OSRM error: %1").arg(message));
        return;
    }
    
    QJsonArray routes = root["routes"].toArray();
    if (routes.isEmpty()) {
        emit routeError("No route found");
        return;
    }
    
    // Parse first route
    QJsonObject routeObj = routes[0].toObject();
    Route route;
    route.totalDistance = routeObj["distance"].toDouble();
    route.totalDuration = routeObj["duration"].toDouble();
    
    // Parse geometry (route coordinates)
    QJsonObject geometry = routeObj["geometry"].toObject();
    QJsonArray coordinates = geometry["coordinates"].toArray();
    for (const QJsonValue& coord : coordinates) {
        QJsonArray point = coord.toArray();
        if (point.size() >= 2) {
            route.coordinates.append(QGeoCoordinate(
                point[1].toDouble(),
                point[0].toDouble()
            ));
        }
    }
    
    // Parse steps (turn-by-turn instructions)
    QJsonArray legs = routeObj["legs"].toArray();
    if (!legs.isEmpty()) {
        QJsonObject leg = legs[0].toObject();
        QJsonArray steps = leg["steps"].toArray();
        
        for (const QJsonValue& stepVal : steps) {
            QJsonObject stepObj = stepVal.toObject();
            
            RouteStep step;
            step.distance = stepObj["distance"].toDouble();
            step.duration = stepObj["duration"].toDouble();
            
            // Parse location
            QJsonArray location = stepObj["location"].toArray();
            if (location.size() >= 2) {
                step.location = QGeoCoordinate(
                    location[1].toDouble(),
                    location[0].toDouble()
                );
            }
            
            // Parse maneuver
            QJsonObject maneuver = stepObj["maneuver"].toObject();
            QString maneuverType = maneuver["type"].toString();
            QString modifier = maneuver["modifier"].toString();
            
            step.type = parseStepType(maneuverType);
            step.instruction = parseInstruction(stepObj);
            
            route.steps.append(step);
        }
    }
    
    route.summary = QString("%1 km, %2 min")
        .arg(route.totalDistance / 1000, 0, 'f', 1)
        .arg(qRound(route.totalDuration / 60));
    
    qDebug() << "OSRM: Route calculated:" << route.steps.size() << "steps,"
             << route.coordinates.size() << "points";
    
    emit routeCalculated(route);
}

QString OSRMProvider::parseStepType(const QString& maneuver)
{
    if (maneuver == "turn") return "turn";
    if (maneuver == "new name") return "straight";
    if (maneuver == "depart") return "depart";
    if (maneuver == "arrive") return "arrive";
    if (maneuver == "merge") return "merge";
    if (maneuver == "on ramp") return "exit";
    if (maneuver == "off ramp") return "exit";
    if (maneuver == "fork") return "fork";
    if (maneuver == "end of road") return "turn";
    if (maneuver == "continue") return "straight";
    if (maneuver == "roundabout") return "roundabout";
    if (maneuver == "rotary") return "roundabout";
    if (maneuver == "roundabout turn") return "roundabout";
    return "straight";
}

QString OSRMProvider::parseInstruction(const QJsonObject& stepObj)
{
    QJsonObject maneuver = stepObj["maneuver"].toObject();
    
    QString type = maneuver["type"].toString();
    QString modifier = maneuver["modifier"].toString();
    QString name = stepObj["name"].toString();
    
    // Build instruction
    QString instruction;
    
    if (type == "depart") {
        instruction = "Head ";
        if (!modifier.isEmpty()) {
            instruction += modifier.replace("-", " ");
        }
    } else if (type == "arrive") {
        instruction = "Arrive at destination";
        return instruction;
    } else if (type == "turn") {
        instruction = "Turn ";
        if (modifier == "left") instruction += "left";
        else if (modifier == "right") instruction += "right";
        else if (modifier == "sharp left") instruction += "sharp left";
        else if (modifier == "sharp right") instruction += "sharp right";
        else if (modifier == "slight left") instruction += "slight left";
        else if (modifier == "slight right") instruction += "slight right";
        else instruction += modifier.replace("-", " ");
    } else if (type == "roundabout" || type == "rotary") {
        int exit = maneuver["exit"].toInt();
        instruction = QString("Take exit %1 at roundabout").arg(exit);
    } else if (type == "continue" || type == "new name") {
        instruction = "Continue";
    } else if (type == "merge") {
        instruction = "Merge";
    } else if (type == "on ramp") {
        instruction = "Take the ramp";
    } else if (type == "off ramp") {
        instruction = "Take the exit";
    } else {
        instruction = "Continue";
    }
    
    // Add road name if available
    if (!name.isEmpty() && name != "-") {
        instruction += " onto " + name;
    }
    
    return instruction;
}

} // namespace navigation
} // namespace extensions
} // namespace openauto
