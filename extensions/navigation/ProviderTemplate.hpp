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

#include "GeocodingProvider.hpp"
#include <QNetworkAccessManager>

namespace openauto {
namespace extensions {
namespace navigation {

/**
 * Template for implementing a new geocoding provider
 * 
 * To add a new provider:
 * 1. Copy this template to YourProviderName.hpp
 * 2. Implement all pure virtual methods
 * 3. Create corresponding .cpp file
 * 4. Register in GeocodingProviderFactory::registerBuiltInProviders()
 * 5. Add to CMakeLists.txt
 * 
 * Example providers to implement:
 * - Google Maps Geocoding API (requires API key)
 * - Mapbox Geocoding API (requires API key)
 * - HERE Geocoding API (requires API key)
 * - Photon (free, self-hostable alternative to Nominatim)
 * - Custom/local geocoding service
 */
class TemplateProvider : public GeocodingProvider {
    Q_OBJECT
    
public:
    explicit TemplateProvider(QObject* parent = nullptr);
    ~TemplateProvider() override = default;
    
    // Required: Unique identifier (lowercase, alphanumeric)
    QString id() const override { return "template"; }
    
    // Required: Display name for UI
    QString displayName() const override { return "Template Provider"; }
    
    // Required: Brief description
    QString description() const override { 
        return "Template provider for demonstration purposes.";
    }
    
    // Required: Does this provider need an API key?
    bool requiresApiKey() const override { return false; }
    
    // Required: Is provider available (e.g., has valid config/key)?
    bool isAvailable() const override { return true; }
    
    // Required: Set API key if needed
    void setApiKey(const QString& apiKey) override;
    
    // Required: Perform geocoding search
    void search(const QString& query) override;
    
    // Required: Reverse geocoding (coordinates to address)
    void reverseGeocode(double latitude, double longitude) override;
    
    // Optional: Provider-specific configuration
    void setCustomOption(const QString& option) { customOption_ = option; }
    
private slots:
    void handleSearchReply(QNetworkReply* reply);
    void handleReverseReply(QNetworkReply* reply);
    
private:
    QNetworkAccessManager* networkManager_;
    QString apiKey_;
    QString customOption_;
    
    // Helper methods
    QVariantList parseSearchResults(const QByteArray& data) const;
    QVariantMap parseReverseResult(const QByteArray& data) const;
    
    // Provider-specific URL building
    QUrl buildSearchUrl(const QString& query) const;
    QUrl buildReverseUrl(double lat, double lng) const;
};

/**
 * Implementation guidelines:
 * 
 * 1. Constructor:
 *    - Initialize QNetworkAccessManager
 *    - Set default configuration
 * 
 * 2. search() method:
 *    - Build API URL with query parameters
 *    - Add authentication if needed (API key in header or URL)
 *    - Make async HTTP request
 *    - Connect reply to handleSearchReply()
 * 
 * 3. reverseGeocode() method:
 *    - Build API URL with lat/lng parameters
 *    - Add authentication if needed
 *    - Make async HTTP request
 *    - Connect reply to handleReverseReply()
 * 
 * 4. handleSearchReply():
 *    - Check for network errors -> emit errorOccurred()
 *    - Parse response (JSON, XML, etc.)
 *    - Convert to standard format (see below)
 *    - Emit searchResultsReady()
 * 
 * 5. handleReverseReply():
 *    - Check for network errors -> emit errorOccurred()
 *    - Parse response
 *    - Extract address string and details
 *    - Emit reverseGeocodeComplete()
 * 
 * Standard result format:
 * QVariantMap with keys:
 *   - latitude (double) - REQUIRED
 *   - longitude (double) - REQUIRED
 *   - display_name (QString) - REQUIRED (full address)
 *   - name (QString) - OPTIONAL (place name)
 *   - type (QString) - OPTIONAL (place type: "city", "road", etc.)
 *   - city (QString) - OPTIONAL
 *   - country (QString) - OPTIONAL
 *   - postcode (QString) - OPTIONAL
 *   - road (QString) - OPTIONAL
 * 
 * Error handling:
 *   - Network errors: emit errorOccurred(reply->errorString())
 *   - API errors: emit errorOccurred("API error: " + apiErrorMessage)
 *   - Parse errors: emit errorOccurred("Invalid response format")
 *   - Rate limiting: emit errorOccurred("Rate limit exceeded")
 * 
 * Best practices:
 *   - Set appropriate User-Agent header
 *   - Respect API rate limits
 *   - Handle API-specific error codes
 *   - Normalize results to standard format
 *   - Add logging with qDebug/qInfo/qWarning
 *   - Clean up network replies (reply->deleteLater())
 */

} // namespace navigation
} // namespace extensions
} // namespace openauto
