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
#include <QJSEngine>
#include <QQmlEngine>
#include "../core/capabilities/CapabilityManager.hpp"

static NavigationBridge* s_instance = nullptr;

NavigationBridge::NavigationBridge(QObject* parent) : QObject(parent) {
    // Determine settings path (relative config folder preferred)
    QString base = QDir::currentPath() + "/config";
    if (!QDir(base).exists()) {
        QDir().mkpath(base);
    }
    settingsPath_ = base + "/navigation_settings.json";
    favouritesPath_ = base + "/navigation_favourites.json";

    // Initialize provider system
    initializeProviders();

    load();
}

NavigationBridge* NavigationBridge::instance() {
    if (!s_instance) {
        s_instance = new NavigationBridge();
    }
    return s_instance;
}

void NavigationBridge::initialise(
    opencardev::crankshaft::core::CapabilityManager* capabilityManager) {
    instance()->capability_manager_ = capabilityManager;
    instance()->applyToCapability();
}

void NavigationBridge::registerQmlType() {
    qmlRegisterSingletonType<NavigationBridge>(
        "CrankshaftReborn.Navigation", 1, 0, "NavigationBridge",
        [](QQmlEngine* engine, QJSEngine* scriptEngine) -> QObject* {
            Q_UNUSED(engine)
            Q_UNUSED(scriptEngine)
            return NavigationBridge::instance();
        });
}

void NavigationBridge::load() {
    QFile f(settingsPath_);
    if (f.exists() && f.open(QIODevice::ReadOnly)) {
        const auto doc = QJsonDocument::fromJson(f.readAll());
        if (doc.isObject()) {
            gpsDevice_ = doc.object().value("gpsDevice").toString(gpsDevice_);
            QString providerId =
                doc.object().value("geocodingProvider").toString(geocodingProviderId_);
            if (providerId != geocodingProviderId_) {
                switchProvider(providerId);
            }
        }
    }
}

void NavigationBridge::save() {
    QJsonObject obj;
    obj["gpsDevice"] = gpsDevice_;
    obj["geocodingProvider"] = geocodingProviderId_;
    QFile f(settingsPath_);
    if (f.open(QIODevice::WriteOnly)) {
        f.write(QJsonDocument(obj).toJson(QJsonDocument::Compact));
        f.close();
    }
}

void NavigationBridge::applyToCapability() {
    if (!capability_manager_)
        return;
    auto locCap = capability_manager_->getLocationCapability("navigation");
    if (!locCap)
        return;
    using DM = opencardev::crankshaft::core::capabilities::LocationCapability::DeviceMode;
    DM mode = DM::Internal;
    if (gpsDevice_ == "USB Receiver")
        mode = DM::USB;
    else if (gpsDevice_ == "GNSS Hat")
        mode = DM::Hat;
    else if (gpsDevice_.startsWith("Mock") && gpsDevice_.contains("Static"))
        mode = DM::MockStatic;
    else if (gpsDevice_.startsWith("Mock") && gpsDevice_.contains("IP"))
        mode = DM::MockIP;
    locCap->setDeviceMode(mode);
    qInfo() << "NavigationBridge applied GPS device:" << gpsDevice_;
}

void NavigationBridge::setGpsDevice(const QString& device) {
    if (device == gpsDevice_)
        return;
    gpsDevice_ = device;
    save();
    applyToCapability();
    emit gpsDeviceChanged();
}

void NavigationBridge::searchLocation(const QString& query) {
    if (!currentProvider_) {
        emit searchError("No geocoding provider available");
        return;
    }

    if (query.trimmed().isEmpty()) {
        emit searchError("Search query is empty");
        return;
    }

    qDebug() << "Searching location with provider" << geocodingProviderId_ << ":" << query;
    currentProvider_->search(query);
}

void NavigationBridge::initializeProviders() {
    using namespace opencardev::crankshaft::extensions::navigation;

    // Register all built-in providers
    GeocodingProviderFactory::registerBuiltInProviders();

    // Create initial provider
    switchProvider(geocodingProviderId_);

    qInfo() << "Initialized geocoding providers. Available:"
            << GeocodingProviderFactory::instance().availableProviders();
}

void NavigationBridge::switchProvider(const QString& providerId) {
    using namespace opencardev::crankshaft::extensions::navigation;

    // Disconnect old provider
    if (currentProvider_) {
        disconnect(currentProvider_, nullptr, this, nullptr);
        currentProvider_->deleteLater();
        currentProvider_ = nullptr;
    }

    // Create new provider
    currentProvider_ = GeocodingProviderFactory::instance().createProvider(providerId, this);

    if (!currentProvider_) {
        qWarning() << "Failed to create provider:" << providerId << ". Falling back to nominatim.";
        currentProvider_ = GeocodingProviderFactory::instance().createProvider("nominatim", this);
        geocodingProviderId_ = "nominatim";
    } else {
        geocodingProviderId_ = providerId;
    }

    // Connect signals
    if (currentProvider_) {
        connect(currentProvider_, &GeocodingProvider::searchResultsReady, this,
                &NavigationBridge::searchResultsReady);
        connect(currentProvider_, &GeocodingProvider::errorOccurred, this,
                &NavigationBridge::searchError);

        qInfo() << "Switched to geocoding provider:" << geocodingProviderId_;
    }
}

QVariantList NavigationBridge::availableProviders() const {
    using namespace opencardev::crankshaft::extensions::navigation;

    QVariantList providers;
    auto infoList = GeocodingProviderFactory::instance().getAllProviderInfo();

    for (const auto& info : infoList) {
        QVariantMap providerMap;
        providerMap["id"] = info.id;
        providerMap["displayName"] = info.displayName;
        providerMap["description"] = info.description;
        providerMap["requiresApiKey"] = info.requiresApiKey;
        providers.append(providerMap);
    }

    return providers;
}

void NavigationBridge::setGeocodingProvider(const QString& providerId) {
    if (providerId == geocodingProviderId_)
        return;

    switchProvider(providerId);
    save();
    emit geocodingProviderChanged();
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
        if (!value.isObject())
            continue;

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
