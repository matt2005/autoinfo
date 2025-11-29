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
import CrankshaftReborn.UI 1.0
import Crankshaft.ConfigManagerBridge 1.0

Item {
    id: root
    
    property string domain: ""
    property string extension: ""
    property int complexityLevel: 0
    
    function refresh() {
        loadPage()
    }
    
    function complexityToIndex(c) {
        var v = (c || "basic").toString().toLowerCase()
        if (v === "advanced") return 1
        if (v === "expert") return 2
        if (v === "developer" || v === "dev") return 3
        return 0
    }

    function loadPage() {
        if (!domain || !extension) {
            return
        }
        
        var page = ConfigManagerBridge.getConfigPage(domain, extension)
        if (!page || !page.sections) {
            sectionsData = []
            return
        }
        
        // Filter sections by complexity level
        var filtered = []
        for (var i = 0; i < page.sections.length; i++) {
            var section = page.sections[i]
            if (complexityToIndex(section.complexity) <= complexityLevel) {
                filtered.push(section)
            }
        }
        sectionsData = filtered
        
        pageTitle.text = page.title || ""
        pageDescription.text = page.description || ""
    }
    
    onDomainChanged: loadPage()
    onExtensionChanged: loadPage()
    onComplexityLevelChanged: loadPage()
    
    Component.onCompleted: loadPage()
    
    property var sectionsData: []
    
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
                color: ThemeManager.textSecondaryColor
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
            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AlwaysOn
            }
            
            ColumnLayout {
                Layout.fillWidth: true
                spacing: 20
                
                Repeater {
                    model: sectionsData ? sectionsData.length : 0
                    
                    delegate: ColumnLayout {
                        property var section: sectionsData[index]
                        Layout.fillWidth: true
                        spacing: 10
                        
                        // Section header
                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 10
                            
                            Image {
                                source: section && section.icon || ""
                                Layout.preferredWidth: 24
                                Layout.preferredHeight: 24
                                visible: section && section.icon !== ""
                            }
                            
                            Text {
                                text: section ? section.title : ""
                                font.pixelSize: 16
                                font.bold: true
                                color: ThemeManager.textColor
                                Layout.fillWidth: true
                            }
                            
                            // Complexity badge
                            Rectangle {
                                Layout.preferredWidth: complexityText.width + 16
                                Layout.preferredHeight: 20
                                color: {
                                    var c = section && (section.complexity || "basic").toString().toLowerCase()
                                    if (c === "basic") return "#4CAF50"
                                    if (c === "advanced") return "#FF9800"
                                    if (c === "expert") return "#F44336"
                                    if (c === "developer" || c === "dev") return "#9C27B0"
                                    return "#9E9E9E"
                                }
                                radius: 10
                                visible: section && complexityToIndex(section.complexity) > 0
                                
                                Text {
                                    id: complexityText
                                    anchors.centerIn: parent
                                    text: {
                                        var c = section && (section.complexity || "basic").toString().toLowerCase()
                                        if (c === "basic") return "Basic"
                                        if (c === "advanced") return "Advanced"
                                        if (c === "expert") return "Expert"
                                        if (c === "developer" || c === "dev") return "Developer"
                                        return ""
                                    }
                                    color: "white"
                                    font.pixelSize: 11
                                    font.bold: true
                                }
                            }
                        }
                        
                        Text {
                            text: section ? section.description : ""
                            font.pixelSize: 12
                            color: ThemeManager.textSecondaryColor
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                            visible: section && section.description !== ""
                        }
                        
                        // Section items
                        ColumnLayout {
                            Layout.fillWidth: true
                            Layout.leftMargin: 10
                            spacing: 15
                            
                            Repeater {
                                model: section && section.items ? section.items.length : 0
                                delegate: ConfigItemView {
                                    Layout.fillWidth: true
                                    domain: root.domain
                                    extension: root.extension
                                    sectionKey: section ? section.key : ""
                                    itemData: section ? section.items[index] : null
                                    complexityLevel: root.complexityLevel
                                }
                            }
                        }
                        
                        // Section separator
                        Rectangle {
                            Layout.fillWidth: true
                            Layout.preferredHeight: 1
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
                    color: ThemeManager.textSecondaryColor
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                    Layout.topMargin: 50
                    visible: !sectionsData || sectionsData.length === 0
                }
            }
        }
    }
}
