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
import Crankshaft.ConfigManagerBridge 1.0

Item {
    id: root
    
    property string domain: ""
    property string extension: ""
    property int complexityLevel: 0
    
    function refresh() {
        loadPage()
    }
    
    function loadPage() {
        if (!domain || !extension) {
            return
        }
        
        var page = ConfigManagerBridge.getConfigPage(domain, extension)
        if (!page || !page.sections) {
            sectionsModel.clear()
            return
        }
        
        sectionsModel.clear()
        
        // Filter sections by complexity level
        for (var i = 0; i < page.sections.length; i++) {
            var section = page.sections[i]
            if (section.complexity <= complexityLevel) {
                sectionsModel.append(section)
            }
        }
        
        pageTitle.text = page.title || ""
        pageDescription.text = page.description || ""
    }
    
    onDomainChanged: loadPage()
    onExtensionChanged: loadPage()
    onComplexityLevelChanged: loadPage()
    
    Component.onCompleted: loadPage()
    
    ListModel {
        id: sectionsModel
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 15
        
        // Page header
        ColumnLayout {
            Layout.fillWidth: true
            spacing: 5
            visible: pageTitle.text !== ""
            
            Text {
                id: pageTitle
                font.pixelSize: 20
                font.bold: true
                color: ThemeManager.textColor
                Layout.fillWidth: true
            }
            
            Text {
                id: pageDescription
                font.pixelSize: 13
                color: ThemeManager.secondaryTextColor
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }
            
            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: ThemeManager.borderColor
            }
        }
        
        // Sections list
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            
            ColumnLayout {
                width: parent.width
                spacing: 20
                
                Repeater {
                    model: sectionsModel
                    
                    delegate: ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 10
                        
                        // Section header
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            
                            Image {
                                source: model.icon || ""
                                width: 24
                                height: 24
                                visible: model.icon !== ""
                            }
                            
                            Text {
                                text: model.title
                                font.pixelSize: 16
                                font.bold: true
                                color: ThemeManager.textColor
                                Layout.fillWidth: true
                            }
                            
                            // Complexity badge
                            Rectangle {
                                width: complexityText.width + 16
                                height: 20
                                color: {
                                    switch(model.complexity) {
                                        case 0: return "#4CAF50"  // Basic - Green
                                        case 1: return "#FF9800"  // Advanced - Orange
                                        case 2: return "#F44336"  // Expert - Red
                                        case 3: return "#9C27B0"  // Developer - Purple
                                        default: return "#9E9E9E"
                                    }
                                }
                                radius: 10
                                visible: model.complexity > 0
                                
                                Text {
                                    id: complexityText
                                    anchors.centerIn: parent
                                    text: {
                                        switch(model.complexity) {
                                            case 0: return "Basic"
                                            case 1: return "Advanced"
                                            case 2: return "Expert"
                                            case 3: return "Developer"
                                            default: return ""
                                        }
                                    }
                                    color: "white"
                                    font.pixelSize: 11
                                    font.bold: true
                                }
                            }
                        }
                        
                        Text {
                            text: model.description
                            font.pixelSize: 12
                            color: ThemeManager.secondaryTextColor
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                            visible: model.description !== ""
                        }
                        
                        // Section items
                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: 10
                            spacing: 15
                            
                            Repeater {
                                model: items
                                
                                delegate: ConfigItemView {
                                    Layout.fillWidth: true
                                    domain: root.domain
                                    extension: root.extension
                                    sectionKey: key
                                    itemData: modelData
                                    complexityLevel: root.complexityLevel
                                }
                            }
                        }
                        
                        // Section separator
                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: ThemeManager.borderColor
                            opacity: 0.3
                        }
                    }
                }
                
                // Empty state
                Text {
                    text: domain && extension ? 
                          "No configuration options available at this complexity level." :
                          "Select an extension to view its configuration."
                    font.pixelSize: 14
                    color: ThemeManager.secondaryTextColor
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                    Layout.topMargin: 50
                    visible: sectionsModel.count === 0
                }
            }
        }
    }
}
