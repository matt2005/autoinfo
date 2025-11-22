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

#include <QObject>
#include <QString>
#include <QMap>
#include <QSet>
#include <QStringList>
#include <memory>
#include "extension.hpp"
#include "extension_manifest.hpp"

namespace openauto {
namespace core {
    class EventBus;
    class WebSocketServer;
    class CapabilityManager;
}

namespace extensions {

class ExtensionManager : public QObject {
    Q_OBJECT

public:
    explicit ExtensionManager(QObject *parent = nullptr);
    ~ExtensionManager() override;

    void initialize(core::CapabilityManager* capability_manager);
    
    // Extension lifecycle
    bool loadExtension(const QString& extension_path);
    bool registerBuiltInExtension(std::shared_ptr<Extension> extension, const QString& extension_path);
    bool unloadExtension(const QString& extension_id);
    void loadAll();
    void unloadAll();
    
    // Extension queries
    bool isLoaded(const QString& extension_id) const;
    QStringList getLoadedExtensions() const;
    ExtensionManifest getManifest(const QString& extension_id) const;
    
    // Extension discovery
    QStringList discoverExtensions(const QString& search_path);

signals:
    void extensionLoaded(const QString& extension_id);
    void extensionUnloaded(const QString& extension_id);
    void extensionError(const QString& extension_id, const QString& error);

private:
    struct ExtensionInfo {
        std::shared_ptr<Extension> extension;
        ExtensionManifest manifest;
        QString path;
        bool is_running;
        
        // Make the struct copyable
        ExtensionInfo() : extension(nullptr), is_running(false) {}
        ExtensionInfo(const ExtensionInfo&) = default;
        ExtensionInfo& operator=(const ExtensionInfo&) = default;
        ExtensionInfo(ExtensionInfo&&) = default;
        ExtensionInfo& operator=(ExtensionInfo&&) = default;
    };
    
    bool validateManifest(const ExtensionManifest& manifest);
    bool checkDependencies(const ExtensionManifest& manifest);
    ExtensionManifest loadManifest(const QString& manifest_path);
    void grantCapabilities(Extension* extension, const ExtensionManifest& manifest);
    // Resolve a safe load order using topological sort. Returns ordered list of ids.
    // Populates missingDeps with any extension -> missing dependency list.
    // Populates cycleGroup with extensions participating in a dependency cycle.
    QStringList resolveLoadOrder(const QMap<QString, ExtensionManifest>& manifests,
                                 const QSet<QString>& alreadyLoaded,
                                 QMap<QString, QStringList>& missingDeps,
                                 QStringList& cycleGroup);
    
    QMap<QString, ExtensionInfo> extensions_;
    core::CapabilityManager* capability_manager_;
    QString extensions_dir_;
};

}  // namespace extensions
}  // namespace openauto
