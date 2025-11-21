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

pragma Singleton
import QtQuick
import CrankshaftReborn.UI 1.0

QtObject {
    id: theme
    
    // Access to ThemeManager singleton
    readonly property var manager: ThemeManager
    
    // Colors - Primary
    readonly property color primary: manager.primaryColor
    readonly property color primaryLight: manager.primaryLightColor
    readonly property color primaryDark: manager.primaryDarkColor
    
    // Colors - Secondary
    readonly property color secondary: manager.secondaryColor
    
    // Colors - Accent
    readonly property color accent: manager.accentColor
    
    // Colors - Background
    readonly property color background: manager.backgroundColor
    readonly property color surface: manager.surfaceColor
    readonly property color card: manager.cardColor
    
    // Colors - Text
    readonly property color text: manager.textColor
    readonly property color textSecondary: manager.textSecondaryColor
    
    // Colors - Status
    readonly property color error: manager.errorColor
    readonly property color warning: manager.warningColor
    readonly property color success: manager.successColor
    readonly property color info: manager.infoColor
    
    // Colors - Other
    readonly property color divider: manager.dividerColor
    readonly property color border: manager.borderColor
    
    // Properties
    readonly property int cornerRadius: manager.cornerRadius
    readonly property int spacing: manager.spacing
    readonly property int padding: manager.padding
    
    // State
    readonly property bool isDark: manager.isDark
    readonly property string currentTheme: manager.currentTheme
    readonly property var availableThemes: manager.availableThemes
    
    // Methods
    function setTheme(themeName) {
        manager.setCurrentTheme(themeName)
    }
    
    function toggleTheme() {
        manager.toggleTheme()
    }
    
    // Helper functions for common UI patterns
    function alpha(color, opacity) {
        return Qt.rgba(color.r, color.g, color.b, opacity)
    }
    
    function lighten(color, factor) {
        return Qt.lighter(color, factor)
    }
    
    function darken(color, factor) {
        return Qt.darker(color, factor)
    }
}
