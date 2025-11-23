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

#include "extension_manager.hpp"
#include "../core/capabilities/CapabilityManager.hpp"
#include "../core/capabilities/Capability.hpp"
#include <QDir>
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QQueue>
#include "../core/config/ConfigManager.hpp"

namespace opencardev::crankshaft {
namespace extensions {

ExtensionManager::ExtensionManager(QObject *parent)
        : QObject(parent),
            capability_manager_(nullptr),
            config_manager_(nullptr),
            extensions_dir_("extensions") {
}

ExtensionManager::~ExtensionManager() {
    unloadAll();
}

void ExtensionManager::initialize(core::CapabilityManager* capability_manager, core::config::ConfigManager* config_manager) {
    capability_manager_ = capability_manager;
    config_manager_ = config_manager;
    qInfo() << "Extension manager initialized with capability-based security";
    // Prefer extensions located next to the executable by default
    const QString defaultExtDir = QCoreApplication::applicationDirPath() + "/extensions";
    if (QDir(defaultExtDir).exists()) {
        extensions_dir_ = defaultExtDir;
    }
    if (config_manager_) {
        QObject::connect(config_manager_, &core::config::ConfigManager::configValueChanged,
                         this, [this](const QString& domain, const QString& extension,
                                      const QString& section, const QString& key,
                                      const QVariant& value) {
            if (domain == "system" && extension == "extensions" && section == "manage") {
                const bool enable = value.toBool();
                if (enable) {
                    enableExtension(key);
                } else {
                    disableExtension(key);
                }
            }
        });
    }
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
    
    // Skip if already loaded (e.g., built-in extension already registered)
    if (extensions_.contains(manifest.id)) {
        qDebug() << "Extension already loaded, skipping:" << manifest.id;
        return true;
    }
    
    if (!validateManifest(manifest)) {
        qWarning() << "Manifest validation failed for:" << manifest.id;
        emit extensionError(manifest.id, "Manifest validation failed");
        return false;
    }
    
    if (!checkDependencies(manifest)) {
        qWarning() << "Dependency check failed for:" << manifest.id;
        emit extensionError(manifest.id, "Missing dependencies or not running");
        return false;
    }
    
    // TODO: Load the actual extension library/executable
    // This would involve loading shared libraries or spawning processes
    
    // For now, store the manifest for capability granting
    ExtensionInfo info;
    info.manifest = manifest;
    info.path = extension_path;
    // Dynamic loading not implemented yet, but mark as running so that
    // dependent extensions consider this satisfied. Replace with actual
    // load/start when plugin mechanism added.
    info.is_running = true;
    // Note: extension pointer will be set when actual extension is created
    extensions_[manifest.id] = info;
    
    qInfo() << "Extension loaded successfully:" << manifest.id;
    emit extensionLoaded(manifest.id);
    
    return true;
}

bool ExtensionManager::registerBuiltInExtension(std::shared_ptr<Extension> extension, const QString& extension_path) {
    if (!extension) {
        qWarning() << "Cannot register null extension";
        return false;
    }
    
    QString manifest_path = extension_path + "/manifest.json";
    ExtensionManifest manifest = loadManifest(manifest_path);
    
    if (!manifest.isValid()) {
        qWarning() << "Invalid manifest for built-in extension:" << extension_path;
        return false;
    }
    
    // Check if already loaded (manifest discovered but extension not instantiated)
    if (extensions_.contains(manifest.id)) {
        qDebug() << "Extension manifest already loaded, adding implementation:" << manifest.id;
        auto& info = extensions_[manifest.id];
        info.extension = extension;
        
        // Grant capabilities based on manifest
        grantCapabilities(extension.get(), manifest);
        
        // Initialize and start extension
        if (extension->initialize()) {
            // Register config items if ConfigManager is available
            if (config_manager_) {
                extension->registerConfigItems(config_manager_);
            }
            extension->start();
            info.is_running = true;
            qInfo() << "Built-in extension started:" << manifest.id;
            return true;
        } else {
            qWarning() << "Failed to initialize built-in extension:" << manifest.id;
            emit extensionError(manifest.id, "Initialization failed");
            return false;
        }
    }
    
    // Store extension info
    ExtensionInfo info;
    info.extension = extension;
    info.manifest = manifest;
    info.path = extension_path;
    info.is_running = false;
    extensions_[manifest.id] = info;
    
    // Grant capabilities based on manifest
    grantCapabilities(extension.get(), manifest);
    
    // Initialize and start extension
    if (extension->initialize()) {
        // Register config items if ConfigManager is available
        if (config_manager_) {
            extension->registerConfigItems(config_manager_);
        }
        bool shouldStart = true;
        if (config_manager_) {
            // Check system.extensions.manage.<id> toggle
            QVariant v = config_manager_->getValue("system", "extensions", "manage", manifest.id);
            if (v.isValid()) shouldStart = v.toBool();
        }
        if (shouldStart) {
            extension->start();
            extensions_[manifest.id].is_running = true;
            qInfo() << "Built-in extension registered and started:" << manifest.id;
        } else {
            extensions_[manifest.id].is_running = false;
            qInfo() << "Built-in extension registered but disabled by config:" << manifest.id;
        }
        emit extensionLoaded(manifest.id);
        return true;
    } else {
        qWarning() << "Failed to initialize built-in extension:" << manifest.id;
        extensions_.remove(manifest.id);
        emit extensionError(manifest.id, "Initialization failed");
        return false;
    }
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

bool ExtensionManager::enableExtension(const QString& extension_id) {
    if (!extensions_.contains(extension_id)) return false;
    auto &info = extensions_[extension_id];
    if (info.is_running) return true;
    if (!info.extension) return false;
    info.extension->start();
    info.is_running = true;
    qInfo() << "Enabled extension:" << extension_id;
    emit extensionLoaded(extension_id);
    return true;
}

bool ExtensionManager::disableExtension(const QString& extension_id) {
    if (!extensions_.contains(extension_id)) return false;
    auto &info = extensions_[extension_id];
    if (!info.is_running) return true;
    if (!info.extension) return false;
    info.extension->stop();
    info.is_running = false;
    qInfo() << "Disabled extension:" << extension_id;
    emit extensionUnloaded(extension_id);
    return true;
}

void ExtensionManager::loadAll() {
    // 1. Aggregate candidate directories (prefer runtime/app dirs over source)
    QStringList searchPaths;
    const QString envExt = qEnvironmentVariable("CRANKSHAFT_EXTENSIONS_PATH");
    if (!envExt.isEmpty()) searchPaths << envExt;
    if (!extensions_dir_.isEmpty()) searchPaths << extensions_dir_;
    // Application dir (e.g., installed bundle or build/bin layout)
    searchPaths << (QCoreApplication::applicationDirPath() + "/extensions");
    // System install paths
    searchPaths << QString::fromUtf8("/usr/share/CrankshaftReborn/extensions");
    searchPaths << QString::fromUtf8("/usr/share/crankshaft_reborn/extensions");
    // Optionally include source-tree extensions for developer runs
    const bool scanSource = qEnvironmentVariableIntValue("CRANKSHAFT_SCAN_SOURCE_EXTENSIONS") != 0;
    if (scanSource) {
        searchPaths << (QDir::currentPath() + "/extensions");
    }

    // 2. Discover paths (unique)
    QStringList extension_paths;
    for (const QString& dir : searchPaths) {
        const QStringList found = discoverExtensions(dir);
        for (const QString& p : found) {
            if (!extension_paths.contains(p)) extension_paths << p;
        }
    }

    if (extension_paths.isEmpty()) {
        qInfo() << "No extensions discovered for loading";
        return;
    }

    // 3. Load manifests first (do not instantiate yet)
    QMap<QString, ExtensionManifest> manifestsById;
    QMap<QString, QString> pathById;
    for (const QString& path : extension_paths) {
        const QString manifest_path = path + "/manifest.json";
        ExtensionManifest manifest = loadManifest(manifest_path);
        if (!manifest.isValid()) {
            qWarning() << "Skipping invalid manifest at" << manifest_path;
            continue;
        }
        if (!validateManifest(manifest)) {
            qWarning() << "Skipping manifest failing validation for" << manifest.id;
            continue;
        }
        // Avoid duplicate ids overriding earlier entries
        if (manifestsById.contains(manifest.id)) {
            qWarning() << "Duplicate extension id discovered, ignoring later instance:" << manifest.id;
            continue;
        }
        manifestsById.insert(manifest.id, manifest);
        pathById.insert(manifest.id, path);
    }

    if (manifestsById.isEmpty()) {
        qInfo() << "No valid manifests discovered";
        return;
    }

    // 4. Resolve dependency order (excluding already loaded built-ins)
    QSet<QString> alreadyLoaded;
    for (const QString& id : extensions_.keys()) {
        alreadyLoaded.insert(id);
    }
    QMap<QString, QStringList> missingDeps;
    QStringList cycleGroup;
    QStringList ordered = resolveLoadOrder(manifestsById, alreadyLoaded, missingDeps, cycleGroup);

    // 5. Report missing dependencies
    for (auto it = missingDeps.begin(); it != missingDeps.end(); ++it) {
        const QString& extId = it.key();
        const QStringList& deps = it.value();
        qWarning() << "Extension" << extId << "has missing dependencies" << deps;
        emit extensionError(extId, QString("Missing dependencies: %1").arg(deps.join(",")));
    }

    // 6. Report cycles
    if (!cycleGroup.isEmpty()) {
        qWarning() << "Dependency cycle detected among extensions:" << cycleGroup;
        for (const QString& extId : cycleGroup) {
            emit extensionError(extId, "Dependency cycle detected");
        }
    }

    // 7. Load in resolved order
    for (const QString& id : ordered) {
        if (alreadyLoaded.contains(id)) {
            qDebug() << "Extension already loaded (built-in), skipping explicit load:" << id;
            continue;
        }
        if (!pathById.contains(id)) {
            continue; // Should not happen
        }
        loadExtension(pathById.value(id));
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
    
    qDebug() << "Discovered" << extension_paths.size() << "extensions";
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
        if (!extensions_.contains(dep)) {
            qWarning() << "Missing dependency:" << dep << "for extension:" << manifest.id;
            return false;
        }
        const auto &info = extensions_[dep];
        if (!info.is_running) {
            qWarning() << "Dependency present but not running:" << dep << "required by:" << manifest.id;
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

void ExtensionManager::grantCapabilities(Extension* extension, const ExtensionManifest& manifest) {
    if (!capability_manager_) {
        qWarning() << "Cannot grant capabilities - CapabilityManager not initialized";
        return;
    }
    
    qInfo() << "Granting capabilities to extension:" << manifest.id;
    qDebug() << "  Requested permissions:" << manifest.requirements.required_permissions;
    
    for (const QString& permission : manifest.requirements.required_permissions) {
        qDebug() << "  Requesting capability:" << permission;
        auto capability = capability_manager_->grantCapability(extension->id(), permission);
        if (capability) {
            qDebug() << "  Calling extension->grantCapability for:" << permission;
            extension->grantCapability(capability);
            qDebug() << "  Granted capability:" << permission;
        } else {
            qWarning() << "  Failed to grant capability:" << permission;
        }
    }
    
    qDebug() << "  All capabilities granted, proceeding to audit log";
    
    // Log the capability grant for security audit
    capability_manager_->logCapabilityUsage(
        extension->id(),
        "extension_initialization",
        QString("Granted %1 capabilities based on manifest permissions").arg(manifest.requirements.required_permissions.size())
    );
}

QStringList ExtensionManager::resolveLoadOrder(const QMap<QString, ExtensionManifest>& manifests,
                                               const QSet<QString>& alreadyLoaded,
                                               QMap<QString, QStringList>& missingDeps,
                                               QStringList& cycleGroup) {
    // Kahn topological sort
    QMap<QString, int> indegree;
    QMap<QString, QStringList> adjacency;
    QSet<QString> candidates;
    for (const auto &k : manifests.keys()) {
        candidates.insert(k);
    }

    // First pass: record missing deps and build adjacency for satisfiable deps
    for (auto it = manifests.begin(); it != manifests.end(); ++it) {
        const QString id = it.key();
        indegree[id] = 0; // initialise
    }
    for (auto it = manifests.begin(); it != manifests.end(); ++it) {
        const QString id = it.key();
        const ExtensionManifest& manifest = it.value();
        for (const QString& dep : manifest.dependencies) {
            if (alreadyLoaded.contains(dep)) {
                // satisfied externally
                continue;
            }
            if (!manifests.contains(dep)) {
                // missing dependency
                missingDeps[id] << dep;
                continue;
            }
            adjacency[dep] << id; // dep -> id edge
            indegree[id] += 1;
        }
    }

    // Remove any candidates with missing dependencies from sorting set
    for (auto it = missingDeps.begin(); it != missingDeps.end(); ++it) {
        candidates.remove(it.key());
    }

    // Queue initial zero indegree nodes
    QQueue<QString> queue;
    for (const QString& id : candidates) {
        if (indegree[id] == 0) {
            queue.enqueue(id);
        }
    }

    QStringList order;
    while (!queue.isEmpty()) {
        QString node = queue.dequeue();
        order << node;
        for (const QString& next : adjacency.value(node)) {
            if (!candidates.contains(next)) {
                continue; // filtered out earlier
            }
            indegree[next] -= 1;
            if (indegree[next] == 0) {
                queue.enqueue(next);
            }
        }
    }

    // Any remaining nodes (with indegree > 0) form cycles
    QSet<QString> inCycle;
    for (const QString& id : candidates) {
        if (!order.contains(id)) {
            if (indegree[id] > 0) {
                inCycle.insert(id);
            }
        }
    }
    cycleGroup = inCycle.values();

    return order;
}

}  // namespace extensions
}  // namespace opencardev::crankshaft
