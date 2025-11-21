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

#include "NavigationBridge.hpp"
#include "../core/CapabilityManager.hpp"
#include <QQmlEngine>
#include <QJSEngine>

static NavigationBridge* s_instance = nullptr;

NavigationBridge::NavigationBridge(QObject* parent) : QObject(parent) {
    // Determine settings path (relative config folder preferred)
    QString base = QDir::currentPath() + "/config";
    if (!QDir(base).exists()) {
        QDir().mkpath(base);
    }
    settingsPath_ = base + "/navigation_settings.json";
    favouritesPath_ = base + "/navigation_favourites.json";
    
    // Initialize network manager for geocoding
    networkManager_ = new QNetworkAccessManager(this);
    
    load();
}

NavigationBridge* NavigationBridge::instance() {
    if (!s_instance) {
        s_instance = new NavigationBridge();
    }
    return s_instance;
}

void NavigationBridge::initialise(openauto::core::CapabilityManager* capabilityManager) {
    instance()->capability_manager_ = capabilityManager;
    instance()->applyToCapability();
}

void NavigationBridge::registerQmlType() {
    qmlRegisterSingletonType<NavigationBridge>("CrankshaftReborn.Navigation", 1, 0, "NavigationBridge",
        [](QQmlEngine* engine, QJSEngine* scriptEngine) -> QObject* {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return NavigationBridge::instance();
        }
    );
}

void NavigationBridge::load() {
    QFile f(settingsPath_);
    if (f.exists() && f.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(f.readAll());
        if (doc.isObject()) {
            gpsDevice_ = doc.object().value("gpsDevice").toString(gpsDevice_);
        }
    }
}

void NavigationBridge::save() {
    QJsonObject obj;
    obj["gpsDevice"] = gpsDevice_;
    QFile f(settingsPath_);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
        f.close();
    }
}

void NavigationBridge::applyToCapability() {
    if (!capability_manager_) return;
    auto locCap = capability_manager_->getLocationCapability("navigation");
    if (!locCap) return;
    using DM = openauto::core::capabilities::LocationCapability::DeviceMode;
    DM mode = DM::Internal;
    if (gpsDevice_ == "USB Receiver") mode = DM::USB;
    else if (gpsDevice_ == "GNSS Hat") mode = DM::Hat;
    else if (gpsDevice_.startsWith("Mock") && gpsDevice_.contains("Static")) mode = DM::MockStatic;
    else if (gpsDevice_.startsWith("Mock") && gpsDevice_.contains("IP")) mode = DM::MockIP;
    locCap->setDeviceMode(mode);
    qInfo() << "NavigationBridge applied GPS device:" << gpsDevice_;
}

void NavigationBridge::setGpsDevice(const QString& device) {
    if (device == gpsDevice_) return;
    gpsDevice_ = device;
    save();
    applyToCapability();
    emit gpsDeviceChanged();
}

void NavigationBridge::searchLocation(const QString& query) {
    if (query.trimmed().isEmpty()) {
        emit searchError("Search query is empty");
        return;
    }
    
    // Use Nominatim (OpenStreetMap) geocoding API
    QUrl url("https://nominatim.openstreetmap.org/search");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", query);
    urlQuery.addQueryItem("format", "json");
    urlQuery.addQueryItem("limit", "10");
    urlQuery.addQueryItem("addressdetails", "1");
    url.setQuery(urlQuery);
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Crankshaft/1.0");
    
    qDebug() << "Searching location:" << query;
    
    QNetworkReply* reply = networkManager_->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleGeocodingResponse(reply);
    });
}

void NavigationBridge::handleGeocodingResponse(QNetworkReply* reply) {
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Geocoding error:" << reply->errorString();
        emit searchError(reply->errorString());
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isArray()) {
        emit searchError("Invalid response from geocoding service");
        return;
    }
    
    QJsonArray results = doc.array();
    QVariantList resultsList;
    
    for (const QJsonValue& value : results) {
        if (!value.isObject()) continue;
        
        QJsonObject obj = value.toObject();
        QVariantMap result;
        
        result["latitude"] = obj.value("lat").toString().toDouble();
        result["longitude"] = obj.value("lon").toString().toDouble();
        result["display_name"] = obj.value("display_name").toString();
        result["name"] = obj.value("name").toString();
        result["type"] = obj.value("type").toString();
        
        // Extract address details if available
        if (obj.contains("address")) {
            QJsonObject address = obj.value("address").toObject();
            result["city"] = address.value("city").toString();
            result["country"] = address.value("country").toString();
            result["postcode"] = address.value("postcode").toString();
        }
        
        resultsList.append(result);
    }
    
    qDebug() << "Geocoding returned" << resultsList.size() << "results";
    emit searchResultsReady(resultsList);
}

QVariantList NavigationBridge::loadFavourites() {
    QFile f(favouritesPath_);
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
        return QVariantList();
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    
    if (!doc.isArray()) {
        return QVariantList();
    }
    
    QVariantList favourites;
    QJsonArray array = doc.array();
    
    for (const QJsonValue& value : array) {
        if (!value.isObject()) continue;
        
        QJsonObject obj = value.toObject();
        QVariantMap favourite;
        
        favourite["name"] = obj.value("name").toString();
        favourite["latitude"] = obj.value("latitude").toDouble();
        favourite["longitude"] = obj.value("longitude").toDouble();
        favourite["address"] = obj.value("address").toString();
        favourite["timestamp"] = obj.value("timestamp").toVariant();
        
        favourites.append(favourite);
    }
    
    qDebug() << "Loaded" << favourites.size() << "favourites";
    return favourites;
}

void NavigationBridge::saveFavourites(const QVariantList& favourites) {
    QJsonArray array;
    
    for (const QVariant& var : favourites) {
        QVariantMap favourite = var.toMap();
        
        QJsonObject obj;
        obj["name"] = favourite.value("name").toString();
        obj["latitude"] = favourite.value("latitude").toDouble();
        obj["longitude"] = favourite.value("longitude").toDouble();
        obj["address"] = favourite.value("address").toString();
        obj["timestamp"] = QJsonValue::fromVariant(favourite.value("timestamp"));
        
        array.append(obj);
    }
    
    QFile f(favouritesPath_);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(array).toJson(QJsonDocument::Compact));
        f.close();
        qDebug() << "Saved" << favourites.size() << "favourites";
    } else {
        qWarning() << "Failed to save favourites to" << favouritesPath_;
    }
}
