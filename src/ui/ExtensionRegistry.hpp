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
#include <QVariantMap>
#include <QVariantList>
#include <QQmlEngine>
#include <QJSEngine>

namespace openauto {
namespace extensions {
    class ExtensionManager;
}

namespace ui {

/**
 * ExtensionRegistry exposes registered extensions to QML.
 * 
 * This provides a secure bridge between extensions and QML UI:
 * - Extensions register UI components via UICapability
 * - ExtensionRegistry collects and exposes registered components
 * - QML loads extension UI components in isolated contexts
 * 
 * Usage in QML:
 *   Repeater {
 *       model: ExtensionRegistry.mainComponents
 *       Loader {
 *           source: modelData.qmlPath
 *       }
 *   }
 */
class ExtensionRegistry : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariantList mainComponents READ mainComponents NOTIFY mainComponentsChanged)
    Q_PROPERTY(QVariantList widgets READ widgets NOTIFY widgetsChanged)
    Q_PROPERTY(int componentCount READ componentCount NOTIFY componentCountChanged)

public:
    explicit ExtensionRegistry(
        extensions::ExtensionManager* extensionManager,
        QObject* parent = nullptr
    );
    
    /**
     * Get singleton instance (C++ access).
     */
    static ExtensionRegistry* instance();
    
    /**
     * Register as QML singleton.
     * Call this before creating QQmlEngine.
     */
    static void registerQmlType();
    
    /**
     * Get singleton instance for QML.
     */
    static QObject* qmlInstance(QQmlEngine* engine, QJSEngine* scriptEngine);
    
    /**
     * Register a UI component from an extension.
     * Called by UICapability implementations.
     * 
     * @param extensionId Extension registering the component
     * @param slotType Component slot type ("main", "widget", etc.)
     * @param qmlPath Path to QML file
     * @param metadata Component metadata (title, icon, etc.)
     */
    Q_INVOKABLE void registerComponent(
        const QString& extensionId,
        const QString& slotType,
        const QString& qmlPath,
        const QVariantMap& metadata
    );
    
    /**
     * Unregister a UI component.
     * 
     * @param componentId Component ID (returned from registerComponent)
     */
    Q_INVOKABLE void unregisterComponent(const QString& componentId);
    
    /**
     * Get all registered main view components.
     * 
     * @return List of component metadata maps
     */
    QVariantList mainComponents() const;
    
    /**
     * Get all registered widget components.
     * 
     * @return List of widget metadata maps
     */
    QVariantList widgets() const;
    
    /**
     * Get total number of registered components.
     */
    int componentCount() const;
    
    /**
     * Get component by ID.
     * 
     * @param componentId Component ID
     * @return Component metadata or empty map if not found
     */
    Q_INVOKABLE QVariantMap getComponent(const QString& componentId) const;

signals:
    void mainComponentsChanged();
    void widgetsChanged();
    void componentCountChanged();
    void componentRegistered(const QString& extensionId, const QString& componentId);
    void componentUnregistered(const QString& componentId);

private:
    struct ComponentInfo {
        QString component_id;
        QString extension_id;
        QString slot_type;
        QString qml_path;
        QVariantMap metadata;
    };
    
    extensions::ExtensionManager* extension_manager_;
    QList<ComponentInfo> components_;
    int next_component_id_;
    
    static ExtensionRegistry* instance_;
};

}  // namespace ui
}  // namespace openauto
