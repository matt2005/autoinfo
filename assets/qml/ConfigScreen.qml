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
import Crankshaft.ConfigManagerBridge 1.0

Rectangle {
    id: root
    color: ThemeManager.backgroundColor
    
    property string currentDomain: "core"
    property string currentExtension: ""
    property int currentComplexity: 0  // 0=Basic, 1=Advanced, 2=Expert, 3=Developer
    
    // Allow setting initial page from outside
    property string initialDomain: ""
    property string initialExtension: ""
    
    signal closed()
    
    Component.onCompleted: {
        loadConfigPages()
    }
    
    // Domain/page data for sidebar; array of { domain, pages: [{extension,title}] }
    property var domainsData: []

    function complexityToIndex(c) {
        var v = (c || "basic").toString().toLowerCase()
        if (v === "advanced") return 1
        if (v === "expert") return 2
        if (v === "developer" || v === "dev") return 3
        return 0
    }

    function loadConfigPages() {
        var pages = ConfigManagerBridge.getAllConfigPages()
        var grouped = {}

        // Group pages by domain, filtering by complexity level
        for (var i = 0; i < pages.length; i++) {
            var page = pages[i]
            // Filter pages by complexity level
            if (complexityToIndex(page.complexity) > currentComplexity) {
                continue
            }
            if (!grouped[page.domain]) {
                grouped[page.domain] = []
            }
            grouped[page.domain].push({ extension: page.extension, title: page.title })
        }

        // Convert to ordered array for UI
        var result = []
        for (var d in grouped) {
            result.push({ domain: d, pages: grouped[d] })
        }
        domainsData = result

        // Select initial or first domain and extension
        if (initialDomain !== "" && initialExtension !== "") {
            currentDomain = initialDomain
            currentExtension = initialExtension
        } else if (domainsData.length > 0) {
            currentDomain = domainsData[0].domain
            var firstPages = domainsData[0].pages || []
            if (firstPages.length > 0) {
                currentExtension = firstPages[0].extension
            } else {
                currentExtension = ""
            }
        } else {
            currentDomain = ""
            currentExtension = ""
        }
    }

    // Programmatic navigation helper
    function openPage(domain, extension) {
        if (!domainsData || domainsData.length === 0) return;
        for (var i = 0; i < domainsData.length; ++i) {
            var d = domainsData[i];
            if (d.domain === domain) {
                for (var j = 0; j < d.pages.length; ++j) {
                    if (d.pages[j].extension === extension) {
                        currentDomain = domain;
                        currentExtension = extension;
                        return;
                    }
                }
            }
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 10
        
        // Header with title and complexity selector
        RowLayout {
            Layout.fillWidth: true
            spacing: 20
            
            Text {
                text: "Configuration"
                font.pixelSize: 24
                font.bold: true
                color: ThemeManager.textColor
                Layout.fillWidth: true
            }
            
            // Complexity level selector
            RowLayout {
                spacing: 10
                
                Text {
                    text: "Level:"
                    color: ThemeManager.textColor
                }
                
                ComboBox {
                    id: complexityCombo
                    model: ["Basic", "Advanced", "Expert", "Developer"]
                    currentIndex: currentComplexity
                    onCurrentIndexChanged: {
                        currentComplexity = currentIndex
                        ConfigManagerBridge.setComplexityLevel(model[currentIndex])
                        // Reload pages when complexity changes
                        loadConfigPages()
                    }
                }
            }
            
            // Close button
            StyledButton {
                text: "Close"
                onClicked: root.closed()
            }
        }
        
        // Main content area
        RowLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 10
            
            // Left sidebar - Domain and extension selector
            Rectangle {
                Layout.preferredWidth: 250
                Layout.fillHeight: true
                color: ThemeManager.cardColor
                border.color: ThemeManager.borderColor
                border.width: 1
                radius: 5

                ColumnLayout {
                    anchors.fill: parent
                    anchors.margins: 10
                    spacing: 5

                    Text {
                        text: "Categories"
                        font.pixelSize: 16
                        font.bold: true
                        color: ThemeManager.textColor
                    }

                    // Use JS-array-backed model to support nested pages without ListModel role limits
                    ListView {
                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        clip: true
                        model: domainsData
                        ScrollBar.vertical: ScrollBar {
                            policy: ScrollBar.AsNeeded
                        }

                        delegate: ColumnLayout {
                            width: parent ? parent.width : 0
                            spacing: 2
                            property string domainName: (modelData && modelData.domain) ? modelData.domain : ""

                            // Domain header
                            Rectangle {
                                Layout.fillWidth: true
                                height: 35
                                color: "transparent"

                                Text {
                                    anchors.left: parent.left
                                    anchors.leftMargin: 5
                                    anchors.verticalCenter: parent.verticalCenter
                                    text: (domainName || "").toUpperCase()
                                    font.pixelSize: 14
                                    font.bold: true
                                    color: ThemeManager.accentColor
                                }
                            }

                            // Extension list for this domain
                            Repeater {
                                model: (modelData && modelData.pages) ? modelData.pages : []

                                Rectangle {
                                    Layout.fillWidth: true
                                    height: 40
                                    color: (modelData && modelData.extension && currentExtension === modelData.extension &&
                                           currentDomain === domainName) ? ThemeManager.accentColor : "transparent"
                                    radius: 3

                                    MouseArea {
                                        anchors.fill: parent
                                        onClicked: {
                                            currentDomain = domainName
                                            if (modelData && modelData.extension)
                                                currentExtension = modelData.extension
                                        }
                                    }

                                    Text {
                                        anchors.left: parent.left
                                        anchors.leftMargin: 15
                                        anchors.verticalCenter: parent.verticalCenter
                                        text: (modelData && modelData.title) ? modelData.title : ""
                                        font.pixelSize: 13
                                        color: (modelData && modelData.extension && currentExtension === modelData.extension &&
                                               currentDomain === domainName) ? "white" : ThemeManager.textColor
                                    }
                                }
                            }
                        }
                    }
                }
            }
            
            // Right side - Configuration page view
            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: ThemeManager.cardColor
                border.color: ThemeManager.borderColor
                border.width: 1
                radius: 5
                
                ConfigPageView {
                    id: pageView
                    anchors.fill: parent
                    anchors.margins: 10
                    domain: currentDomain
                    extension: currentExtension
                    complexityLevel: currentComplexity
                }
            }
        }
        
        // Bottom action bar
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            StyledButton {
                text: "Reset to Defaults"
                onClicked: {
                    if (currentDomain && currentExtension) {
                        ConfigManagerBridge.resetToDefaults(currentDomain, currentExtension)
                        pageView.refresh()
                    }
                }
            }
            
            StyledButton {
                text: "Export Config..."
                onClicked: exportDialog.open()
            }
            
            StyledButton {
                text: "Import Config..."
                onClicked: importDialog.open()
            }
            
            StyledButton {
                text: "Backup..."
                onClicked: backupDialog.open()
            }
            
            Item {
                Layout.fillWidth: true
            }
            
            StyledButton {
                text: "Save"
                highlighted: true
                onClicked: {
                    if (ConfigManagerBridge.save()) {
                        console.log("Configuration saved successfully")
                    } else {
                        console.error("Failed to save configuration")
                    }
                }
            }
        }
    }
    
    // Export dialog
    Dialog {
        id: exportDialog
        title: "Export Configuration"
        modal: true
        anchors.centerIn: parent
        width: 400
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 10
            
            CheckBox {
                id: maskSecretsCheck
                text: "Mask secret values"
                checked: true
            }
            
            Text {
                text: "Export configuration to share with others or backup."
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                color: ThemeManager.textColor
            }
        }
        
        standardButtons: Dialog.Ok | Dialog.Cancel
        
        onAccepted: {
            var config = ConfigManagerBridge.exportConfig(maskSecretsCheck.checked)
            // TODO: Show save file dialog and save JSON
            console.log("Exported config:", JSON.stringify(config, null, 2))
        }
    }
    
    // Import dialog
    Dialog {
        id: importDialog
        title: "Import Configuration"
        modal: true
        anchors.centerIn: parent
        width: 400
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 10
            
            CheckBox {
                id: overwriteCheck
                text: "Overwrite existing settings"
                checked: false
            }
            
            Text {
                text: "Import configuration from a file. Uncheck 'Overwrite' to merge with existing settings."
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                color: ThemeManager.textColor
            }
        }
        
        standardButtons: Dialog.Ok | Dialog.Cancel
        
        onAccepted: {
            // TODO: Show open file dialog and load JSON
            // ConfigManagerBridge.importConfig(config, overwriteCheck.checked)
            console.log("Import requested")
        }
    }
    
    // Backup dialog
    Dialog {
        id: backupDialog
        title: "Backup Configuration"
        modal: true
        anchors.centerIn: parent
        width: 400
        
        ColumnLayout {
            anchors.fill: parent
            spacing: 10
            
            CheckBox {
                id: compressCheck
                text: "Compress backup file (.gz)"
                checked: true
            }
            
            CheckBox {
                id: includSecretsCheck
                text: "Include secret values"
                checked: false
            }
            
            Text {
                text: "Create a compressed backup file of your configuration."
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                color: ThemeManager.textColor
            }
        }
        
        standardButtons: Dialog.Ok | Dialog.Cancel
        
        onAccepted: {
            // TODO: Show save file dialog
            // ConfigManagerBridge.backupToFile(filePath, !includeSecretsCheck.checked, compressCheck.checked)
            console.log("Backup requested")
        }
    }
}
