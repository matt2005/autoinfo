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

#include "ConfigTypes.hpp"

namespace opencardev { namespace crankshaft {
namespace core { namespace config {

QString configComplexityToString(ConfigComplexity complexity) {
    switch (complexity) {
        case ConfigComplexity::Basic:
            return "basic";
        case ConfigComplexity::Advanced:
            return "advanced";
        case ConfigComplexity::Expert:
            return "expert";
        case ConfigComplexity::Developer:
            return "developer";
        default:
            return "basic";
    }
}

ConfigComplexity stringToConfigComplexity(const QString& str) {
    QString lower = str.toLower();
    if (lower == "advanced") return ConfigComplexity::Advanced;
    if (lower == "expert") return ConfigComplexity::Expert;
    if (lower == "developer" || lower == "dev") return ConfigComplexity::Developer;
    return ConfigComplexity::Basic;
}

QString configItemTypeToString(ConfigItemType type) {
    switch (type) {
        case ConfigItemType::Boolean: return "boolean";
        case ConfigItemType::Integer: return "integer";
        case ConfigItemType::Double: return "double";
        case ConfigItemType::String: return "string";
        case ConfigItemType::Selection: return "selection";
        case ConfigItemType::MultiSelection: return "multiselection";
        case ConfigItemType::Color: return "color";
        case ConfigItemType::File: return "file";
        case ConfigItemType::Directory: return "directory";
        case ConfigItemType::Custom: return "custom";
        default: return "string";
    }
}

ConfigItemType stringToConfigItemType(const QString& str) {
    QString lower = str.toLower();
    if (lower == "boolean" || lower == "bool") return ConfigItemType::Boolean;
    if (lower == "integer" || lower == "int") return ConfigItemType::Integer;
    if (lower == "double" || lower == "float") return ConfigItemType::Double;
    if (lower == "selection" || lower == "select") return ConfigItemType::Selection;
    if (lower == "multiselection") return ConfigItemType::MultiSelection;
    if (lower == "color") return ConfigItemType::Color;
    if (lower == "file") return ConfigItemType::File;
    if (lower == "directory" || lower == "dir") return ConfigItemType::Directory;
    if (lower == "custom") return ConfigItemType::Custom;
    return ConfigItemType::String;
}

QVariantMap ConfigItem::toMap() const {
    QVariantMap map;
    map["key"] = key;
    map["label"] = label;
    map["description"] = description;
    map["type"] = configItemTypeToString(type);
    map["defaultValue"] = defaultValue;
    map["currentValue"] = currentValue;
    map["complexity"] = configComplexityToString(complexity);
    map["properties"] = properties;
    map["required"] = required;
    map["validator"] = validator;
    map["icon"] = icon;
    map["unit"] = unit;
    map["readOnly"] = readOnly;
    map["isSecret"] = isSecret;
    return map;
}

ConfigItem ConfigItem::fromMap(const QVariantMap& map) {
    ConfigItem item;
    item.key = map.value("key").toString();
    item.label = map.value("label").toString();
    item.description = map.value("description").toString();
    item.type = stringToConfigItemType(map.value("type").toString());
    item.defaultValue = map.value("defaultValue");
    item.currentValue = map.value("currentValue", map.value("defaultValue"));
    item.complexity = stringToConfigComplexity(map.value("complexity").toString());
    item.properties = map.value("properties").toMap();
    item.required = map.value("required").toBool();
    item.validator = map.value("validator").toString();
    item.icon = map.value("icon").toString();
    item.unit = map.value("unit").toString();
    item.readOnly = map.value("readOnly").toBool();
    item.isSecret = map.value("isSecret").toBool();
    return item;
}

QVariantMap ConfigSection::toMap() const {
    QVariantMap map;
    map["key"] = key;
    map["title"] = title;
    map["description"] = description;
    map["icon"] = icon;
    map["complexity"] = configComplexityToString(complexity);
    
    QVariantList itemsList;
    for (const ConfigItem& item : items) {
        itemsList.append(item.toMap());
    }
    map["items"] = itemsList;
    
    return map;
}

ConfigSection ConfigSection::fromMap(const QVariantMap& map) {
    ConfigSection section;
    section.key = map.value("key").toString();
    section.title = map.value("title").toString();
    section.description = map.value("description").toString();
    section.icon = map.value("icon").toString();
    section.complexity = stringToConfigComplexity(map.value("complexity").toString());
    
    QVariantList itemsList = map.value("items").toList();
    for (const QVariant& itemVar : itemsList) {
        section.items.append(ConfigItem::fromMap(itemVar.toMap()));
    }
    
    return section;
}

QVariantMap ConfigPage::toMap() const {
    QVariantMap map;
    map["domain"] = domain;
    map["extension"] = extension;
    map["title"] = title;
    map["description"] = description;
    map["icon"] = icon;
    map["complexity"] = configComplexityToString(complexity);
    
    QVariantList sectionsList;
    for (const ConfigSection& section : sections) {
        sectionsList.append(section.toMap());
    }
    map["sections"] = sectionsList;
    
    return map;
}

ConfigPage ConfigPage::fromMap(const QVariantMap& map) {
    ConfigPage page;
    page.domain = map.value("domain").toString();
    page.extension = map.value("extension").toString();
    page.title = map.value("title").toString();
    page.description = map.value("description").toString();
    page.icon = map.value("icon").toString();
    page.complexity = stringToConfigComplexity(map.value("complexity").toString());
    
    QVariantList sectionsList = map.value("sections").toList();
    for (const QVariant& sectionVar : sectionsList) {
        page.sections.append(ConfigSection::fromMap(sectionVar.toMap()));
    }
    
    return page;
}

}  // namespace config
}  // namespace core
}  // namespace crankshaft
}  // namespace opencardev
