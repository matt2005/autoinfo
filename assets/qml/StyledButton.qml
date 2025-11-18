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

import QtQuick
import QtQuick.Controls
import CrankshaftReborn.UI 1.0

Button {
    id: control
    
    property string buttonType: "primary" // primary, secondary, accent, outline
    
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)
    
    padding: Theme.padding
    spacing: Theme.spacing / 2
    
    contentItem: Text {
        text: control.text
        font: control.font
        opacity: enabled ? 1.0 : 0.5
        color: getTextColor()
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    
    background: Rectangle {
        implicitWidth: 100
        implicitHeight: 40
        opacity: enabled ? 1 : 0.5
        color: getBackgroundColor()
        border.color: buttonType === "outline" ? Theme.primary : "transparent"
        border.width: buttonType === "outline" ? 2 : 0
        radius: Theme.cornerRadius
        
        Behavior on color {
            ColorAnimation { duration: 150 }
        }
    }
    
    function getBackgroundColor() {
        if (!control.enabled) {
            return Theme.surfaceVariant
        }
        
        if (control.down) {
            switch(buttonType) {
                case "primary": return Theme.primaryDark
                case "secondary": return Theme.secondaryDark
                case "accent": return Theme.accentDark
                case "outline": return Theme.alpha(Theme.primary, 0.1)
                default: return Theme.primaryDark
            }
        }
        
        if (control.hovered) {
            switch(buttonType) {
                case "primary": return Theme.primaryLight
                case "secondary": return Theme.secondaryLight
                case "accent": return Theme.accentLight
                case "outline": return Theme.alpha(Theme.primary, 0.05)
                default: return Theme.primaryLight
            }
        }
        
        switch(buttonType) {
            case "primary": return Theme.primary
            case "secondary": return Theme.secondary
            case "accent": return Theme.accent
            case "outline": return "transparent"
            default: return Theme.primary
        }
    }
    
    function getTextColor() {
        if (buttonType === "outline") {
            return Theme.primary
        }
        
        // For solid buttons, determine if we need light or dark text
        if (buttonType === "secondary") {
            return Theme.isDark ? Theme.background : Theme.text
        }
        
        return Theme.isDark ? Theme.background : Theme.surface
    }
}
