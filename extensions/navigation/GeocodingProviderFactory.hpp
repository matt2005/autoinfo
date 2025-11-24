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

#include <QMap>
#include <QObject>
#include <QString>
#include <QStringList>
#include <memory>
#include "GeocodingProvider.hpp"

namespace opencardev::crankshaft {
namespace extensions {
namespace navigation {

/**
 * Factory for creating geocoding provider instances
 * Manages registration and instantiation of providers
 */
class GeocodingProviderFactory {
  public:
    /**
     * Get singleton instance
     */
    static GeocodingProviderFactory& instance();

    /**
     * Register a provider creator function
     * @param id - Provider identifier
     * @param creator - Function that creates provider instance
     * @param displayName - Display name for UI
     * @param description - Provider description
     * @param requiresApiKey - Whether API key is needed
     */
    using ProviderCreator = std::function<GeocodingProvider*(QObject*)>;
    void registerProvider(const QString& id, ProviderCreator creator, const QString& displayName,
                          const QString& description, bool requiresApiKey);

    /**
     * Create provider instance by ID
     * @param id - Provider identifier
     * @param parent - Parent QObject
     * @return Provider instance or nullptr if not found
     */
    GeocodingProvider* createProvider(const QString& id, QObject* parent = nullptr) const;

    /**
     * Get list of available provider IDs
     */
    QStringList availableProviders() const;

    /**
     * Get provider metadata
     */
    struct ProviderInfo {
        QString id;
        QString displayName;
        QString description;
        bool requiresApiKey;
    };

    ProviderInfo getProviderInfo(const QString& id) const;
    QList<ProviderInfo> getAllProviderInfo() const;

    /**
     * Register all built-in providers
     */
    static void registerBuiltInProviders();

  private:
    GeocodingProviderFactory() = default;
    ~GeocodingProviderFactory() = default;
    GeocodingProviderFactory(const GeocodingProviderFactory&) = delete;
    GeocodingProviderFactory& operator=(const GeocodingProviderFactory&) = delete;

    struct ProviderRegistration {
        ProviderCreator creator;
        QString displayName;
        QString description;
        bool requiresApiKey;
    };

    QMap<QString, ProviderRegistration> providers_;
};

}  // namespace navigation
}  // namespace extensions
}  // namespace opencardev::crankshaft
