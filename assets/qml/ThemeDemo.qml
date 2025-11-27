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
 * Example QML application demonstrating the theme system
 * 
 * This file shows how to use the CrankshaftReborn theme system
 * in QML applications with reactive property bindings.
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CrankshaftReborn.UI 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 800
    height: 600
    title: "CrankshaftReborn - Theme Demo"
    
    // Apply theme colours to window
    color: Theme.background
    
    // Smooth colour transitions when theme changes
    Behavior on color {
        ColorAnimation { duration: 200 }
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: Theme.padding * 2
        spacing: Theme.spacing * 2
        
        // Header
        Rectangle {
            Layout.fillWidth: true
            Layout.preferredHeight: 80
            color: Theme.surface
            radius: Theme.cornerRadius
            
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: Theme.padding
                spacing: Theme.spacing
                
                Text {
                    text: "🎨 Theme System Demo"
                    font.pixelSize: 24
                    font.bold: true
                    color: Theme.text
                    Layout.fillWidth: true
                    
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }
                
                // Theme toggle button
                StyledButton {
                    text: Theme.isDark ? "☀️ Light" : "🌙 Dark"
                    buttonType: "outline"
                    onClicked: Theme.toggleTheme()
                }
            }
        }
        
        // Content area
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: Theme.surface
            radius: Theme.cornerRadius
            
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
            
            ColumnLayout {
                anchors.fill: parent
                anchors.margins: Theme.padding * 2
                spacing: Theme.spacing * 2
                
                // Current theme info
                Text {
                    text: "Current Theme: " + Theme.currentTheme
                    font.pixelSize: 18
                    color: Theme.text
                    Layout.fillWidth: true
                    
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }
                
                Text {
                    text: "Mode: " + (Theme.isDark ? "Dark" : "Light")
                    font.pixelSize: 14
                    color: Theme.textSecondary
                    Layout.fillWidth: true
                    
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }
                
                // Divider
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: Theme.divider
                    
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }
                
                // Button showcase
                Text {
                    text: "Button Styles"
                    font.pixelSize: 16
                    font.bold: true
                    color: Theme.text
                    
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }
                
                Flow {
                    Layout.fillWidth: true
                    spacing: Theme.spacing
                    
                    StyledButton {
                        text: "Primary"
                        buttonType: "primary"
                        onClicked: console.log("Primary clicked");
                    }
                    
                    StyledButton {
                        text: "Secondary"
                        buttonType: "secondary"
                        onClicked: console.log("Secondary clicked");
                    }
                    
                    StyledButton {
                        text: "Accent"
                        buttonType: "accent"
                        onClicked: console.log("Accent clicked");
                    }
                    
                    StyledButton {
                        text: "Outline"
                        buttonType: "outline"
                        onClicked: console.log("Outline clicked");
                    }
                }
                
                // Divider
                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: Theme.divider
                    
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }
                
                // Colour palette showcase
                Text {
                    text: "Colour Palette"
                    font.pixelSize: 16
                    font.bold: true
                    color: Theme.text
                    
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }
                
                Flow {
                    Layout.fillWidth: true
                    spacing: Theme.spacing / 2
                    
                    ColourSwatch { colourName: "Primary"; colour: Theme.primary }
                    ColourSwatch { colourName: "Secondary"; colour: Theme.secondary }
                    ColourSwatch { colourName: "Accent"; colour: Theme.accent }
                    ColourSwatch { colourName: "Error"; colour: Theme.error }
                    ColourSwatch { colourName: "Warning"; colour: Theme.warning }
                    ColourSwatch { colourName: "Success"; colour: Theme.success }
                    ColourSwatch { colourName: "Info"; colour: Theme.info }
                }
                
                Item { Layout.fillHeight: true } // Spacer
                
                // Theme selector
                Text {
                    text: "Available Themes"
                    font.pixelSize: 16
                    font.bold: true
                    color: Theme.text
                    
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                }
                
                ComboBox {
                    Layout.preferredWidth: 200
                    model: Theme.availableThemes
                    currentIndex: {
                        var themes = Theme.availableThemes
                        for (var i = 0; i < themes.length; i++) {
                            if (themes[i] === Theme.currentTheme) {
                                return i
                            }
                        }
                        return 0
                    }
                    
                    onCurrentTextChanged: {
                        if (currentText !== Theme.currentTheme) {
                            Theme.setTheme(currentText)
                        }
                    }
                    
                    background: Rectangle {
                        color: Theme.surfaceVariant
                        radius: Theme.cornerRadius
                        
                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }
                    
                    contentItem: Text {
                        text: parent.displayText
                        color: Theme.text
                        verticalAlignment: Text.AlignVCenter
                        leftPadding: Theme.padding
                        
                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }
                }
            }
        }
    }
    
    // Colour swatch component
    component ColourSwatch: Rectangle {
        property string colourName: ""
        property color colour: "transparent"
        
        width: 80
        height: 60
        color: colour
        radius: Theme.cornerRadius / 2
        
        Behavior on color {
            ColorAnimation { duration: 200 }
        }
        
        Text {
            anchors.centerIn: parent
            text: colourName
            font.pixelSize: 10
            color: {
                // Choose contrasting text colour
                var luminance = (colour.r * 299 + colour.g * 587 + colour.b * 114) / 1000
                return luminance > 0.5 ? "#000000" : "#FFFFFF"
            }
            
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
        }
    }
}
