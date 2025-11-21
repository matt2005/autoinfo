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

#include <QString>
#include <QStringList>
#include <QVariantMap>

namespace openauto {
namespace extensions {

struct ExtensionManifest {
    QString id;
    QString name;
    QString version;
    QString description;
    QString author;
    QString type;  // service, ui, integration, platform
    
    QStringList dependencies;
    QStringList platforms;  // linux, windows, all
    
    QString entry_point;  // Path to the main executable/library
    QString config_schema;  // Path to JSON schema for configuration
    
    struct Requirements {
        QString min_core_version;
        QStringList required_permissions;
    } requirements;
    
    QVariantMap metadata;
    
    static ExtensionManifest fromJson(const QVariantMap& json);
    QVariantMap toJson() const;
    bool isValid() const;
};

}  // namespace extensions
}  // namespace openauto
