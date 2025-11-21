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
 * Direct coordinate input component for advanced users
 */
Item {
    id: root
    
    // Signals
    signal coordinatesSubmitted(real latitude, real longitude, string label)
    
    // Theme properties
    readonly property color surfaceColor: ThemeManager.surfaceColor
    readonly property color surfaceVariant: ThemeManager.cardColor
    readonly property color accentColor: ThemeManager.accentColor
    readonly property color errorColor: ThemeManager.errorColor
    readonly property color textColor: ThemeManager.textColor
    readonly property color textSecondary: ThemeManager.textSecondaryColor
    readonly property color outlineColor: ThemeManager.borderColor
    readonly property int paddingSize: ThemeManager.padding
    readonly property int spacingSize: ThemeManager.spacing
    
    Rectangle {
        anchors.fill: parent
        color: surfaceVariant
        radius: 8
        
        Column {
            anchors.centerIn: parent
            width: parent.width - paddingSize * 4
            spacing: spacingSize * 2
            
            // Header
            Column {
                width: parent.width
                spacing: spacingSize
                
                Text {
                    text: "⚙️ Advanced Coordinate Input"
                    font.pixelSize: 20
                    font.bold: true
                    color: textColor
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                
                Text {
                    text: "Enter GPS coordinates directly"
                    font.pixelSize: 14
                    color: textSecondary
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
            
            // Latitude input
            Column {
                width: parent.width
                spacing: 8
                
                Text {
                    text: "Latitude"
                    font.pixelSize: 14
                    font.bold: true
                    color: textColor
                }
                
                Rectangle {
                    width: parent.width
                    height: 50
                    color: surfaceColor
                    radius: 8
                    border.color: latitudeField.activeFocus ? accentColor : outlineColor
                    border.width: 2
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: paddingSize
                        spacing: spacingSize
                        
                        TextField {
                            id: latitudeField
                            Layout.fillWidth: true
                            placeholderText: "e.g., 51.5074"
                            color: textColor
                            font.pixelSize: 16
                            validator: DoubleValidator {
                                bottom: -90.0
                                top: 90.0
                                decimals: 8
                                notation: DoubleValidator.StandardNotation
                            }
                            
                            background: Rectangle {
                                color: "transparent"
                            }
                        }
                        
                        Text {
                            text: "°"
                            font.pixelSize: 18
                            color: textSecondary
                        }
                    }
                }
                
                Text {
                    text: "Range: -90° to +90° (South to North)"
                    font.pixelSize: 11
                    color: textSecondary
                    opacity: 0.7
                }
            }
            
            // Longitude input
            Column {
                width: parent.width
                spacing: 8
                
                Text {
                    text: "Longitude"
                    font.pixelSize: 14
                    font.bold: true
                    color: textColor
                }
                
                Rectangle {
                    width: parent.width
                    height: 50
                    color: surfaceColor
                    radius: 8
                    border.color: longitudeField.activeFocus ? accentColor : outlineColor
                    border.width: 2
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: paddingSize
                        spacing: spacingSize
                        
                        TextField {
                            id: longitudeField
                            Layout.fillWidth: true
                            placeholderText: "e.g., -0.1278"
                            color: textColor
                            font.pixelSize: 16
                            validator: DoubleValidator {
                                bottom: -180.0
                                top: 180.0
                                decimals: 8
                                notation: DoubleValidator.StandardNotation
                            }
                            
                            background: Rectangle {
                                color: "transparent"
                            }
                        }
                        
                        Text {
                            text: "°"
                            font.pixelSize: 18
                            color: textSecondary
                        }
                    }
                }
                
                Text {
                    text: "Range: -180° to +180° (West to East)"
                    font.pixelSize: 11
                    color: textSecondary
                    opacity: 0.7
                }
            }
            
            // Label input
            Column {
                width: parent.width
                spacing: 8
                
                Text {
                    text: "Label (Optional)"
                    font.pixelSize: 14
                    font.bold: true
                    color: textColor
                }
                
                Rectangle {
                    width: parent.width
                    height: 50
                    color: surfaceColor
                    radius: 8
                    border.color: labelField.activeFocus ? accentColor : outlineColor
                    border.width: 2
                    
                    TextField {
                        id: labelField
                        anchors.fill: parent
                        anchors.margins: paddingSize
                        placeholderText: "e.g., Home, Office, etc."
                        color: textColor
                        font.pixelSize: 16
                        
                        background: Rectangle {
                            color: "transparent"
                        }
                    }
                }
            }
            
            // Submit button
            Button {
                width: parent.width
                height: 60
                enabled: isValid()
                
                background: Rectangle {
                    color: parent.enabled ? accentColor : surfaceVariant
                    radius: 8
                    opacity: parent.pressed ? 0.8 : 1.0
                }
                
                contentItem: Text {
                    text: "Set Destination"
                    font.pixelSize: 18
                    font.bold: true
                    color: parent.enabled ? "white" : textSecondary
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                }
                
                onClicked: {
                    if (isValid()) {
                        var lat = parseFloat(latitudeField.text)
                        var lng = parseFloat(longitudeField.text)
                        var label = labelField.text.trim() || "Custom Location (" + lat.toFixed(4) + ", " + lng.toFixed(4) + ")"
                        
                        root.coordinatesSubmitted(lat, lng, label)
                        
                        // Clear fields
                        latitudeField.text = ""
                        longitudeField.text = ""
                        labelField.text = ""
                    }
                }
            }
            
            // Examples
            Rectangle {
                width: parent.width
                height: examplesColumn.height + paddingSize * 2
                color: surfaceColor
                radius: 6
                opacity: 0.5
                
                Column {
                    id: examplesColumn
                    anchors.centerIn: parent
                    width: parent.width - paddingSize * 2
                    spacing: 4
                    
                    Text {
                        text: "Examples:"
                        font.pixelSize: 12
                        font.bold: true
                        color: textColor
                    }
                    
                    Text {
                        text: "• London: 51.5074, -0.1278"
                        font.pixelSize: 11
                        color: textSecondary
                        font.family: "monospace"
                    }
                    
                    Text {
                        text: "• New York: 40.7128, -74.0060"
                        font.pixelSize: 11
                        color: textSecondary
                        font.family: "monospace"
                    }
                    
                    Text {
                        text: "• Tokyo: 35.6762, 139.6503"
                        font.pixelSize: 11
                        color: textSecondary
                        font.family: "monospace"
                    }
                }
            }
        }
    }
    
    function isValid() {
        if (latitudeField.text.trim() === "" || longitudeField.text.trim() === "") {
            return false
        }
        
        var lat = parseFloat(latitudeField.text)
        var lng = parseFloat(longitudeField.text)
        
        if (isNaN(lat) || isNaN(lng)) {
            return false
        }
        
        if (lat < -90 || lat > 90) {
            return false
        }
        
        if (lng < -180 || lng > 180) {
            return false
        }
        
        return true
    }
}
