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

#include "NominatimProvider.hpp"
#include <QDebug>

namespace opencardev::crankshaft {
namespace extensions {
namespace navigation {

NominatimProvider::NominatimProvider(QObject* parent)
    : GeocodingProvider(parent)
    , networkManager_(new QNetworkAccessManager(this))
    , serverUrl_("https://nominatim.openstreetmap.org")
    , resultLimit_(10)
{
}

void NominatimProvider::search(const QString& query) {
    if (query.trimmed().isEmpty()) {
        emit errorOccurred("Search query is empty");
        return;
    }
    
    QUrl url(serverUrl_ + "/search");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("q", query);
    urlQuery.addQueryItem("format", "json");
    urlQuery.addQueryItem("limit", QString::number(resultLimit_));
    urlQuery.addQueryItem("addressdetails", "1");
    url.setQuery(urlQuery);
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Crankshaft/1.0");
    
    qDebug() << "Nominatim search:" << query;
    
    QNetworkReply* reply = networkManager_->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleSearchReply(reply);
    });
}

void NominatimProvider::reverseGeocode(double latitude, double longitude) {
    QUrl url(serverUrl_ + "/reverse");
    QUrlQuery urlQuery;
    urlQuery.addQueryItem("lat", QString::number(latitude, 'f', 8));
    urlQuery.addQueryItem("lon", QString::number(longitude, 'f', 8));
    urlQuery.addQueryItem("format", "json");
    urlQuery.addQueryItem("addressdetails", "1");
    url.setQuery(urlQuery);
    
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::UserAgentHeader, "Crankshaft/1.0");
    
    qDebug() << "Nominatim reverse geocode:" << latitude << longitude;
    
    QNetworkReply* reply = networkManager_->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        handleReverseReply(reply);
    });
}

void NominatimProvider::handleSearchReply(QNetworkReply* reply) {
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Nominatim search error:" << reply->errorString();
        emit errorOccurred(reply->errorString());
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isArray()) {
        emit errorOccurred("Invalid response from geocoding service");
        return;
    }
    
    QVariantList results = parseSearchResults(doc.array());
    qDebug() << "Nominatim returned" << results.size() << "results";
    emit searchResultsReady(results);
}

void NominatimProvider::handleReverseReply(QNetworkReply* reply) {
    reply->deleteLater();
    
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Nominatim reverse error:" << reply->errorString();
        emit errorOccurred(reply->errorString());
        return;
    }
    
    QByteArray data = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isObject()) {
        emit errorOccurred("Invalid response from geocoding service");
        return;
    }
    
    QVariantMap result = parseReverseResult(doc.object());
    QString address = result.value("display_name").toString();
    
    qDebug() << "Nominatim reverse complete:" << address;
    emit reverseGeocodeComplete(address, result);
}

QVariantList NominatimProvider::parseSearchResults(const QJsonArray& results) const {
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
        result["class"] = obj.value("class").toString();
        result["importance"] = obj.value("importance").toDouble();
        
        // Extract address details if available
        if (obj.contains("address")) {
            QJsonObject address = obj.value("address").toObject();
            result["city"] = address.value("city").toString();
            result["town"] = address.value("town").toString();
            result["village"] = address.value("village").toString();
            result["country"] = address.value("country").toString();
            result["postcode"] = address.value("postcode").toString();
            result["road"] = address.value("road").toString();
            result["house_number"] = address.value("house_number").toString();
        }
        
        resultsList.append(result);
    }
    
    return resultsList;
}

QVariantMap NominatimProvider::parseReverseResult(const QJsonObject& result) const {
    QVariantMap resultMap;
    
    resultMap["latitude"] = result.value("lat").toString().toDouble();
    resultMap["longitude"] = result.value("lon").toString().toDouble();
    resultMap["display_name"] = result.value("display_name").toString();
    resultMap["name"] = result.value("name").toString();
    resultMap["type"] = result.value("type").toString();
    
    if (result.contains("address")) {
        QJsonObject address = result.value("address").toObject();
        resultMap["city"] = address.value("city").toString();
        resultMap["town"] = address.value("town").toString();
        resultMap["country"] = address.value("country").toString();
        resultMap["postcode"] = address.value("postcode").toString();
        resultMap["road"] = address.value("road").toString();
    }
    
    return resultMap;
}

} // namespace navigation
} // namespace extensions
} // namespace openauto
