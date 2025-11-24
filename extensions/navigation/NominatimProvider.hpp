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

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>
#include <QUrlQuery>
#include "GeocodingProvider.hpp"

namespace opencardev::crankshaft {
namespace extensions {
namespace navigation {

/**
 * OpenStreetMap Nominatim geocoding provider
 * Free, no API key required, public service with usage policy
 */
class NominatimProvider : public GeocodingProvider {
    Q_OBJECT

  public:
    explicit NominatimProvider(QObject* parent = nullptr);
    ~NominatimProvider() override = default;

    QString id() const override { return "nominatim"; }
    QString displayName() const override { return "OpenStreetMap Nominatim"; }
    QString description() const override {
        return "Free geocoding service by OpenStreetMap. No API key required.";
    }
    bool requiresApiKey() const override { return false; }
    bool isAvailable() const override { return true; }

    void setApiKey(const QString& apiKey) override { Q_UNUSED(apiKey); }
    void search(const QString& query) override;
    void reverseGeocode(double latitude, double longitude) override;

    /**
     * Set custom Nominatim server URL (for self-hosted instances)
     */
    void setServerUrl(const QString& url) { serverUrl_ = url; }

    /**
     * Set result limit (default: 10)
     */
    void setResultLimit(int limit) { resultLimit_ = limit; }

  private slots:
    void handleSearchReply(QNetworkReply* reply);
    void handleReverseReply(QNetworkReply* reply);

  private:
    QNetworkAccessManager* networkManager_;
    QString serverUrl_;
    int resultLimit_;

    QVariantList parseSearchResults(const QJsonArray& results) const;
    QVariantMap parseReverseResult(const QJsonObject& result) const;
};

}  // namespace navigation
}  // namespace extensions
}  // namespace opencardev::crankshaft
