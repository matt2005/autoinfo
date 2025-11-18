/*
 * Project: OpenAuto
 * This file is part of openauto project.
 * Copyright (C) 2025 OpenCarDev Team
 *
 *  openauto is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  openauto is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with openauto. If not, see <http://www.gnu.org/licenses/>.
 */

#include "extension_manager.hpp"
#include <QDir>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>

namespace openauto {
namespace extensions {

ExtensionManager::ExtensionManager(QObject *parent)
        : QObject(parent),
            event_bus_(nullptr),
            ws_server_(nullptr),
            extensions_dir_("extensions") {
}

ExtensionManager::~ExtensionManager() {
    unloadAll();
}

void ExtensionManager::initialize(core::EventBus* event_bus, core::WebSocketServer* ws_server) {
    event_bus_ = event_bus;
    ws_server_ = ws_server;
    qInfo() << "Extension manager initialized";
}

bool ExtensionManager::loadExtension(const QString& extension_path) {
    qInfo() << "Loading extension from:" << extension_path;
    
    QString manifest_path = extension_path + "/manifest.json";
    ExtensionManifest manifest = loadManifest(manifest_path);
    
    if (!manifest.isValid()) {
        qWarning() << "Invalid manifest for extension:" << extension_path;
        emit extensionError(manifest.id, "Invalid manifest");
        return false;
    }
    
    if (!validateManifest(manifest)) {
        qWarning() << "Manifest validation failed for:" << manifest.id;
        emit extensionError(manifest.id, "Manifest validation failed");
        return false;
    }
    
    if (!checkDependencies(manifest)) {
        qWarning() << "Dependency check failed for:" << manifest.id;
        emit extensionError(manifest.id, "Missing dependencies");
        return false;
    }
    
    // TODO: Load the actual extension library/executable
    // This would involve loading shared libraries or spawning processes
    
    qInfo() << "Extension loaded successfully:" << manifest.id;
    emit extensionLoaded(manifest.id);
    
    return true;
}

bool ExtensionManager::unloadExtension(const QString& extension_id) {
    if (!extensions_.contains(extension_id)) {
        return false;
    }
    
    qInfo() << "Unloading extension:" << extension_id;
    
    auto& info = extensions_[extension_id];
    if (info.extension) {
        info.extension->stop();
        info.extension->cleanup();
    }
    
    extensions_.remove(extension_id);
    emit extensionUnloaded(extension_id);
    
    return true;
}

void ExtensionManager::loadAll() {
    // Build a list of candidate directories
    QStringList searchPaths;
    // Respect explicit dir if set externally
    if (!extensions_dir_.isEmpty()) {
        searchPaths << extensions_dir_;
    }
    // App dir (portable)
    searchPaths << (QCoreApplication::applicationDirPath() + "/extensions");
    // Current working dir (developer)
    searchPaths << (QDir::currentPath() + "/extensions");
    // System install
    searchPaths << QString::fromUtf8("/usr/share/CrankshaftReborn/extensions");
    searchPaths << QString::fromUtf8("/usr/share/crankshaft_reborn/extensions");
    // Optional override via env
    const QString envExt = qEnvironmentVariable("CRANKSHAFT_EXTENSIONS_PATH");
    if (!envExt.isEmpty()) {
        searchPaths.prepend(envExt);
    }

    QStringList extension_paths;
    for (const QString& dir : searchPaths) {
        qInfo() << "Loading all extensions from:" << dir;
        const QStringList found = discoverExtensions(dir);
        for (const QString& p : found) {
            if (!extension_paths.contains(p)) extension_paths << p;
        }
    }
    for (const QString& path : extension_paths) {
        loadExtension(path);
    }
}

void ExtensionManager::unloadAll() {
    qInfo() << "Unloading all extensions";
    
    QStringList ids = extensions_.keys();
    for (const QString& id : ids) {
        unloadExtension(id);
    }
}

bool ExtensionManager::isLoaded(const QString& extension_id) const {
    return extensions_.contains(extension_id);
}

QStringList ExtensionManager::getLoadedExtensions() const {
    return extensions_.keys();
}

ExtensionManifest ExtensionManager::getManifest(const QString& extension_id) const {
    if (extensions_.contains(extension_id)) {
        return extensions_[extension_id].manifest;
    }
    return ExtensionManifest();
}

QStringList ExtensionManager::discoverExtensions(const QString& search_path) {
    QStringList extension_paths;
    QDir dir(search_path);
    
    if (!dir.exists()) {
        qWarning() << "Extensions directory does not exist:" << search_path;
        return extension_paths;
    }
    
    QStringList subdirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString& subdir : subdirs) {
        QString manifest_path = dir.absoluteFilePath(subdir) + "/manifest.json";
        if (QFile::exists(manifest_path)) {
            extension_paths.append(dir.absoluteFilePath(subdir));
        }
    }
    
    qInfo() << "Discovered" << extension_paths.size() << "extensions";
    return extension_paths;
}

bool ExtensionManager::validateManifest(const ExtensionManifest& manifest) {
    // Validate version compatibility
    // Check platform compatibility
    // Validate permissions
    return true;
}

bool ExtensionManager::checkDependencies(const ExtensionManifest& manifest) {
    for (const QString& dep : manifest.dependencies) {
        if (!isLoaded(dep)) {
            qWarning() << "Missing dependency:" << dep << "for extension:" << manifest.id;
            return false;
        }
    }
    return true;
}

ExtensionManifest ExtensionManager::loadManifest(const QString& manifest_path) {
    QFile file(manifest_path);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open manifest file:" << manifest_path;
        return ExtensionManifest();
    }
    
    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    
    if (!doc.isObject()) {
        qWarning() << "Invalid JSON in manifest:" << manifest_path;
        return ExtensionManifest();
    }
    
    QVariantMap json = doc.object().toVariantMap();
    return ExtensionManifest::fromJson(json);
}

}  // namespace extensions
}  // namespace openauto
