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
#include <QVariant>
#include <QVariantMap>

namespace opencardev {
namespace crankshaft {
namespace core {
namespace config {

enum class ConfigComplexity {
    Basic = 0,     // Simple on/off, basic settings
    Advanced = 1,  // More detailed configuration
    Expert = 2,    // Technical configuration
    Developer = 3  // Debug and development options
};

enum class ConfigItemType {
    Boolean,         // true/false toggle
    Integer,         // Integer input
    Double,          // Floating point input
    String,          // Text input
    Selection,       // Dropdown/radio selection
    MultiSelection,  // Multiple selection (checkboxes)
    Color,           // Color picker
    File,            // File selector
    Directory,       // Directory selector
    Custom           // Custom UI component
};

struct ConfigItem {
    QString key;                  // Unique key (e.g., "autoconnect")
    QString label;                // Display label
    QString description;          // Help text
    ConfigItemType type;          // Input type
    QVariant defaultValue;        // Default value
    QVariant currentValue;        // Current value
    ConfigComplexity complexity;  // Minimum complexity level to show

    // Type-specific properties
    QVariantMap properties;  // Min/max for numbers, options for selection, etc.

    // Validation
    bool required;
    QString validator;  // Regex or validation rule

    // UI hints
    QString icon;
    QString unit;  // Unit label (%, ms, etc.)
    bool readOnly;
    bool isSecret;  // Should be masked in exports (passwords, tokens, etc.)

    ConfigItem()
        : type(ConfigItemType::String),
          complexity(ConfigComplexity::Basic),
          required(false),
          readOnly(false),
          isSecret(false) {}

    QVariantMap toMap() const;
    static ConfigItem fromMap(const QVariantMap& map);
};

struct ConfigSection {
    QString key;                  // Section key (e.g., "connection")
    QString title;                // Section title
    QString description;          // Section description
    QString icon;                 // Section icon
    ConfigComplexity complexity;  // Minimum complexity level to show section
    QList<ConfigItem> items;      // Configuration items in this section

    ConfigSection() : complexity(ConfigComplexity::Basic) {}

    QVariantMap toMap() const;
    static ConfigSection fromMap(const QVariantMap& map);
};

struct ConfigPage {
    QString domain;                 // Domain (core/thirdparty)
    QString extension;              // Extension ID
    QString title;                  // Page title
    QString description;            // Page description
    QString icon;                   // Page icon
    ConfigComplexity complexity;    // Minimum complexity level to show page
    QList<ConfigSection> sections;  // Configuration sections

    ConfigPage() : complexity(ConfigComplexity::Basic) {}

    // Helper to get full key: domain.extension
    QString getFullKey() const { return domain + "." + extension; }

    QVariantMap toMap() const;
    static ConfigPage fromMap(const QVariantMap& map);
};

// Helper functions
QString configComplexityToString(ConfigComplexity complexity);
ConfigComplexity stringToConfigComplexity(const QString& str);
QString configItemTypeToString(ConfigItemType type);
ConfigItemType stringToConfigItemType(const QString& str);

}  // namespace config
}  // namespace core
}  // namespace crankshaft
}  // namespace opencardev
