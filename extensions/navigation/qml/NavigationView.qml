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

/**
 * Navigation extension UI component
 * 
 * Demonstrates capability-based extension architecture:
 * - Extension uses LocationCapability for GPS
 * - Extension uses NetworkCapability for map tiles
 * - Extension uses EventCapability for communication
 * - Extension cannot directly access core services
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtPositioning
import QtLocation

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
    readonly property color dividerColor: ThemeManager.dividerColor
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
    property var routeCoordinates: []
    property string currentInstruction: ""
    property string nextInstruction: ""
    property real distanceToNextTurn: 0
    
    // Settings (now managed via ConfigManager)
    property bool showSpeedLimit: true
    property bool voiceGuidance: true
    property bool avoidTolls: false
    property bool avoidMotorways: false
    property string distanceUnit: "metric" // "metric" or "imperial"
    // GPS hardware settings
    property var gpsDevices: ["Internal", "USB Receiver", "GNSS Hat", "Mock (Static)", "Mock (IP)"]
    property string selectedGpsDevice: "Internal"
    signal gpsDeviceChanged(string device)
    
    // OSM Map Plugin
    Plugin {
        id: mapPlugin
        name: "osm"
        PluginParameter {
            name: "osm.mapping.providersrepository.disabled"
            value: "true"
        }
        PluginParameter {
            name: "osm.mapping.providersrepository.address"
            value: "http://maps-redirect.qt.io/osm/5.6/"
        }
    }
    
    // Main map view
    Map {
        id: map
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: controlPanel.top
        }
        plugin: mapPlugin
        center: QtPositioning.coordinate(root.currentLat, root.currentLng)
        zoomLevel: root.isNavigating ? 16 : 14
        
        // Smooth animations
        Behavior on center {
            CoordinateAnimation { duration: 500; easing.type: Easing.InOutQuad }
        }
        Behavior on zoomLevel {
            NumberAnimation { duration: 300 }
        }
        
        // Current position marker
        MapQuickItem {
            id: positionMarker
            coordinate: QtPositioning.coordinate(root.currentLat, root.currentLng)
            anchorPoint.x: locationPin.width / 2
            anchorPoint.y: locationPin.height
            
            sourceItem: Item {
                id: locationPin
                width: 40
                height: 40
                
                // Pulsing circle
                Rectangle {
                    anchors.centerIn: parent
                    width: 60
                    height: 60
                    radius: 30
                    color: root.accentColor
                    opacity: 0.3
                    
                    SequentialAnimation on scale {
                        running: true
                        loops: Animation.Infinite
                        NumberAnimation { from: 1.0; to: 1.5; duration: 1000 }
                        NumberAnimation { from: 1.5; to: 1.0; duration: 1000 }
                    }
                }
                
                // Position dot
                Rectangle {
                    anchors.centerIn: parent
                    width: 20
                    height: 20
                    radius: 10
                    color: root.accentColor
                    border.color: root.surfaceColor
                    border.width: 3
                }
                
                // Direction indicator (when navigating)
                Canvas {
                    visible: root.isNavigating
                    anchors.centerIn: parent
                    width: 40
                    height: 40
                    onPaint: {
                        var ctx = getContext("2d")
                        ctx.reset()
                        ctx.fillStyle = root.accentColor
                        ctx.beginPath()
                        ctx.moveTo(20, 5)
                        ctx.lineTo(25, 15)
                        ctx.lineTo(15, 15)
                        ctx.closePath()
                        ctx.fill()
                    }
                }
            }
        }
        
        // Destination marker
        MapQuickItem {
            id: destinationMarker
            visible: root.isNavigating
            coordinate: QtPositioning.coordinate(root.destLat, root.destLng)
            anchorPoint.x: destPin.width / 2
            anchorPoint.y: destPin.height
            
            sourceItem: Item {
                id: destPin
                width: 50
                height: 60
                
                // Pin shape
                Rectangle {
                    width: 50
                    height: 50
                    radius: 25
                    color: root.errorColor
                    border.color: root.surfaceColor
                    border.width: 3
                    
                    Text {
                        text: "🏁"
                        font.pixelSize: 30
                        anchors.centerIn: parent
                    }
                }
                
                // Pin point
                Rectangle {
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.top: parent.top
                    anchors.topMargin: 45
                    width: 8
                    height: 15
                    color: root.errorColor
                    
                    Rectangle {
                        anchors.bottom: parent.bottom
                        anchors.horizontalCenter: parent.horizontalCenter
                        width: 4
                        height: 4
                        radius: 2
                        color: root.errorColor
                    }
                }
            }
        }
        
        // Route line
        MapPolyline {
            id: routeLine
            visible: root.isNavigating && root.routeCoordinates.length > 0
            line.width: 5
            line.color: root.accentColor
            path: root.routeCoordinates
        }
    }
    
    // Destination search overlay
    Rectangle {
        id: searchOverlay
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: controlPanel.top
        }
        color: root.surfaceColor
        opacity: 0.98
        visible: searchOverlayVisible
        z: 10
        
        property bool searchOverlayVisible: false
        
        Column {
            anchors.fill: parent
            spacing: 0
            
            // Header
            Rectangle {
                width: parent.width
                height: 60
                color: root.surfaceVariant
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: root.paddingSize
                    spacing: root.spacingSize
                    
                    Text {
                        text: "📍 Set Destination"
                        font.pixelSize: 20
                        font.bold: true
                        color: root.textColor
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    Button {
                        width: 50
                        height: 50
                        text: "✕"
                        
                        background: Rectangle {
                            color: root.errorColor
                            radius: 8
                            opacity: parent.pressed ? 0.8 : 0.6
                        }
                        
                        contentItem: Text {
                            text: parent.text
                            font.pixelSize: 20
                            font.bold: true
                            color: "white"
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }
                        
                        onClicked: {
                            searchOverlay.searchOverlayVisible = false
                        }
                    }
                }
            }
            
            // Search component
            Loader {
                id: destinationSearchLoader
                width: parent.width
                height: parent.height - 60
                source: "DestinationSearch.qml"
                
                onLoaded: {
                    if (item) {
                        item.destinationSelected.connect(function(lat, lng, address) {
                            root.destLat = lat
                            root.destLng = lng
                            console.log("Destination selected:", address, lat, lng)
                            
                            // Send navigation command
                            if (NavigationBridge) {
                                // This would trigger the backend navigation
                                console.log("Triggering navigation to:", lat, lng)
                            }
                            
                            searchOverlay.searchOverlayVisible = false
                            root.isNavigating = true
                            
                            // Calculate example distance (would be real calculation in production)
                            var latDiff = Math.abs(root.currentLat - root.destLat);
                            var lngDiff = Math.abs(root.currentLng - root.destLng);
                            root.distanceRemaining = Math.sqrt(latDiff * latDiff + lngDiff * lngDiff) * 111000; // rough km to meters
                            root.etaSeconds = Math.floor(root.distanceRemaining / 15); // ~54 km/h average
                        })
                    }
                }
            }
        }
        
        // Connect NavigationBridge search results
        Connections {
            target: NavigationBridge
            function onSearchResultsReady(results) {
                if (destinationSearchLoader.item) {
                    destinationSearchLoader.item.setSearchResults(results)
                }
            }
        }
    }
    
    // Navigation instructions panel
    Rectangle {
        id: instructionsPanel
        anchors {
            bottom: controlPanel.top
            left: parent.left
            right: parent.right
        }
        height: root.isNavigating ? 180 : 0
        visible: height > 0
        color: root.surfaceColor
        border.color: root.outlineColor
        border.width: 1
        
        Behavior on height {
            NumberAnimation { duration: 300; easing.type: Easing.InOutQuad }
        }
        
        Column {
            anchors.fill: parent
            anchors.margins: root.paddingSize * 2
            spacing: root.spacingSize
            
            // Current instruction
            Row {
                width: parent.width
                spacing: root.spacingSize * 2
                
                // Turn icon
                Rectangle {
                    width: 80
                    height: 80
                    radius: 8
                    color: root.accentColor
                    
                    Text {
                        anchors.centerIn: parent
                        text: getTurnIcon(currentInstruction)
                        font.pixelSize: 48
                    }
                }
                
                // Instruction details
                Column {
                    width: parent.width - 100
                    spacing: root.spacingSize / 2
                    
                    // Distance to turn
                    Text {
                        text: formatDistance(distanceToNextTurn)
                        font.pixelSize: 32
                        font.bold: true
                        color: root.textColor
                    }
                    
                    // Instruction text
                    Text {
                        text: currentInstruction || "Continue on current road"
                        font.pixelSize: 18
                        color: root.textColor
                        wrapMode: Text.WordWrap
                        width: parent.width
                    }
                }
            }
            
            Rectangle {
                width: parent.width
                height: 1
                color: root.dividerColor
            }
            
            // Next instruction preview
            Row {
                width: parent.width
                spacing: root.spacingSize * 2
                
                Text {
                    text: getTurnIcon(nextInstruction)
                    font.pixelSize: 24
                    color: root.textSecondary
                }
                
                Text {
                    text: nextInstruction || "Then continue"
                    font.pixelSize: 14
                    color: root.textSecondary
                    elide: Text.ElideRight
                    width: parent.width - 50
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
        color: root.surfaceVariant
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: root.paddingSize
            spacing: root.spacingSize
            
            // Set Destination button
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                text: root.isNavigating ? "📍 Change Destination" : "📍 Set Destination"
                visible: !root.isNavigating || true // Always show for now
                
                    background: Rectangle {
                    color: root.accentColor
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
                    searchOverlay.searchOverlayVisible = true
                }
            }
            
            // Start/Stop navigation button
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                text: root.isNavigating ? "Stop Navigation" : "Start Navigation"
                visible: root.isNavigating // Only show when navigating
                
                    background: Rectangle {
                    color: root.isNavigating ? root.errorColor : root.accentColor
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
                    root.isNavigating = false
                    root.distanceRemaining = 0
                    root.etaSeconds = 0
                }
            }
            
            // Save as favourite button
            Button {
                Layout.preferredWidth: 60
                Layout.preferredHeight: 60
                text: "⭐"
                visible: root.isNavigating
                
                    background: Rectangle {
                    color: root.surfaceColor
                    radius: 8
                    border.color: root.outlineColor
                    border.width: 2
                }
                
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 24
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    saveFavouriteDialog.open()
                }
            }
            
            // Settings button - opens config page
            Button {
                Layout.preferredWidth: 60
                Layout.preferredHeight: 60
                text: "⚙️"
                
                background: Rectangle {
                    color: root.surfaceColor
                    radius: 8
                    border.color: root.outlineColor
                    border.width: 2
                }
                
                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 24
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    // Open the Settings tab and navigate to navigation config page
                    if (NavigationBridge) {
                        NavigationBridge.requestOpenSettings()
                    }
                }
            }
        }
    }
    
    // Settings panel removed - now using ConfigScreen
    
    // Navigation info overlay (when navigating)
    Rectangle {
        id: navInfo
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: root.isNavigating ? 80 : 0
        color: root.surfaceColor
        opacity: 0.95
        visible: height > 0
        
        Behavior on height {
            NumberAnimation { duration: 300; easing.type: Easing.OutQuad }
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: root.paddingSize
            spacing: root.spacingSize * 2
            visible: root.isNavigating
            
            // Distance
            Column {
                Layout.fillWidth: true
                spacing: 4
                
                Text {
                    text: {
                        if (distanceUnit === "imperial") {
                            return (distanceRemaining / 1609.34).toFixed(1) + " mi";
                        } else {
                            return (distanceRemaining / 1000).toFixed(1) + " km";
                        }
                    }
                    font.pixelSize: 24
                    font.bold: true
                    color: root.textColor
                }
                
                Text {
                    text: "Distance"
                    font.pixelSize: 12
                    color: root.textSecondary
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
                    color: root.textColor
                }
                
                Text {
                    text: "ETA"
                    font.pixelSize: 12
                    color: root.textSecondary
                }
            }
            
            // Speed (example)
            Column {
                Layout.fillWidth: true
                spacing: 4
                
                Text {
                    text: distanceUnit === "imperial" ? "31 mph" : "50 km/h"
                    font.pixelSize: 24
                    font.bold: true
                    color: root.textColor
                }
                
                Text {
                    text: "Speed"
                    font.pixelSize: 12
                    color: root.textSecondary
                }
            }

            // GPS Device
            Column {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    text: selectedGpsDevice
                    font.pixelSize: 24
                    font.bold: true
                    color: root.textColor
                }

                Text {
                    text: "GPS"
                    font.pixelSize: 12
                    color: root.textSecondary
                }
            }
        }
    }
    
    // Save favourite dialog
    Dialog {
        id: saveFavouriteDialog
        anchors.centerIn: parent
        modal: true
        title: "Save Favourite"
        width: 400
        
        contentItem: Column {
            spacing: root.spacingSize * 2
            padding: root.paddingSize * 2
            
            Text {
                text: "Save this location as a favourite?"
                font.pixelSize: 16
                color: root.textColor
                wrapMode: Text.WordWrap
                width: parent.width
            }
            
            Column {
                width: parent.width
                spacing: 8
                
                Text {
                    text: "Name:"
                    font.pixelSize: 14
                    font.bold: true
                    color: root.textColor
                }
                
                Rectangle {
                    width: parent.width
                    height: 45
                    color: root.surfaceVariant
                    radius: 6
                    border.color: favouriteNameField.activeFocus ? root.accentColor : root.outlineColor
                    border.width: 2
                    
                    TextField {
                        id: favouriteNameField
                        anchors.fill: parent
                        anchors.margins: root.paddingSize
                        placeholderText: "e.g., Home, Work, etc."
                        color: root.textColor
                        font.pixelSize: 16
                        
                        background: Rectangle {
                            color: "transparent"
                        }
                    }
                }
            }
            
            Text {
                text: "📍 " + destLat.toFixed(6) + ", " + destLng.toFixed(6)
                font.pixelSize: 12
                color: root.textSecondary
            }
            
            Row {
                spacing: root.spacingSize
                anchors.horizontalCenter: parent.horizontalCenter
                
                Button {
                    text: "Cancel"
                    width: 120
                    height: 45
                    
                        background: Rectangle {
                        color: root.surfaceVariant
                        radius: 6
                        border.color: root.outlineColor
                        border.width: 1
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 16
                        color: root.textColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        favouriteNameField.text = ""
                        saveFavouriteDialog.close()
                    }
                }
                
                Button {
                    text: "Save"
                    width: 120
                    height: 45
                    enabled: favouriteNameField.text.trim().length > 0
                    
                    background: Rectangle {
                        color: parent.enabled ? root.accentColor : root.surfaceVariant
                        radius: 6
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 16
                        font.bold: true
                        color: parent.enabled ? "white" : root.textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        if (favouriteNameField.text.trim().length > 0) {
                            if (destinationSearchLoader.item) {
                                destinationSearchLoader.item.addFavourite(
                                    favouriteNameField.text.trim(),
                                    destLat,
                                    destLng,
                                    "Saved location"
                                )
                            }
                            favouriteNameField.text = ""
                            saveFavouriteDialog.close()
                        }
                    }
                }
            }
        }
        
        background: Rectangle {
            color: root.surfaceColor
            radius: 12
            border.color: root.outlineColor
            border.width: 2
        }
    }
    
    // Helper functions
    function getTurnIcon(instruction: string): string {
        if (!instruction) return "➡️"
        
        const lowerInst = instruction.toLowerCase()
        if (lowerInst.includes("left")) return "↰"
        if (lowerInst.includes("right")) return "↱"
        if (lowerInst.includes("u-turn") || lowerInst.includes("uturn")) return "↶"
        if (lowerInst.includes("straight") || lowerInst.includes("continue")) return "⬆️"
        if (lowerInst.includes("roundabout")) return "⭮"
        if (lowerInst.includes("exit")) return "🛣️"
        if (lowerInst.includes("arrive") || lowerInst.includes("destination")) return "🏁"
        return "➡️"
    }
    
    function formatDistance(meters: real): string {
        if (distanceUnit === "imperial") {
            const feet = meters * 3.28084
            if (feet < 1000) {
                return Math.round(feet) + " ft"
            } else {
                const miles = feet / 5280
                return miles.toFixed(1) + " mi"
            }
        } else {
            if (meters < 1000) {
                return Math.round(meters) + " m"
            } else {
                const km = meters / 1000
                return km.toFixed(1) + " km"
            }
        }
    }
    
    function formatETA(seconds: int): string {
        if (seconds < 60) {
            return "< 1 min"
        } else if (seconds < 3600) {
            return Math.round(seconds / 60) + " min"
        } else {
            const hours = Math.floor(seconds / 3600)
            const mins = Math.round((seconds % 3600) / 60)
            return hours + " h " + mins + " min"
        }
    }
    
    // TODO: Event connections for routing
    // Will be connected once NavigationBridge exposes routing signals
    // For now, routing happens in C++ backend and can be tested via console logs
    
    Component.onCompleted: {
        console.log("Navigation view loaded")
    }
}
