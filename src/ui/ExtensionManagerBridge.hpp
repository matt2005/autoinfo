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
#include <QVariantList>
#include <QVariantMap>
#include <qqml.h>

namespace opencardev { namespace crankshaft {
namespace extensions {
    class ExtensionManager;
}

namespace ui {

class ExtensionManagerBridge : public QObject {
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    static ExtensionManagerBridge* instance();
    static void registerQmlType();
    static void initialise(extensions::ExtensionManager* manager);

    Q_INVOKABLE QVariantList getLoadedExtensions() const;
    Q_INVOKABLE QVariantList getAvailableExtensions() const;
    Q_INVOKABLE QVariantMap getExtensionInfo(const QString& extensionId) const;
    Q_INVOKABLE bool reloadExtension(const QString& extensionId);
    Q_INVOKABLE bool enableExtension(const QString& extensionId);
    Q_INVOKABLE bool disableExtension(const QString& extensionId);
    Q_INVOKABLE bool isExtensionEnabled(const QString& extensionId) const;
    Q_INVOKABLE void refreshExtensions();

signals:
    void extensionLoaded(const QString& extensionId);
    void extensionUnloaded(const QString& extensionId);
    void extensionError(const QString& extensionId, const QString& error);
    void extensionsRefreshed();

private:
    explicit ExtensionManagerBridge(QObject* parent = nullptr);
    ~ExtensionManagerBridge() override = default;

    void connectSignals();

    static ExtensionManagerBridge* instance_;
    extensions::ExtensionManager* extension_manager_;
};

}  // namespace ui
}  // namespace crankshaft
}  // namespace opencardev
