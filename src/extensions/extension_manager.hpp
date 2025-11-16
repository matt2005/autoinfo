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

#pragma once

#include <QObject>
#include <QString>
#include <QMap>
#include <memory>
#include "extension.hpp"
#include "extension_manifest.hpp"

namespace openauto {
namespace core {
    class EventBus;
    class WebSocketServer;
}

namespace extensions {

class ExtensionManager : public QObject {
    Q_OBJECT

public:
    explicit ExtensionManager(QObject *parent = nullptr);
    ~ExtensionManager() override;

    void initialize(core::EventBus* event_bus, core::WebSocketServer* ws_server);
    
    // Extension lifecycle
    bool loadExtension(const QString& extension_path);
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
        std::unique_ptr<Extension> extension;
        ExtensionManifest manifest;
        QString path;
        bool is_running;
    };
    
    bool validateManifest(const ExtensionManifest& manifest);
    bool checkDependencies(const ExtensionManifest& manifest);
    ExtensionManifest loadManifest(const QString& manifest_path);
    
    QMap<QString, ExtensionInfo> extensions_;
    core::EventBus* event_bus_;
    core::WebSocketServer* ws_server_;
    QString extensions_dir_;
};

}  // namespace extensions
}  // namespace openauto
