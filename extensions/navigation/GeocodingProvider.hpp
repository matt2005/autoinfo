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
#include <QVariantList>
#include <QVariantMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QUrl>

namespace openauto {
namespace extensions {
namespace navigation {

/**
 * Abstract interface for geocoding/mapping service providers
 * Implementations provide search functionality for address -> coordinates
 */
class GeocodingProvider : public QObject {
    Q_OBJECT
    
public:
    explicit GeocodingProvider(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~GeocodingProvider() = default;
    
    /**
     * Unique identifier for this provider (e.g., "nominatim", "google", "mapbox")
     */
    virtual QString id() const = 0;
    
    /**
     * Display name for UI (e.g., "OpenStreetMap Nominatim")
     */
    virtual QString displayName() const = 0;
    
    /**
     * Description of the provider and its features
     */
    virtual QString description() const = 0;
    
    /**
     * Whether this provider requires an API key
     */
    virtual bool requiresApiKey() const = 0;
    
    /**
     * Whether this provider is available (e.g., has valid config)
     */
    virtual bool isAvailable() const = 0;
    
    /**
     * Set API key if required
     */
    virtual void setApiKey(const QString& apiKey) = 0;
    
    /**
     * Perform geocoding search
     * @param query - Search string (address, place name, etc.)
     */
    virtual void search(const QString& query) = 0;
    
    /**
     * Reverse geocoding: coordinates -> address
     * @param latitude - Latitude coordinate
     * @param longitude - Longitude coordinate
     */
    virtual void reverseGeocode(double latitude, double longitude) = 0;
    
signals:
    /**
     * Emitted when search results are ready
     * @param results - List of result maps with keys:
     *   - latitude (double)
     *   - longitude (double)
     *   - display_name (QString)
     *   - name (QString)
     *   - type (QString)
     *   - city (QString, optional)
     *   - country (QString, optional)
     *   - postcode (QString, optional)
     */
    void searchResultsReady(const QVariantList& results);
    
    /**
     * Emitted when reverse geocoding is complete
     * @param address - Formatted address string
     * @param details - Map with detailed address components
     */
    void reverseGeocodeComplete(const QString& address, const QVariantMap& details);
    
    /**
     * Emitted on error
     * @param error - Error message
     */
    void errorOccurred(const QString& error);
    
protected:
    /**
     * Helper to normalize results to common format
     */
    virtual QVariantList normalizeResults(const QVariantList& rawResults) const {
        return rawResults;
    }
};

} // namespace navigation
} // namespace extensions
} // namespace openauto
