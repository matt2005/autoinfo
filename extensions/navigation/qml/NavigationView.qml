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
    
    // Settings
    property bool showSpeedLimit: true
    property bool voiceGuidance: true
    property bool avoidTolls: false
    property bool avoidMotorways: false
    property string distanceUnit: "metric" // "metric" or "imperial"
    property bool settingsVisible: false
    // GPS hardware settings
    property var gpsDevices: ["Internal", "USB Receiver", "GNSS Hat", "Mock (Static)", "Mock (IP)"]
    property string selectedGpsDevice: "Internal"
    signal gpsDeviceChanged(string device)
    
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
    
    // Destination search overlay
    Rectangle {
        id: searchOverlay
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: controlPanel.top
        }
        color: surfaceColor
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
                color: surfaceVariant
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: paddingSize
                    spacing: spacingSize
                    
                    Text {
                        text: "📍 Set Destination"
                        font.pixelSize: 20
                        font.bold: true
                        color: textColor
                    }
                    
                    Item { Layout.fillWidth: true }
                    
                    Button {
                        width: 50
                        height: 50
                        text: "✕"
                        
                        background: Rectangle {
                            color: errorColor
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
            DestinationSearch {
                id: destinationSearch
                width: parent.width
                height: parent.height - 60
                
                onDestinationSelected: function(lat, lng, address) {
                    destLat = lat
                    destLng = lng
                    console.log("Destination selected:", address, lat, lng)
                    
                    // Send navigation command
                    if (NavigationBridge) {
                        // This would trigger the backend navigation
                        console.log("Triggering navigation to:", lat, lng)
                    }
                    
                    searchOverlay.searchOverlayVisible = false
                    isNavigating = true
                    
                    // Calculate example distance (would be real calculation in production)
                    var latDiff = Math.abs(currentLat - destLat)
                    var lngDiff = Math.abs(currentLng - destLng)
                    distanceRemaining = Math.sqrt(latDiff * latDiff + lngDiff * lngDiff) * 111000 // rough km to meters
                    etaSeconds = Math.floor(distanceRemaining / 15) // ~54 km/h average
                }
                
                onSearchLocation: function(query) {
                    // Already handled by NavigationBridge in GeocodingSearch
                }
            }
        }
        
        // Connect NavigationBridge search results
        Connections {
            target: NavigationBridge
            function onSearchResultsReady(results) {
                destinationSearch.setSearchResults(results)
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
            
            // Set Destination button
            Button {
                Layout.fillWidth: true
                Layout.preferredHeight: 60
                text: isNavigating ? "📍 Change Destination" : "📍 Set Destination"
                visible: !isNavigating || true // Always show for now
                
                background: Rectangle {
                    color: accentColor
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
                text: isNavigating ? "Stop Navigation" : "Start Navigation"
                visible: isNavigating // Only show when navigating
                
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
                    isNavigating = false
                    distanceRemaining = 0
                    etaSeconds = 0
                }
            }
            
            // Save as favourite button
            Button {
                Layout.preferredWidth: 60
                Layout.preferredHeight: 60
                text: "⭐"
                visible: isNavigating
                
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
                    saveFavouriteDialog.open()
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
                    settingsVisible = !settingsVisible
                }
            }
        }
    }
    
    // Settings panel overlay
    Rectangle {
        id: settingsPanel
        anchors {
            top: parent.top
            right: parent.right
            bottom: controlPanel.top
        }
        width: settingsVisible ? 350 : 0
        color: surfaceColor
        opacity: 0.98
        visible: width > 0
        clip: true
        
        Behavior on width {
            NumberAnimation { duration: 250; easing.type: Easing.OutQuad }
        }
        
        // Settings content
        Column {
            anchors.fill: parent
            anchors.margins: paddingSize
            spacing: spacingSize
            visible: settingsVisible
            
            // Header
            Row {
                width: parent.width
                spacing: spacingSize
                
                Text {
                    text: "⚙️ Navigation Settings"
                    font.pixelSize: 20
                    font.bold: true
                    color: textColor
                    anchors.verticalCenter: parent.verticalCenter
                }
                
                Item { Layout.fillWidth: true }
                
                Button {
                    width: 40
                    height: 40
                    text: "✕"
                    anchors.verticalCenter: parent.verticalCenter
                    
                    background: Rectangle {
                        color: errorColor
                        radius: 6
                        opacity: parent.pressed ? 0.8 : 0.6
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 18
                        font.bold: true
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: settingsVisible = false
                }
            }
            
            Rectangle {
                width: parent.width
                height: 2
                color: dividerColor
                opacity: 0.3
            }
            
            // Display settings
            Text {
                text: "Display"
                font.pixelSize: 16
                font.bold: true
                color: accentColor
                topPadding: spacingSize
            }
            
            // Show speed limit
            Row {
                width: parent.width
                spacing: spacingSize
                
                Text {
                    text: "Show Speed Limit"
                    font.pixelSize: 14
                    color: textColor
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 60
                }
                
                Rectangle {
                    width: 50
                    height: 30
                    radius: 15
                    color: showSpeedLimit ? accentColor : surfaceVariant
                    border.color: outlineColor
                    border.width: 1
                    anchors.verticalCenter: parent.verticalCenter
                    
                    Rectangle {
                        width: 24
                        height: 24
                        radius: 12
                        color: "white"
                        x: showSpeedLimit ? parent.width - width - 3 : 3
                        y: 3
                        
                        Behavior on x {
                            NumberAnimation { duration: 150 }
                        }
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: showSpeedLimit = !showSpeedLimit
                    }
                }
            }
            
            // Voice guidance
            Row {
                width: parent.width
                spacing: spacingSize
                
                Text {
                    text: "Voice Guidance"
                    font.pixelSize: 14
                    color: textColor
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 60
                }
                
                Rectangle {
                    width: 50
                    height: 30
                    radius: 15
                    color: voiceGuidance ? accentColor : surfaceVariant
                    border.color: outlineColor
                    border.width: 1
                    anchors.verticalCenter: parent.verticalCenter
                    
                    Rectangle {
                        width: 24
                        height: 24
                        radius: 12
                        color: "white"
                        x: voiceGuidance ? parent.width - width - 3 : 3
                        y: 3
                        
                        Behavior on x {
                            NumberAnimation { duration: 150 }
                        }
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: voiceGuidance = !voiceGuidance
                    }
                }
            }
            
            // Route preferences
            Text {
                text: "Route Preferences"
                font.pixelSize: 16
                font.bold: true
                color: accentColor
                topPadding: spacingSize * 2
            }
            
            // Avoid tolls
            Row {
                width: parent.width
                spacing: spacingSize
                
                Text {
                    text: "Avoid Toll Roads"
                    font.pixelSize: 14
                    color: textColor
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 60
                }
                
                Rectangle {
                    width: 50
                    height: 30
                    radius: 15
                    color: avoidTolls ? accentColor : surfaceVariant
                    border.color: outlineColor
                    border.width: 1
                    anchors.verticalCenter: parent.verticalCenter
                    
                    Rectangle {
                        width: 24
                        height: 24
                        radius: 12
                        color: "white"
                        x: avoidTolls ? parent.width - width - 3 : 3
                        y: 3
                        
                        Behavior on x {
                            NumberAnimation { duration: 150 }
                        }
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: avoidTolls = !avoidTolls
                    }
                }
            }
            
            // Avoid motorways
            Row {
                width: parent.width
                spacing: spacingSize
                
                Text {
                    text: "Avoid Motorways"
                    font.pixelSize: 14
                    color: textColor
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 60
                }
                
                Rectangle {
                    width: 50
                    height: 30
                    radius: 15
                    color: avoidMotorways ? accentColor : surfaceVariant
                    border.color: outlineColor
                    border.width: 1
                    anchors.verticalCenter: parent.verticalCenter
                    
                    Rectangle {
                        width: 24
                        height: 24
                        radius: 12
                        color: "white"
                        x: avoidMotorways ? parent.width - width - 3 : 3
                        y: 3
                        
                        Behavior on x {
                            NumberAnimation { duration: 150 }
                        }
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: avoidMotorways = !avoidMotorways
                    }
                }
            }
            
            // Hardware
            Text {
                text: "Hardware"
                font.pixelSize: 16
                font.bold: true
                color: accentColor
                topPadding: spacingSize * 2
            }

            // GPS Device selection
            Row {
                width: parent.width
                spacing: spacingSize

                Text {
                    text: "GPS Device"
                    font.pixelSize: 14
                    color: textColor
                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width - 160
                }

                ComboBox {
                    id: gpsDeviceCombo
                    width: 140
                    model: gpsDevices
                    currentIndex: gpsDevices.indexOf(selectedGpsDevice)
                    font.pixelSize: 14

                    delegate: ItemDelegate {
                        width: parent.width
                        text: modelData
                        font.pixelSize: 14
                        highlighted: gpsDeviceCombo.currentIndex === index
                    }

                    onActivated: function(index) {
                        selectedGpsDevice = gpsDevices[index]
                        gpsDeviceChanged(selectedGpsDevice)
                        if (NavigationBridge) {
                            NavigationBridge.setGpsDevice(selectedGpsDevice)
                        }
                    }
                }
            }

            // Services
            Text {
                text: "Services"
                font.pixelSize: 16
                font.bold: true
                color: accentColor
                topPadding: spacingSize * 2
            }

            // Geocoding Provider selection
            Column {
                width: parent.width
                spacing: 8

                Row {
                    width: parent.width
                    spacing: spacingSize

                    Text {
                        text: "Map Provider"
                        font.pixelSize: 14
                        color: textColor
                        anchors.verticalCenter: parent.verticalCenter
                        width: parent.width - 160
                    }

                    ComboBox {
                        id: providerCombo
                        width: 140
                        model: ListModel { id: providersModel }
                        textRole: "displayName"
                        font.pixelSize: 14

                        delegate: ItemDelegate {
                            width: parent.width
                            text: model.displayName
                            font.pixelSize: 14
                            highlighted: providerCombo.currentIndex === index
                        }

                        onActivated: function(index) {
                            if (NavigationBridge && providersModel.count > 0) {
                                var providerId = providersModel.get(index).id
                                NavigationBridge.setGeocodingProvider(providerId)
                            }
                        }
                        
                        Component.onCompleted: {
                            if (NavigationBridge) {
                                // Populate providers list
                                var providers = NavigationBridge.availableProviders
                                providersModel.clear()
                                for (var i = 0; i < providers.length; i++) {
                                    providersModel.append(providers[i])
                                }
                                
                                // Set current provider
                                var currentProvider = NavigationBridge.geocodingProvider
                                for (var j = 0; j < providersModel.count; j++) {
                                    if (providersModel.get(j).id === currentProvider) {
                                        providerCombo.currentIndex = j
                                        break
                                    }
                                }
                            }
                        }
                    }
                }

                // Provider description
                Text {
                    text: {
                        if (NavigationBridge && providersModel.count > 0 && providerCombo.currentIndex >= 0) {
                            return providersModel.get(providerCombo.currentIndex).description
                        }
                        return ""
                    }
                    font.pixelSize: 11
                    color: textSecondary
                    opacity: 0.7
                    wrapMode: Text.WordWrap
                    width: parent.width
                }
            }

            // Units
            Text {
                text: "Units"
                font.pixelSize: 16
                font.bold: true
                color: accentColor
                topPadding: spacingSize * 2
            }
            
            Row {
                width: parent.width
                spacing: spacingSize
                
                Button {
                    width: (parent.width - spacingSize) / 2
                    height: 40
                    text: "Metric (km)"
                    
                    background: Rectangle {
                        color: distanceUnit === "metric" ? accentColor : surfaceVariant
                        radius: 6
                        border.color: outlineColor
                        border.width: 1
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 14
                        color: distanceUnit === "metric" ? "white" : textColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: distanceUnit = "metric"
                }
                
                Button {
                    width: (parent.width - spacingSize) / 2
                    height: 40
                    text: "Imperial (mi)"
                    
                    background: Rectangle {
                        color: distanceUnit === "imperial" ? accentColor : surfaceVariant
                        radius: 6
                        border.color: outlineColor
                        border.width: 1
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 14
                        color: distanceUnit === "imperial" ? "white" : textColor
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: distanceUnit = "imperial"
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
                    text: {
                        if (distanceUnit === "imperial") {
                            return (distanceRemaining / 1609.34).toFixed(1) + " mi";
                        } else {
                            return (distanceRemaining / 1000).toFixed(1) + " km";
                        }
                    }
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
                    text: distanceUnit === "imperial" ? "31 mph" : "50 km/h"
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

            // GPS Device
            Column {
                Layout.fillWidth: true
                spacing: 4

                Text {
                    text: selectedGpsDevice
                    font.pixelSize: 24
                    font.bold: true
                    color: textColor
                }

                Text {
                    text: "GPS"
                    font.pixelSize: 12
                    color: textSecondary
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
            spacing: spacingSize * 2
            padding: paddingSize * 2
            
            Text {
                text: "Save this location as a favourite?"
                font.pixelSize: 16
                color: textColor
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
                    color: textColor
                }
                
                Rectangle {
                    width: parent.width
                    height: 45
                    color: surfaceVariant
                    radius: 6
                    border.color: favouriteNameField.activeFocus ? accentColor : outlineColor
                    border.width: 2
                    
                    TextField {
                        id: favouriteNameField
                        anchors.fill: parent
                        anchors.margins: paddingSize
                        placeholderText: "e.g., Home, Work, etc."
                        color: textColor
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
                color: textSecondary
            }
            
            Row {
                spacing: spacingSize
                anchors.horizontalCenter: parent.horizontalCenter
                
                Button {
                    text: "Cancel"
                    width: 120
                    height: 45
                    
                    background: Rectangle {
                        color: surfaceVariant
                        radius: 6
                        border.color: outlineColor
                        border.width: 1
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 16
                        color: textColor
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
                        color: parent.enabled ? accentColor : surfaceVariant
                        radius: 6
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 16
                        font.bold: true
                        color: parent.enabled ? "white" : textSecondary
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    
                    onClicked: {
                        if (favouriteNameField.text.trim().length > 0) {
                            destinationSearch.addFavourite(
                                favouriteNameField.text.trim(),
                                destLat,
                                destLng,
                                "Saved location"
                            )
                            favouriteNameField.text = ""
                            saveFavouriteDialog.close()
                        }
                    }
                }
            }
        }
        
        background: Rectangle {
            color: surfaceColor
            radius: 12
            border.color: outlineColor
            border.width: 2
        }
    }
    
    Component.onCompleted: {
        console.log("Navigation view loaded")
    }
}
