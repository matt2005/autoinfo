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

#include "Capability.hpp"
#include <QString>
#include <QVariantMap>

namespace openauto {
namespace core {
namespace capabilities {

/**
 * UI capability for registering user interface components.
 * 
 * Extensions with this capability can:
 * - Register main view components (full-screen tabs)
 * - Register widget components (dashboard widgets, notifications)
 * - Show notifications
 * - Update status bar information
 * 
 * Extensions cannot directly access QML engine or Qt Quick APIs.
 */
class UICapability : public Capability {
public:
    virtual ~UICapability() = default;
    
    QString id() const override { return "ui"; }
    
    /**
     * UI component slot types.
     */
    enum class SlotType {
        MainView,      // Full-screen primary view (e.g., navigation map)
        Widget,        // Dashboard widget (e.g., speed widget)
        Notification,  // Temporary notification
        StatusBar,     // Status bar item (e.g., GPS indicator)
        QuickAction    // Quick action button
    };
    
    /**
     * Register a main view component.
     * Main views appear as tabs in the primary navigation.
     * 
     * @param qmlPath Path to QML file (relative to extension assets or qrc:/)
     * @param metadata Component metadata (title, icon, etc.)
     * @return true if registered successfully
     */
    virtual bool registerMainView(const QString& qmlPath, const QVariantMap& metadata) = 0;
    
    /**
     * Register a widget component.
     * Widgets can appear on dashboard or other widget containers.
     * 
     * @param qmlPath Path to QML file
     * @param metadata Widget metadata (size, position hints, etc.)
     * @return true if registered successfully
     */
    virtual bool registerWidget(const QString& qmlPath, const QVariantMap& metadata) = 0;
    
    /**
     * Show a temporary notification.
     * 
     * @param title Notification title
     * @param message Notification message
     * @param duration Duration in milliseconds (0 = default)
     * @param icon Optional icon name or path
     */
    virtual void showNotification(
        const QString& title,
        const QString& message,
        int duration = 0,
        const QString& icon = QString()
    ) = 0;
    
    /**
     * Update status bar item.
     * 
     * @param itemId Unique item identifier
     * @param text Status text
     * @param icon Optional icon
     */
    virtual void updateStatusBar(
        const QString& itemId,
        const QString& text,
        const QString& icon = QString()
    ) = 0;
    
    /**
     * Unregister a UI component.
     * 
     * @param componentId Component identifier (from registration)
     */
    virtual void unregisterComponent(const QString& componentId) = 0;

protected:
    UICapability() = default;
};

}  // namespace capabilities
}  // namespace core
}  // namespace openauto
