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

#include "GeocodingProviderFactory.hpp"
#include "NominatimProvider.hpp"
#include <QDebug>

namespace openauto {
namespace extensions {
namespace navigation {

GeocodingProviderFactory& GeocodingProviderFactory::instance() {
    static GeocodingProviderFactory factory;
    return factory;
}

void GeocodingProviderFactory::registerProvider(const QString& id,
                                               ProviderCreator creator,
                                               const QString& displayName,
                                               const QString& description,
                                               bool requiresApiKey) {
    ProviderRegistration reg;
    reg.creator = creator;
    reg.displayName = displayName;
    reg.description = description;
    reg.requiresApiKey = requiresApiKey;
    
    providers_[id] = reg;
    qInfo() << "Registered geocoding provider:" << id;
}

GeocodingProvider* GeocodingProviderFactory::createProvider(const QString& id, QObject* parent) const {
    if (!providers_.contains(id)) {
        qWarning() << "Unknown geocoding provider:" << id;
        return nullptr;
    }
    
    return providers_[id].creator(parent);
}

QStringList GeocodingProviderFactory::availableProviders() const {
    return providers_.keys();
}

GeocodingProviderFactory::ProviderInfo GeocodingProviderFactory::getProviderInfo(const QString& id) const {
    ProviderInfo info;
    
    if (providers_.contains(id)) {
        const auto& reg = providers_[id];
        info.id = id;
        info.displayName = reg.displayName;
        info.description = reg.description;
        info.requiresApiKey = reg.requiresApiKey;
    }
    
    return info;
}

QList<GeocodingProviderFactory::ProviderInfo> GeocodingProviderFactory::getAllProviderInfo() const {
    QList<ProviderInfo> infoList;
    
    for (auto it = providers_.constBegin(); it != providers_.constEnd(); ++it) {
        ProviderInfo info;
        info.id = it.key();
        info.displayName = it.value().displayName;
        info.description = it.value().description;
        info.requiresApiKey = it.value().requiresApiKey;
        infoList.append(info);
    }
    
    return infoList;
}

void GeocodingProviderFactory::registerBuiltInProviders() {
    auto& factory = instance();
    
    // Register Nominatim provider
    factory.registerProvider(
        "nominatim",
        [](QObject* parent) -> GeocodingProvider* {
            return new NominatimProvider(parent);
        },
        "OpenStreetMap Nominatim",
        "Free geocoding service by OpenStreetMap. No API key required.",
        false
    );
    
    // Future providers can be registered here:
    // factory.registerProvider("google", ...);
    // factory.registerProvider("mapbox", ...);
    // factory.registerProvider("here", ...);
}

} // namespace navigation
} // namespace extensions
} // namespace openauto
