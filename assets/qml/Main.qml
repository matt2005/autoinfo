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
 * Main application UI with extension integration
 * 
 * Demonstrates:
 * - Dynamic extension loading via ExtensionRegistry
 * - Tab-based navigation for extension views
 * - Extension isolation and sandboxing
 * - Theme integration
 */

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CrankshaftReborn.UI 1.0
import CrankshaftReborn.Extensions 1.0

ApplicationWindow {
    id: root
    visible: true
    width: 1024
    height: 600
    title: "Crankshaft Reborn - Capability-Based Extensions"
    
    color: Theme.background
    
    Behavior on color {
        ColorAnimation { duration: 200 }
    }
    
    // Header bar with extension tabs
    header: Rectangle {
        height: 60
        color: Theme.surface
        
        Behavior on color {
            ColorAnimation { duration: 200 }
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.margins: Theme.padding
            spacing: Theme.spacing
            
            // App logo/title
            RowLayout {
                spacing: Theme.spacing
                
                Text {
                    text: "🚗"
                    font.pixelSize: 32
                }
                
                Column {
                    spacing: 2
                    
                    Text {
                        text: "Crankshaft Reborn"
                        font.pixelSize: 18
                        font.bold: true
                        color: Theme.text
                        
                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }
                    
                    Text {
                        text: "Capability-Based Extensions"
                        font.pixelSize: 10
                        color: Theme.textSecondary
                        
                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }
                }
            }
            
            Rectangle {
                width: 1
                height: 40
                color: Theme.border
                Layout.leftMargin: Theme.spacing
                Layout.rightMargin: Theme.spacing
            }
            
            // Extension tabs
            TabBar {
                id: tabBar
                Layout.fillWidth: true
                Layout.fillHeight: true
                background: Rectangle { color: "transparent" }
                
                // Home tab (always present)
                TabButton {
                    text: "🏠 Home"
                    font.pixelSize: 14
                    
                    background: Rectangle {
                        color: tabBar.currentIndex === 0 ? Theme.accent : "transparent"
                        opacity: tabBar.currentIndex === 0 ? 0.1 : 0
                        radius: Theme.cornerRadius
                        
                        Behavior on opacity {
                            NumberAnimation { duration: 150 }
                        }
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: tabBar.currentIndex === 0 ? Theme.accent : Theme.text
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                        
                        Behavior on color {
                            ColorAnimation { duration: 150 }
                        }
                    }
                }
                
                // Dynamic extension tabs
                Repeater {
                    model: ExtensionRegistry.mainComponents
                    
                    TabButton {
                        text: (modelData.icon ? modelData.icon + " " : "") + modelData.title
                        font.pixelSize: 14
                        
                        required property var modelData
                        required property int index
                        property int tabIndex: index + 1
                        
                        background: Rectangle {
                            color: tabBar.currentIndex === tabIndex ? Theme.accent : "transparent"
                            opacity: tabBar.currentIndex === tabIndex ? 0.1 : 0
                            radius: Theme.cornerRadius
                            
                            Behavior on opacity {
                                NumberAnimation { duration: 150 }
                            }
                        }
                        
                        contentItem: Text {
                            text: parent.text
                            font: parent.font
                            color: tabBar.currentIndex === tabIndex ? Theme.accent : Theme.text
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            
                            Behavior on color {
                                ColorAnimation { duration: 150 }
                            }
                        }
                    }
                }

                // Settings tab (appears after dynamic extensions)
                TabButton {
                    readonly property int tabIndex: 1 + ExtensionRegistry.mainComponents.length
                    text: "⚙ Settings"
                    font.pixelSize: 14

                    background: Rectangle {
                        color: tabBar.currentIndex === parent.tabIndex ? Theme.accent : "transparent"
                        opacity: tabBar.currentIndex === parent.tabIndex ? 0.1 : 0
                        radius: Theme.cornerRadius

                        Behavior on opacity {
                            NumberAnimation { duration: 150 }
                        }
                    }

                    contentItem: Text {
                        text: parent.text
                        font: parent.font
                        color: tabBar.currentIndex === parent.tabIndex ? Theme.accent : Theme.text
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        Behavior on color {
                            ColorAnimation { duration: 150 }
                        }
                    }

                    onClicked: tabBar.currentIndex = tabIndex
                }
            }
            
            Rectangle {
                width: 1
                height: 40
                color: Theme.border
                Layout.leftMargin: Theme.spacing
                Layout.rightMargin: Theme.spacing
            }
            
            // Theme toggle
            Button {
                text: Theme.isDark ? "☀️" : "🌙"
                font.pixelSize: 20
                flat: true
                onClicked: Theme.toggleTheme()
                
                ToolTip.visible: hovered
                ToolTip.text: "Toggle " + (Theme.isDark ? "light" : "dark") + " mode"
            }
        }
    }
    
    // Content area with extension views
    StackLayout {
        id: stackLayout
        anchors.fill: parent
        currentIndex: tabBar.currentIndex
        
        // Home view (index 0)
        Rectangle {
            color: Theme.background
            
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
            
            ColumnLayout {
                anchors.centerIn: parent
                spacing: Theme.spacing * 3
                width: 600
                
                // Welcome message
                Column {
                    Layout.alignment: Qt.AlignHCenter
                    spacing: Theme.spacing
                    
                    Text {
                        text: "Welcome to Crankshaft Reborn"
                        font.pixelSize: 32
                        font.bold: true
                        color: Theme.text
                        anchors.horizontalCenter: parent.horizontalCenter
                        
                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }
                    
                    Text {
                        text: "Capability-Based Extension Architecture Demo"
                        font.pixelSize: 16
                        color: Theme.textSecondary
                        anchors.horizontalCenter: parent.horizontalCenter
                        
                        Behavior on color {
                            ColorAnimation { duration: 200 }
                        }
                    }
                }
                
                // Extension info
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 200
                    color: Theme.surface
                    radius: Theme.cornerRadius
                    border.color: Theme.border
                    border.width: 1
                    
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: Theme.padding * 2
                        spacing: Theme.spacing * 2
                        
                        Text {
                            text: "📦 Loaded Extensions"
                            font.pixelSize: 20
                            font.bold: true
                            color: Theme.text
                            
                            Behavior on color {
                                ColorAnimation { duration: 200 }
                            }
                        }
                        
                        GridLayout {
                            columns: 2
                            rowSpacing: Theme.spacing
                            columnSpacing: Theme.spacing * 2
                            Layout.fillWidth: true
                            
                            Text {
                                text: "Main Views:"
                                color: Theme.textSecondary
                                font.pixelSize: 14
                            }
                            Text {
                                text: ExtensionRegistry.mainComponents.length
                                color: Theme.accent
                                font.pixelSize: 14
                                font.bold: true
                            }
                            
                            Text {
                                text: "Widgets:"
                                color: Theme.textSecondary
                                font.pixelSize: 14
                            }
                            Text {
                                text: ExtensionRegistry.widgets.length
                                color: Theme.accent
                                font.pixelSize: 14
                                font.bold: true
                            }
                            
                            Text {
                                text: "Total Components:"
                                color: Theme.textSecondary
                                font.pixelSize: 14
                            }
                            Text {
                                text: ExtensionRegistry.componentCount
                                color: Theme.accent
                                font.pixelSize: 14
                                font.bold: true
                            }
                        }
                        
                        Item { Layout.fillHeight: true }
                        
                        Text {
                            text: "Click extension tabs above to view"
                            color: Theme.textSecondary
                            font.pixelSize: 12
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
                
                // Architecture info
                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 150
                    color: Theme.surface
                    radius: Theme.cornerRadius
                    border.color: Theme.border
                    border.width: 1
                    
                    Behavior on color {
                        ColorAnimation { duration: 200 }
                    }
                    
                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: Theme.padding * 2
                        spacing: Theme.spacing
                        
                        Text {
                            text: "🔒 Security Features"
                            font.pixelSize: 16
                            font.bold: true
                            color: Theme.text
                        }
                        
                        Column {
                            spacing: Theme.spacing / 2
                            Layout.fillWidth: true
                            
                            Repeater {
                                model: [
                                    "✓ Capability-based access control",
                                    "✓ Extensions cannot access core services directly",
                                    "✓ All extension operations are audited",
                                    "✓ Capabilities can be revoked at runtime"
                                ]
                                
                                Text {
                                    text: modelData
                                    color: Theme.textSecondary
                                    font.pixelSize: 12
                                    
                                    required property string modelData
                                }
                            }
                        }
                    }
                }

                // (Settings tab moved to header TabBar)
            }
        }
        
        // Dynamic extension views
        Repeater {
            model: ExtensionRegistry.mainComponents
            
            // Each extension loads in isolated Loader
            Loader {
                required property var modelData
                
                source: modelData.qmlPath || ""
                asynchronous: true
                
                // Error handling
                onStatusChanged: {
                    if (status === Loader.Error) {
                        console.error("Failed to load extension view:", modelData.qmlPath);
                    } else if (status === Loader.Ready) {
                        console.log("Loaded extension view:", modelData.title);
                    }
                }
                
                // Loading indicator
                Rectangle {
                    anchors.fill: parent
                    color: Theme.background
                    visible: parent.status === Loader.Loading
                    
                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: Theme.spacing * 2
                        
                        BusyIndicator {
                            Layout.alignment: Qt.AlignHCenter
                            running: parent.visible
                        }
                        
                        Text {
                            text: "Loading " + modelData.title + "..."
                            color: Theme.textSecondary
                            font.pixelSize: 14
                        }
                    }
                }
                
                // Error display
                Rectangle {
                    anchors.fill: parent
                    color: Theme.background
                    visible: parent.status === Loader.Error
                    
                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: Theme.spacing * 2
                        
                        Text {
                            text: "⚠️"
                            font.pixelSize: 48
                            Layout.alignment: Qt.AlignHCenter
                        }
                        
                        Text {
                            text: "Failed to load extension"
                            color: Theme.error
                            font.pixelSize: 16
                            font.bold: true
                        }
                        
                        Text {
                            text: modelData.title
                            color: Theme.textSecondary
                            font.pixelSize: 14
                        }
                    }
                }
            }
        }

        // Settings / Configuration view (last page)
        Rectangle {
            color: Theme.background

            Behavior on color {
                ColorAnimation { duration: 200 }
            }

            // Full configuration screen
            ConfigScreen {
                anchors.fill: parent
                onClosed: {
                    // When closed from within, return to Home tab
                    tabBar.currentIndex = 0
                }
            }
        }
    }
    
    // Status bar
    footer: Rectangle {
        height: 30
        color: Theme.surface
        
        Behavior on color {
            ColorAnimation { duration: 200 }
        }
        
        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: Theme.padding
            anchors.rightMargin: Theme.padding
            spacing: Theme.spacing * 2
            
            Text {
                text: "🚗 Crankshaft Reborn v1.0.0"
                color: Theme.textSecondary
                font.pixelSize: 10
            }
            
            Rectangle {
                width: 1
                height: 16
                color: Theme.border
            }
            
            Text {
                text: "Extensions: " + ExtensionRegistry.componentCount
                color: Theme.textSecondary
                font.pixelSize: 10
            }
            
            Item { Layout.fillWidth: true }
            
            Text {
                text: new Date().toLocaleTimeString(Qt.locale(), "hh:mm")
                color: Theme.textSecondary
                font.pixelSize: 10
            }
        }
    }
    
    Component.onCompleted: {
        console.log("Main UI loaded");
        console.log("Registered extensions:", ExtensionRegistry.componentCount);
    }
}
