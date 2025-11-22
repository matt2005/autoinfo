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

#include "extension_manifest.hpp"

namespace opencardev::crankshaft {
namespace extensions {

ExtensionManifest ExtensionManifest::fromJson(const QVariantMap& json) {
    ExtensionManifest manifest;
    
    manifest.id = json.value("id").toString();
    manifest.name = json.value("name").toString();
    manifest.version = json.value("version").toString();
    manifest.description = json.value("description").toString();
    manifest.author = json.value("author").toString();
    manifest.type = json.value("type").toString();
    
    QVariantList deps = json.value("dependencies").toList();
    for (const auto& dep : deps) {
        manifest.dependencies.append(dep.toString());
    }
    
    QVariantList platforms = json.value("platforms").toList();
    for (const auto& platform : platforms) {
        manifest.platforms.append(platform.toString());
    }
    
    manifest.entry_point = json.value("entry_point").toString();
    manifest.config_schema = json.value("config_schema").toString();
    
    QVariantMap requirements = json.value("requirements").toMap();
    manifest.requirements.min_core_version = requirements.value("min_core_version").toString();
    
    QVariantList permissions = requirements.value("required_permissions").toList();
    for (const auto& perm : permissions) {
        manifest.requirements.required_permissions.append(perm.toString());
    }
    
    manifest.metadata = json.value("metadata").toMap();
    
    return manifest;
}

QVariantMap ExtensionManifest::toJson() const {
    QVariantMap json;
    
    json["id"] = id;
    json["name"] = name;
    json["version"] = version;
    json["description"] = description;
    json["author"] = author;
    json["type"] = type;
    
    QVariantList deps;
    for (const auto& dep : dependencies) {
        deps.append(dep);
    }
    json["dependencies"] = deps;
    
    QVariantList plat;
    for (const auto& platform : platforms) {
        plat.append(platform);
    }
    json["platforms"] = plat;
    
    json["entry_point"] = entry_point;
    json["config_schema"] = config_schema;
    
    QVariantMap reqs;
    reqs["min_core_version"] = requirements.min_core_version;
    
    QVariantList perms;
    for (const auto& perm : requirements.required_permissions) {
        perms.append(perm);
    }
    reqs["required_permissions"] = perms;
    
    json["requirements"] = reqs;
    json["metadata"] = metadata;
    
    return json;
}

bool ExtensionManifest::isValid() const {
    // Basic validation: id, name and version are required
    // entry_point and type can be optional for declarative extensions
    return !id.isEmpty() && 
           !name.isEmpty() && 
           !version.isEmpty();
}

}  // namespace extensions
}  // namespace opencardev::crankshaft
