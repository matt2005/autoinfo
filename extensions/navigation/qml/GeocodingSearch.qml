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

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

/**
 * Geocoding search component using Nominatim/OpenStreetMap
 */
Item {
    id: root
    
    // Signals
    signal locationSelected(real latitude, real longitude, string address)
    signal searchRequested(string query)
    
    // Properties
    property var searchResults: []
    property bool isSearching: false
    property string searchQuery: ""
    
    // Theme properties
    readonly property color surfaceColor: ThemeManager.surfaceColor
    readonly property color surfaceVariant: ThemeManager.cardColor
    readonly property color accentColor: ThemeManager.accentColor
    readonly property color textColor: ThemeManager.textColor
    readonly property color textSecondary: ThemeManager.textSecondaryColor
    readonly property color outlineColor: ThemeManager.borderColor
    readonly property int paddingSize: ThemeManager.padding
    readonly property int spacingSize: ThemeManager.spacing
    
    Column {
        anchors.fill: parent
        spacing: root.spacingSize
        
        // Search input
        RowLayout {
            anchors.left: parent.left
            anchors.right: parent.right
            height: 50
            spacing: root.spacingSize
            
            Rectangle {
                Layout.fillWidth: true
                Layout.preferredHeight: parent.height
                color: root.surfaceVariant
                radius: 8
                border.color: searchField.activeFocus ? root.accentColor : root.outlineColor
                border.width: 2
                
                RowLayout {
                    anchors.fill: parent
                    anchors.margins: root.paddingSize
                    spacing: root.spacingSize
                    
                    Text {
                        text: "🔍"
                        font.pixelSize: 20
                        color: root.textSecondary
                    }
                    
                    TextField {
                        id: searchField
                        Layout.fillWidth: true
                        placeholderText: "Search for address or place..."
                        color: root.textColor
                        font.pixelSize: 16
                        
                        background: Rectangle {
                            color: "transparent"
                        }
                        
                        onAccepted: {
                            if (text.trim().length > 0) {
                                performSearch()
                            }
                        }
                        
                        Keys.onReturnPressed: {
                            if (text.trim().length > 0) {
                                performSearch()
                            }
                        }
                    }
                    
                    Button {
                        id: clearBtn
                        visible: searchField.text.length > 0
                        Layout.preferredWidth: 30
                        Layout.preferredHeight: 30
                        text: "✕"

                        background: Rectangle {
                            color: "transparent"
                            radius: 15
                        }

                        contentItem: Text {
                            text: clearBtn.text
                            font.pixelSize: 16
                            color: root.textSecondary
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                        }

                        onClicked: {
                            searchField.text = ""
                            root.searchResults = []
                        }
                    }
                }
            }
            
            Button {
                Layout.preferredWidth: 100
                Layout.preferredHeight: parent.height
                text: "Search"
                enabled: searchField.text.trim().length > 0 && !root.isSearching

                background: Rectangle {
                    color: parent.enabled ? root.accentColor : root.surfaceVariant
                    radius: 8
                    opacity: parent.pressed ? 0.8 : 1.0
                }

                contentItem: Text {
                    text: parent.text
                    font.pixelSize: 16
                    font.bold: true
                    color: parent.enabled ? "white" : root.textSecondary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }

                onClicked: performSearch()
            }
        }
        
        // Results area
            Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            height: parent.height - 58
            color: root.surfaceVariant
            radius: 8
            
            ListView {
                id: resultsListView
                anchors.fill: parent
                anchors.margins: root.paddingSize
                spacing: root.spacingSize
                clip: true
                visible: root.searchResults.length > 0
                
                model: searchResults
                
                delegate: Rectangle {
                    width: resultsListView.width
                    height: 80
                    color: root.surfaceColor
                    radius: 6
                    border.color: root.outlineColor
                    border.width: 1
                    
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            root.locationSelected(
                                modelData.latitude,
                                modelData.longitude,
                                modelData.display_name
                            )
                        }
                    }
                    
                        RowLayout {
                        anchors.fill: parent
                        anchors.margins: root.paddingSize
                        spacing: root.spacingSize
                        
                        Column {
                            Layout.fillWidth: true
                            spacing: 4
                            
                            Text {
                                text: modelData.name || "Unknown"
                                font.pixelSize: 16
                                font.bold: true
                                color: root.textColor
                                elide: Text.ElideRight
                                width: parent.width
                            }
                            
                            Text {
                                text: modelData.display_name || ""
                                font.pixelSize: 12
                                color: root.textSecondary
                                elide: Text.ElideRight
                                width: parent.width
                                wrapMode: Text.WordWrap
                                maximumLineCount: 2
                            }
                        }
                        
                        Text {
                            text: "→"
                            font.pixelSize: 24
                            color: root.accentColor
                        }
                    }
                }
            }
            
            // Empty state
            Column {
                anchors.centerIn: parent
                spacing: spacingSize
                visible: !isSearching && searchResults.length === 0
                
                Text {
                    text: searchQuery.length > 0 ? "🔍" : "📍"
                    font.pixelSize: 48
                    anchors.horizontalCenter: parent.horizontalCenter
                    opacity: 0.3
                }
                
                Text {
                    text: searchQuery.length > 0 ? "No results found" : "Enter an address or place to search"
                    font.pixelSize: 16
                    color: textSecondary
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
            
            // Loading state
            Column {
                anchors.centerIn: parent
                spacing: spacingSize
                visible: isSearching
                
                BusyIndicator {
                    anchors.horizontalCenter: parent.horizontalCenter
                    running: isSearching
                }
                
                Text {
                    text: "Searching..."
                    font.pixelSize: 16
                    color: textSecondary
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }
    
    function performSearch() {
        if (searchField.text.trim().length === 0) return
        
        searchQuery = searchField.text
        isSearching = true
        searchResults = []
        root.searchRequested(searchQuery)
        
        // Call backend to perform search
        if (NavigationBridge) {
            NavigationBridge.searchLocation(searchQuery)
        }
    }
}
