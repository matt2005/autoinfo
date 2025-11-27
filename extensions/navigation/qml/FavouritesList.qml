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

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

/**
 * Favourites list component
 */
Item {
    id: root
    
    // Signals
    signal favouriteSelected(var favourite)
    signal favouriteDeleted(int index)
    
    // Properties
    property var favourites: []
    
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
        color: root.surfaceVariant
        radius: 8
        
        ListView {
            id: favouritesListView
            anchors.fill: parent
            anchors.margins: root.paddingSize
            spacing: root.spacingSize
            clip: true
            visible: root.favourites.length > 0

            model: root.favourites

            delegate: Rectangle {
                width: parent.width
                height: 90
                color: root.surfaceColor
                radius: 6
                border.color: root.outlineColor
                border.width: 1
                
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: root.paddingSize
                        spacing: root.spacingSize
                    
                    // Star icon
                    Text {
                        text: "⭐"
                        font.pixelSize: 32
                        Layout.preferredWidth: 40
                    }
                    
                    // Info
                    Column {
                        Layout.fillWidth: true
                        spacing: 4
                        
                        Text {
                            text: modelData.name || "Unnamed"
                            font.pixelSize: 18
                            font.bold: true
                            color: root.textColor
                            elide: Text.ElideRight
                            width: parent.width
                        }
                        
                        Text {
                            text: modelData.address || ""
                            font.pixelSize: 12
                            color: root.textSecondary
                            elide: Text.ElideRight
                            width: parent.width
                            wrapMode: Text.WordWrap
                            maximumLineCount: 2
                        }
                        
                        Text {
                            text: "📍 " + modelData.latitude.toFixed(6) + ", " + modelData.longitude.toFixed(6)
                            font.pixelSize: 11
                            color: root.textSecondary
                            opacity: 0.7
                        }
                    }
                    
                    // Navigate button
                    StyledButton {
                        id: navBtn
                        Layout.preferredWidth: 60
                        Layout.preferredHeight: 60
                        text: "→"
                        Accessible.name: "Navigate to favourite"

                        background: Rectangle {
                            color: root.accentColor
                            radius: 8
                            opacity: navBtn.pressed ? 0.8 : 1.0
                        }

                        onClicked: {
                            root.favouriteSelected(modelData)
                        }
                    }
                    
                    // Delete button
                    StyledButton {
                        id: delBtn
                        Layout.preferredWidth: 60
                        Layout.preferredHeight: 60
                        text: "🗑️"
                        Accessible.name: "Delete favourite"

                        background: Rectangle {
                            color: root.errorColor
                            radius: 8
                            opacity: delBtn.pressed ? 0.8 : 0.6
                        }

                        onClicked: {
                            deleteDialog.favouriteIndex = index
                            deleteDialog.favouriteName = modelData.name
                            deleteDialog.open()
                        }
                    }
                }
            }
        }
        
        // Empty state
            Column {
            anchors.centerIn: parent
            spacing: root.spacingSize * 2
            visible: root.favourites.length === 0
            
            Text {
                text: "⭐"
                font.pixelSize: 64
                anchors.horizontalCenter: parent.horizontalCenter
                opacity: 0.3
            }
            
                Text {
                text: "No favourites yet"
                font.pixelSize: 18
                font.bold: true
                color: root.textColor
                anchors.horizontalCenter: parent.horizontalCenter
            }
            
            Text {
                text: "Search for a location and save it\nas a favourite for quick access"
                font.pixelSize: 14
                color: root.textSecondary
                horizontalAlignment: Text.AlignHCenter
                anchors.horizontalCenter: parent.horizontalCenter
                lineHeight: 1.5
            }
        }
    }
    
    // Delete confirmation dialog
    Dialog {
        id: deleteDialog
        anchors.centerIn: parent
        modal: true
        title: "Delete Favourite"
        width: Math.min(parent.width * 0.8, 400)
        implicitWidth: width
        
        property int favouriteIndex: -1
        property string favouriteName: ""
        
        contentItem: Column {
            spacing: root.spacingSize * 2
            padding: root.paddingSize * 2
            
            Text {
                text: "Delete '" + deleteDialog.favouriteName + "' from favourites?"
                font.pixelSize: 16
                color: root.textColor
                wrapMode: Text.WordWrap
                width: parent.width
            }
            
            Row {
                spacing: root.spacingSize
                anchors.horizontalCenter: parent.horizontalCenter
                
                StyledButton {
                    id: cancelBtn
                    text: "Cancel"
                    Accessible.name: "Cancel"
                    width: 120
                    height: 45

                    background: Rectangle {
                        color: root.surfaceVariant
                        radius: 6
                        border.color: root.outlineColor
                        border.width: 1
                    }

                    onClicked: deleteDialog.close()
                }

                StyledButton {
                    id: deleteBtn
                    text: "Delete"
                    Accessible.name: "Delete"
                    width: 120
                    height: 45

                    background: Rectangle {
                        color: root.errorColor
                        radius: 6
                    }

                    onClicked: {
                        root.favouriteDeleted(deleteDialog.favouriteIndex)
                        deleteDialog.close()
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
}
