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

/**
 * Navigation extension UI component
 * 
 * Demonstrates capability-based extension architecture:
 * - Extension uses LocationCapability for GPS
 * - Extension uses NetworkCapability for map tiles
 * - Extension uses EventCapability for communication
 * - Extension cannot directly access core services
 * 
 * NOTE: QtPositioning is available. QtLocation (for full map support) is not available
 * in Debian 12 - it would require building from source or upgrading to a newer release
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtPositioning

Item {
    id: root
    
    // Use ThemeManager (available as context property from main.cpp)
    readonly property color surfaceColor: ThemeManager.surfaceColor
    readonly property color surfaceVariant: ThemeManager.cardColor
    readonly property color accentColor: ThemeManager.accentColor
    readonly property color errorColor: ThemeManager.errorColor
    readonly property color textColor: ThemeManager.textColor
    readonly property color textSecondary: ThemeManager.textSecondaryColor
    readonly property color outlineColor: ThemeManager.borderColor
    readonly property int paddingSize: ThemeManager.padding
    readonly property int spacingSize: ThemeManager.spacing
    
    // Current location (updated by extension via events)
    property real currentLat: 51.5074
    property real currentLng: -0.1278
    property real destLat: 51.5074
    property real destLng: -0.1278
    property bool isNavigating: false
    property real distanceRemaining: 0
    property int etaSeconds: 0
    
    // Map placeholder with grid pattern
    Rectangle {
        id: mapPlaceholder
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: controlPanel.top
        }
        color: surfaceColor
        
        // Grid pattern to simulate map
        Grid {
            anchors.fill: parent
            columns: 10
            rows: 10
            Repeater {
                model: 100
                Rectangle {
                    width: mapPlaceholder.width / 10
                    height: mapPlaceholder.height / 10
                    color: "transparent"
                    border.color: outlineColor
                    border.width: 1
                    opacity: 0.2
                }
            }
        }
        
        // Centre content
        Column {
            anchors.centerIn: parent
            spacing: 20
            
            // Location marker
            Rectangle {
                width: 80
                height: 80
                radius: 40
                color: accentColor
                border.color: surfaceColor
                border.width: 4
                anchors.horizontalCenter: parent.horizontalCenter
                
                Text {
                    text: "📍"
                    font.pixelSize: 40
                    anchors.centerIn: parent
                }
            }
            
            // Title
            Text {
                text: "Navigation Extension"
                font.pixelSize: 28
                font.bold: true
                color: textColor
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            // Coordinates
            Column {
                spacing: 8
                anchors.horizontalCenter: parent.horizontalCenter
                
                Text {
                    text: "Latitude: " + currentLat.toFixed(6)
                    font.pixelSize: 16
                    color: textSecondary
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                
                Text {
                    text: "Longitude: " + currentLng.toFixed(6)
                    font.pixelSize: 16
                    color: textSecondary
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
            
            // Status
            Rectangle {
                width: 320
                height: 60
                radius: 8
                color: isNavigating ? accentColor : surfaceVariant
                opacity: 0.3
                anchors.horizontalCenter: parent.horizontalCenter
                
                Text {
                    text: isNavigating ? "🧭 Navigating..." : "📡 Ready"
                    font.pixelSize: 18
                    font.bold: true
                    color: textColor
                    anchors.centerIn: parent
                }
            }
            
            // Info message
            Text {
                text: "Using capability-based architecture\nLocation updates via LocationCapability"
                font.pixelSize: 14
                color: textSecondary
                opacity: 0.7
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
                lineHeight: 1.4
            }
            
            // Note about full features
            Rectangle {
                width: 400
                height: 50
                radius: 6
                color: surfaceVariant
                opacity: 0.5
                anchors.horizontalCenter: parent.horizontalCenter
                
                Text {
                    text: "Install qml6-module-qtlocation for full map view"
                    font.pixelSize: 12
                    color: textSecondary
                    anchors.centerIn: parent
                }
            }
        }
    }
    
    // Navigation controls
    Rectangle {
        id: controlPanel
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        height: 100
        color: surfaceVariant
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: paddingSize
            spacing: spacingSize
            
            // Start/Stop navigation button
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                text: isNavigating ? "Stop Navigation" : "Start Navigation"
                
                background: Rectangle {
                    color: isNavigating ? errorColor : accentColor
                    radius: 8
                }
                
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 18
                    font.bold: true
                    color: "white"
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    isNavigating = !isNavigating
                    if (isNavigating) {
                        distanceRemaining = 5000 // 5km example
                        etaSeconds = 600 // 10 minutes example
                    }
                }
            }
            
            // Settings button
            Button {
                Layout.preferredWidth: 60
                Layout.preferredHeight: 60
                text: "⚙️"
                
                background: Rectangle {
                    color: surfaceColor
                    radius: 8
                    border.color: outlineColor
                    border.width: 2
                }
                
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 24
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    console.log("Navigation settings clicked")
                }
            }
        }
    }
    
    // Navigation info overlay (when navigating)
    Rectangle {
        id: navInfo
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: isNavigating ? 80 : 0
        color: surfaceColor
        opacity: 0.95
        visible: height > 0
        
        Behavior on height {
            NumberAnimation { duration: 300; easing.type: Easing.OutQuad }
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: paddingSize
            spacing: spacingSize * 2
            visible: isNavigating
            
            // Distance
            Column {
                Layout.fillWidth: true
                spacing: 4
                
                Text {
                    text: (distanceRemaining / 1000).toFixed(1) + " km"
                    font.pixelSize: 24
                    font.bold: true
                    color: textColor
                }
                
                Text {
                    text: "Distance"
                    font.pixelSize: 12
                    color: textSecondary
                }
            }
            
            // ETA
            Column {
                Layout.fillWidth: true
                spacing: 4
                
                Text {
                    text: Math.floor(etaSeconds / 60) + " min"
                    font.pixelSize: 24
                    font.bold: true
                    color: textColor
                }
                
                Text {
                    text: "ETA"
                    font.pixelSize: 12
                    color: textSecondary
                }
            }
            
            // Speed (example)
            Column {
                Layout.fillWidth: true
                spacing: 4
                
                Text {
                    text: "50 km/h"
                    font.pixelSize: 24
                    font.bold: true
                    color: textColor
                }
                
                Text {
                    text: "Speed"
                    font.pixelSize: 12
                    color: textSecondary
                }
            }
        }
    }
    
    Component.onCompleted: {
        console.log("Navigation view loaded")
    }
}
