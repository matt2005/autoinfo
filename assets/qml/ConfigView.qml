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
import CrankshaftReborn.UI 1.0

Item {
    id: configView
    
    Rectangle {
        anchors.fill: parent
        color: ThemeManager.backgroundColor
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: 20
            spacing: 20
            
            // Header
            RowLayout {
                Layout.fillWidth: true
                spacing: 15
                
                Text {
                    text: "⚙️ Configuration"
                    font.pixelSize: 28
                    font.bold: true
                    color: ThemeManager.textColor
                }
                
                Item {
                    Layout.fillWidth: true
                }
                
                Button {
                    text: "↻ Refresh"
                    onClicked: extensionList.refreshExtensions()
                }
            }
            
            // Tab Bar
            TabBar {
                id: tabBar
                Layout.fillWidth: true
                
                TabButton {
                    text: "Extensions"
                }
                
                TabButton {
                    text: "System"
                }
                
                TabButton {
                    text: "Display"
                }
                
                TabButton {
                    text: "About"
                }
            }
            
            // Content
            StackLayout {
                Layout.fillWidth: true
                Layout.fillHeight: true
                currentIndex: tabBar.currentIndex
                
                // Extensions Tab
                ExtensionManagerView {
                    id: extensionList
                }
                
                // System Tab
                Item {
                    Text {
                        anchors.centerIn: parent
                        text: "System Settings"
                        font.pixelSize: 24
                        color: ThemeManager.textColor
                    }
                }
                
                // Display Tab
                Item {
                    Text {
                        anchors.centerIn: parent
                        text: "Display Settings"
                        font.pixelSize: 24
                        color: ThemeManager.textColor
                    }
                }
                
                // About Tab
                Item {
                    ColumnLayout {
                        anchors.centerIn: parent
                        spacing: 15
                        
                        Text {
                            text: "Crankshaft Reborn"
                            font.pixelSize: 32
                            font.bold: true
                            color: ThemeManager.textColor
                            Layout.alignment: Qt.AlignHCenter
                        }
                        
                        Text {
                            text: "Version 1.0.0"
                            font.pixelSize: 18
                            color: ThemeManager.textColor
                            Layout.alignment: Qt.AlignHCenter
                        }
                        
                        Text {
                            text: "© 2025 OpenCarDev Team"
                            font.pixelSize: 14
                            color: ThemeManager.textColor
                            opacity: 0.7
                            Layout.alignment: Qt.AlignHCenter
                        }
                    }
                }
            }
        }
    }
}
