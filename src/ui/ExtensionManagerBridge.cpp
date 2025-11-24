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

#include "ExtensionManagerBridge.hpp"
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlEngine>
#include "../extensions/extension_manager.hpp"
#include "../extensions/extension_manifest.hpp"

namespace opencardev {
namespace crankshaft {
namespace ui {

ExtensionManagerBridge* ExtensionManagerBridge::instance_ = nullptr;

ExtensionManagerBridge* ExtensionManagerBridge::instance() {
    if (!instance_) {
        instance_ = new ExtensionManagerBridge();
    }
    return instance_;
}

void ExtensionManagerBridge::registerQmlType() {
    qmlRegisterSingletonType<ExtensionManagerBridge>(
        "CrankshaftReborn.UI", 1, 0, "ExtensionManagerBridge",
        [](QQmlEngine*, QJSEngine*) -> QObject* { return ExtensionManagerBridge::instance(); });
}

void ExtensionManagerBridge::initialise(extensions::ExtensionManager* manager) {
    instance()->extension_manager_ = manager;
    instance()->connectSignals();
    qInfo() << "ExtensionManagerBridge initialized";
}

ExtensionManagerBridge::ExtensionManagerBridge(QObject* parent)
    : QObject(parent), extension_manager_(nullptr) {}

void ExtensionManagerBridge::connectSignals() {
    if (!extension_manager_) {
        return;
    }

    connect(extension_manager_, &extensions::ExtensionManager::extensionLoaded, this,
            &ExtensionManagerBridge::extensionLoaded);
    connect(extension_manager_, &extensions::ExtensionManager::extensionUnloaded, this,
            &ExtensionManagerBridge::extensionUnloaded);
    connect(extension_manager_, &extensions::ExtensionManager::extensionError, this,
            &ExtensionManagerBridge::extensionError);
}

QVariantList ExtensionManagerBridge::getLoadedExtensions() const {
    QVariantList result;
    if (!extension_manager_) {
        return result;
    }

    QStringList loaded = extension_manager_->getLoadedExtensions();
    for (const QString& id : loaded) {
        extensions::ExtensionManifest manifest = extension_manager_->getManifest(id);
        QVariantMap extInfo;
        extInfo["id"] = manifest.id;
        extInfo["name"] = manifest.name;
        extInfo["version"] = manifest.version;
        extInfo["description"] = manifest.description;
        extInfo["author"] = manifest.author;
        extInfo["type"] = manifest.type;
        extInfo["domain"] = extensions::extensionDomainToString(manifest.domain);
        extInfo["enabled"] = extension_manager_->isExtensionEnabled(manifest.id);
        extInfo["loaded"] = true;
        result.append(extInfo);
    }

    return result;
}

QVariantList ExtensionManagerBridge::getAvailableExtensions() const {
    QVariantList result;
    if (!extension_manager_) {
        return result;
    }

    QStringList searchPaths = extension_manager_->getExtensionSearchPaths();
    QSet<QString> discoveredIds;

    for (const QString& path : searchPaths) {
        QStringList extensions = extension_manager_->discoverExtensions(path);
        for (const QString& extPath : extensions) {
            QString manifestPath = extPath + "/manifest.json";
            QFile file(manifestPath);
            if (!file.open(QIODevice::ReadOnly)) {
                continue;
            }

            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            file.close();

            if (!doc.isObject()) {
                continue;
            }

            QVariantMap json = doc.object().toVariantMap();
            extensions::ExtensionManifest manifest = extensions::ExtensionManifest::fromJson(json);

            if (!manifest.isValid() || discoveredIds.contains(manifest.id)) {
                continue;
            }

            discoveredIds.insert(manifest.id);

            QVariantMap extInfo;
            extInfo["id"] = manifest.id;
            extInfo["name"] = manifest.name;
            extInfo["version"] = manifest.version;
            extInfo["description"] = manifest.description;
            extInfo["author"] = manifest.author;
            extInfo["type"] = manifest.type;
            extInfo["domain"] = extensions::extensionDomainToString(manifest.domain);
            extInfo["enabled"] = extension_manager_->isExtensionEnabled(manifest.id);
            extInfo["loaded"] = extension_manager_->isLoaded(manifest.id);
            extInfo["path"] = extPath;
            result.append(extInfo);
        }
    }

    return result;
}

QVariantMap ExtensionManagerBridge::getExtensionInfo(const QString& extensionId) const {
    QVariantMap result;
    if (!extension_manager_ || !extension_manager_->isLoaded(extensionId)) {
        return result;
    }

    extensions::ExtensionManifest manifest = extension_manager_->getManifest(extensionId);
    result["id"] = manifest.id;
    result["name"] = manifest.name;
    result["version"] = manifest.version;
    result["description"] = manifest.description;
    result["author"] = manifest.author;
    result["type"] = manifest.type;
    result["domain"] = extensions::extensionDomainToString(manifest.domain);
    result["dependencies"] = QVariant::fromValue(manifest.dependencies);
    result["platforms"] = QVariant::fromValue(manifest.platforms);
    result["permissions"] = QVariant::fromValue(manifest.requirements.required_permissions);
    result["enabled"] = extension_manager_->isExtensionEnabled(manifest.id);
    result["loaded"] = true;

    return result;
}

bool ExtensionManagerBridge::reloadExtension(const QString& extensionId) {
    if (!extension_manager_) {
        return false;
    }

    return extension_manager_->reloadExtension(extensionId);
}

bool ExtensionManagerBridge::enableExtension(const QString& extensionId) {
    if (!extension_manager_) {
        return false;
    }

    bool success = extension_manager_->enableExtension(extensionId);
    if (success) {
        emit extensionsRefreshed();
    }
    return success;
}

bool ExtensionManagerBridge::disableExtension(const QString& extensionId) {
    if (!extension_manager_) {
        return false;
    }

    bool success = extension_manager_->disableExtension(extensionId);
    if (success) {
        emit extensionsRefreshed();
    }
    return success;
}

bool ExtensionManagerBridge::isExtensionEnabled(const QString& extensionId) const {
    if (!extension_manager_) {
        return false;
    }

    return extension_manager_->isExtensionEnabled(extensionId);
}

void ExtensionManagerBridge::refreshExtensions() {
    emit extensionsRefreshed();
}

}  // namespace ui
}  // namespace crankshaft
}  // namespace opencardev
