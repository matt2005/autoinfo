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

#include <QString>
#include <QColor>
#include <QObject>

namespace CrankshaftReborn {
namespace UI {

/**
 * @brief Theme data structure containing all theme colors and properties
 */
struct Theme {
    QString name;
    QString displayName;
    bool isDark{false};
    
    // Primary colors
    QColor primaryColor;
    QColor primaryLightColor;
    QColor primaryDarkColor;
    
    // Secondary colors
    QColor secondaryColor;
    QColor secondaryLightColor;
    QColor secondaryDarkColor;
    
    // Accent colors
    QColor accentColor;
    QColor accentLightColor;
    QColor accentDarkColor;
    
    // Background colors
    QColor backgroundColor;
    QColor surfaceColor;
    QColor cardColor;
    
    // Text colors
    QColor textColor;
    QColor textSecondaryColor;
    QColor textDisabledColor;
    
    // UI element colors
    QColor dividerColor;
    QColor borderColor;
    QColor shadowColor;
    
    // Status colors
    QColor successColor;
    QColor warningColor;
    QColor errorColor;
    QColor infoColor;
    
    // Additional properties
    int cornerRadius{8};
    int spacing{8};
    int padding{16};
    
    /**
     * @brief Load theme from JSON file
     * @param filePath Path to theme JSON file
     * @return Loaded theme or default theme on error
     */
    static Theme fromFile(const QString& filePath);
    
    /**
     * @brief Save theme to JSON file
     * @param filePath Path to save theme JSON
     * @return True if saved successfully
     */
    bool toFile(const QString& filePath) const;
    
    /**
     * @brief Create default light theme
     */
    static Theme defaultLight();
    
    /**
     * @brief Create default dark theme
     */
    static Theme defaultDark();
};

} // namespace UI
} // namespace CrankshaftReborn
