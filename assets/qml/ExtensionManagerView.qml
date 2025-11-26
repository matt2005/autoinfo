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
    id: extensionManagerView
    
    property var allExtensions: []
    
    function refreshExtensions() {
        allExtensions = ExtensionManagerBridge.getAvailableExtensions();
        extensionListModel.clear();
        for (var i = 0; i < allExtensions.length; i++) {
            extensionListModel.append(allExtensions[i]);
        }
    }
    
    Component.onCompleted: {
        refreshExtensions();
    }
    
    Connections {
        target: ExtensionManagerBridge
        
        function onExtensionLoaded(extensionId) {
            refreshExtensions();
        }
        
        function onExtensionUnloaded(extensionId) {
            refreshExtensions();
        }
        
        function onExtensionsRefreshed() {
            refreshExtensions();
        }
        
        function onExtensionError(extensionId, error) {
            errorDialog.extensionId = extensionId;
            errorDialog.errorText = error;
            errorDialog.open();
        }
    }
    
    ColumnLayout {
        anchors.fill: parent
        spacing: 10
        
        // Filter Controls
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            Text {
                text: "Filter:"
                color: ThemeManager.textColor
                font.pixelSize: 14
            }
            
            ComboBox {
                id: domainFilter
                Layout.preferredWidth: 150
                model: ["All", "Core", "Third-Party"]
                currentIndex: 0
                onCurrentIndexChanged: filterExtensions()
            }
            
            ComboBox {
                id: statusFilter
                Layout.preferredWidth: 150
                model: ["All", "Loaded", "Not Loaded", "Enabled", "Disabled"]
                currentIndex: 0
                onCurrentIndexChanged: filterExtensions()
            }
            
            Item {
                Layout.fillWidth: true
            }
            
            TextField {
                id: searchField
                Layout.preferredWidth: 200
                placeholderText: "Search extensions..."
                onTextChanged: filterExtensions()
            }
        }
        
        // Extension List
        Rectangle {
            Layout.fillWidth: true
            Layout.fillHeight: true
            color: ThemeManager.cardColor
            radius: 8
            border.color: ThemeManager.borderColor
            border.width: 1
            
            ListView {
                id: extensionListView
                anchors.fill: parent
                anchors.margins: 10
                spacing: 8
                clip: true
                
                model: ListModel {
                    id: extensionListModel
                }
                
                delegate: Rectangle {
                    width: extensionListView.width - 20
                    height: 100
                    color: model.loaded ? ThemeManager.accentColor : ThemeManager.backgroundColor
                    opacity: model.enabled ? 1.0 : 0.6
                    radius: 6
                    border.color: ThemeManager.borderColor
                    border.width: 1
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 15
                        
                        // Extension Info
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 5
                            
                            RowLayout {
                                Layout.fillWidth: true
                                spacing: 10
                                
                                Text {
                                    text: model.name
                                    font.pixelSize: 18
                                    font.bold: true
                                    color: ThemeManager.textColor
                                }
                                
                                Rectangle {
                                    width: domainBadgeText.width + 12
                                    height: 20
                                    radius: 10
                                    color: model.domain === "core" ? "#4CAF50" : "#FF9800"
                                    
                                    Text {
                                        id: domainBadgeText
                                        anchors.centerIn: parent
                                        text: model.domain
                                        font.pixelSize: 10
                                        font.bold: true
                                        color: "white"
                                    }
                                }
                                
                                Rectangle {
                                    visible: model.loaded
                                    width: loadedBadgeText.width + 12
                                    height: 20
                                    radius: 10
                                    color: "#2196F3"
                                    
                                    Text {
                                        id: loadedBadgeText
                                        anchors.centerIn: parent
                                        text: "Loaded"
                                        font.pixelSize: 10
                                        font.bold: true
                                        color: "white"
                                    }
                                }
                            }
                            
                            Text {
                                text: model.description
                                font.pixelSize: 12
                                color: ThemeManager.textColor
                                opacity: 0.8
                                wrapMode: Text.WordWrap
                                Layout.fillWidth: true
                                maximumLineCount: 2
                                elide: Text.ElideRight
                            }
                            
                            Text {
                                text: "v" + model.version + " • " + model.author
                                font.pixelSize: 10
                                color: ThemeManager.textColor
                                opacity: 0.6
                            }
                        }
                        
                        // Action Buttons
                        ColumnLayout {
                            Layout.alignment: Qt.AlignVCenter
                            spacing: 5
                            
                            Button {
                                text: model.loaded ? "🔄 Reload" : "▶️ Load"
                                enabled: model.enabled
                                visible: model.domain === "thirdparty"
                                Layout.preferredWidth: 100
                                onClicked: {
                                    if (model.loaded) {
                                        ExtensionManagerBridge.reloadExtension(model.id);
                                    }
                                }
                            }
                            
                            Button {
                                text: model.enabled ? "🚫 Disable" : "✅ Enable"
                                Layout.preferredWidth: 100
                                onClicked: {
                                    if (model.enabled) {
                                        ExtensionManagerBridge.disableExtension(model.id);
                                    } else {
                                        ExtensionManagerBridge.enableExtension(model.id);
                                    }
                                }
                            }
                            
                            Button {
                                text: "ℹ️ Info"
                                Layout.preferredWidth: 100
                                onClicked: {
                                    extensionInfoDialog.extensionId = model.id;
                                    extensionInfoDialog.open();
                                }
                            }
                        }
                    }
                    
                    MouseArea {
                        anchors.fill: parent
                        z: -1
                        onClicked: {
                            extensionInfoDialog.extensionId = model.id;
                            extensionInfoDialog.open();
                        }
                    }
                }
                
                ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AlwaysOn
                }
            }
        }
    }
    
    function filterExtensions() {
        extensionListModel.clear();
        
        for (var i = 0; i < allExtensions.length; i++) {
            var ext = allExtensions[i];
            var matchesDomain = true;
            var matchesStatus = true;
            var matchesSearch = true;
            
            // Domain filter
            if (domainFilter.currentIndex === 1) {
                matchesDomain = ext.domain === "core";
            } else if (domainFilter.currentIndex === 2) {
                matchesDomain = ext.domain === "thirdparty";
            }
            
            // Status filter
            if (statusFilter.currentIndex === 1) {
                matchesStatus = ext.loaded;
            } else if (statusFilter.currentIndex === 2) {
                matchesStatus = !ext.loaded;
            } else if (statusFilter.currentIndex === 3) {
                matchesStatus = ext.enabled;
            } else if (statusFilter.currentIndex === 4) {
                matchesStatus = !ext.enabled;
            }
            
            // Search filter
            if (searchField.text.length > 0) {
                var searchLower = searchField.text.toLowerCase();
                matchesSearch = ext.name.toLowerCase().includes(searchLower) ||
                               ext.id.toLowerCase().includes(searchLower) ||
                               ext.description.toLowerCase().includes(searchLower);
            }
            
            if (matchesDomain && matchesStatus && matchesSearch) {
                extensionListModel.append(ext);
            }
        }
    }
    
    // Extension Info Dialog
    Dialog {
        id: extensionInfoDialog
        property string extensionId: ""
        
        title: "Extension Information"
        modal: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 500
        height: 400
        
        contentItem: ScrollView {
            clip: true
            
            ColumnLayout {
                width: parent.width
                spacing: 10
                
                Text {
                    text: {
                        if (extensionInfoDialog.extensionId) {
                            var info = ExtensionManagerBridge.getExtensionInfo(extensionInfoDialog.extensionId);
                            var text = "<b>Name:</b> " + info.name + "<br>";
                            text += "<b>ID:</b> " + info.id + "<br>";
                            text += "<b>Version:</b> " + info.version + "<br>";
                            text += "<b>Author:</b> " + info.author + "<br>";
                            text += "<b>Domain:</b> " + info.domain + "<br>";
                            text += "<b>Type:</b> " + info.type + "<br>";
                            text += "<b>Description:</b> " + info.description + "<br><br>";
                            
                            if (info.dependencies && info.dependencies.length > 0) {
                                text += "<b>Dependencies:</b> " + info.dependencies.join(", ") + "<br>";
                            }
                            
                            if (info.permissions && info.permissions.length > 0) {
                                text += "<b>Permissions:</b> " + info.permissions.join(", ") + "<br>";
                            }
                            
                            return text;
                        }
                        return "No information available";
                    }
                    textFormat: Text.RichText
                    wrapMode: Text.WordWrap
                    color: ThemeManager.textColor
                    Layout.fillWidth: true
                }
            }
        }
        
        standardButtons: Dialog.Ok
    }
    
    // Error Dialog
    Dialog {
        id: errorDialog
        property string extensionId: ""
        property string errorText: ""
        
        title: "Extension Error"
        modal: true
        x: (parent.width - width) / 2
        y: (parent.height - height) / 2
        width: 400
        
        contentItem: ColumnLayout {
            spacing: 10
            
            Text {
                text: "<b>Extension:</b> " + errorDialog.extensionId
                textFormat: Text.RichText
                color: ThemeManager.textColor
            }
            
            Text {
                text: "<b>Error:</b> " + errorDialog.errorText
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
                color: "#F44336"
                Layout.fillWidth: true
            }
        }
        
        standardButtons: Dialog.Ok
    }
}
