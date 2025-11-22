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
#include <QVariant>
#include <QVariantList>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QUrlQuery>
#include "../core/capabilities/LocationCapability.hpp"
#include "../../extensions/navigation/GeocodingProvider.hpp"
#include "../../extensions/navigation/GeocodingProviderFactory.hpp"
class openauto_capability_manager_forward_decl;

namespace opencardev::crankshaft {
namespace core { class CapabilityManager; }
}

/**
 * NavigationBridge exposes persistent navigation-related settings to QML
 * and mediates GPS device selection into the LocationCapability, including
 * mock device modes for development.
 */
class NavigationBridge : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString gpsDevice READ gpsDevice WRITE setGpsDevice NOTIFY gpsDeviceChanged)
    Q_PROPERTY(QString geocodingProvider READ geocodingProvider WRITE setGeocodingProvider NOTIFY geocodingProviderChanged)
    Q_PROPERTY(QVariantList availableProviders READ availableProviders NOTIFY availableProvidersChanged)
public:
    static NavigationBridge* instance();

    static void initialise(opencardev::crankshaft::core::CapabilityManager* capabilityManager);
    static void registerQmlType();

    QString gpsDevice() const { return gpsDevice_; }
    QString geocodingProvider() const { return geocodingProviderId_; }
    QVariantList availableProviders() const;

public slots:
    void setGpsDevice(const QString& device);
    void setGeocodingProvider(const QString& providerId);
    
    // Geocoding API
    void searchLocation(const QString& query);
    
    // Favourites management
    QVariantList loadFavourites();
    void saveFavourites(const QVariantList& favourites);

signals:
    void gpsDeviceChanged();
    void geocodingProviderChanged();
    void availableProvidersChanged();
    void searchResultsReady(const QVariantList& results);
    void searchError(const QString& error);
    void routeCalculated(const QVariantMap& routeData);
    void routeError(const QString& error);

private:
    explicit NavigationBridge(QObject* parent = nullptr);
    void load();
    void save();
    void applyToCapability();
    void initializeProviders();
    void switchProvider(const QString& providerId);
    
    opencardev::crankshaft::core::CapabilityManager* capability_manager_ = nullptr;
    QString gpsDevice_ = "Internal";
    QString geocodingProviderId_ = "nominatim";
    QString settingsPath_;
    QString favouritesPath_;
    opencardev::crankshaft::extensions::navigation::GeocodingProvider* currentProvider_ = nullptr;
};
