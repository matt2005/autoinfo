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
import QtQuick.Dialogs
import CrankshaftReborn.UI 1.0
import Crankshaft.ConfigManagerBridge 1.0

Item {
    id: root
    
    property string domain: ""
    property string extension: ""
    property string sectionKey: ""
    property var itemData: null
    property int complexityLevel: 0
    
    height: itemLayout.height
    visible: itemData && (complexityIndex(itemData.complexity) <= complexityLevel)

    function complexityIndex(c) {
        var v = (c || "basic").toString().toLowerCase()
        if (v === "advanced") return 1
        if (v === "expert") return 2
        if (v === "developer" || v === "dev") return 3
        return 0
    }
    
    function getConfigKey() {
        return domain + "." + extension + "." + sectionKey + "." + (itemData ? itemData.key : "")
    }
    
    function loadValue() {
        if (!itemData) return
        
        var value = ConfigManagerBridge.getValue(getConfigKey())
        
        switch(itemData.type) {
            case "boolean":
                booleanSwitch.checked = value
                break
            case "integer":
                integerSpinBox.value = value
                break
            case "double":
                doubleSpinBox.value = value
                break
            case "string":
                stringTextField.text = value
                break
            case "selection":
                selectionComboBox.currentIndex = (itemData.properties && itemData.properties.options) ? itemData.properties.options.indexOf(value) : -1
                break
            case "multiselection":
                loadMultiSelection(value)
                break
            case "color":
                colorButton.selectedColor = value
                break
            case "file":
            case "directory":
                fileTextField.text = value
                break
        }
    }
    
    function loadMultiSelection(value) {
        multiSelectionRepeater.model = []
        var selectedValues = Array.isArray(value) ? value : []
        var items = []
        var opts = (itemData.properties && itemData.properties.options) ? itemData.properties.options : []
        for (var i = 0; i < opts.length; i++) {
            items.push({
                label: opts[i],
                checked: selectedValues.indexOf(opts[i]) !== -1
            })
        }
        multiSelectionRepeater.model = items
    }
    
    function saveValue(value) {
        if (!itemData || itemData.readOnly) return
        ConfigManagerBridge.setValue(getConfigKey(), value)
    }
    
    Component.onCompleted: loadValue()
    
    RowLayout {
        id: itemLayout
        width: parent.width
        spacing: 15
        
        // Label and description
        ColumnLayout {
            Layout.fillWidth: true
            Layout.minimumWidth: 200
            spacing: 3
            
            RowLayout {
                spacing: 5
                
                Text {
                    text: itemData ? itemData.label : ""
                    font.pixelSize: 13
                    font.bold: itemData && itemData.required
                    color: ThemeManager.textColor
                }
                
                Text {
                    text: "*"
                    font.pixelSize: 13
                    font.bold: true
                    color: "#F44336"
                    visible: itemData && itemData.required
                }
                
                Text {
                    text: "(read-only)"
                    font.pixelSize: 11
                    // Use textSecondaryColor (correct property name) with fallback
                    color: ThemeManager.textSecondaryColor || ThemeManager.textColor
                    visible: itemData && itemData.readOnly
                }
            }
            
            Text {
                text: itemData ? itemData.description : ""
                font.pixelSize: 11
                // Use textSecondaryColor (correct property name) with fallback
                color: ThemeManager.textSecondaryColor || ThemeManager.textColor
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
                visible: itemData && itemData.description !== ""
            }
        }
        
        // Input widget (type-specific)
        Item {
            Layout.preferredWidth: 300
            Layout.minimumHeight: 36
            
            // Boolean (type 0)
            Switch {
                id: booleanSwitch
                anchors.verticalCenter: parent.verticalCenter
                visible: itemData && itemData.type === "boolean"
                enabled: itemData && !itemData.readOnly
                onToggled: saveValue(checked)
            }
            
            // Integer (type 1)
            SpinBox {
                id: integerSpinBox
                anchors.verticalCenter: parent.verticalCenter
                width: 150
                visible: itemData && itemData.type === "integer"
                enabled: itemData && !itemData.readOnly
                from: itemData ? ((itemData.properties && itemData.properties.minValue !== undefined) ? itemData.properties.minValue : -2147483648) : -2147483648
                to: itemData ? ((itemData.properties && itemData.properties.maxValue !== undefined) ? itemData.properties.maxValue : 2147483647) : 2147483647
                stepSize: itemData ? ((itemData.properties && itemData.properties.step !== undefined) ? itemData.properties.step : 1) : 1
                editable: true
                onValueModified: saveValue(value)
                
                background: Rectangle {
                    color: ThemeManager.surfaceColor
                    border.color: ThemeManager.borderColor
                    border.width: 1
                    radius: 4
                }
                
                contentItem: TextInput {
                    text: integerSpinBox.textFromValue(integerSpinBox.value, integerSpinBox.locale)
                    font.pixelSize: 13
                    color: ThemeManager.textColor
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                    readOnly: !integerSpinBox.editable
                    validator: integerSpinBox.validator
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                }
            }
            
            // Double (type 2)
            SpinBox {
                id: doubleSpinBox
                anchors.verticalCenter: parent.verticalCenter
                width: 150
                visible: itemData && itemData.type === "double"
                enabled: itemData && !itemData.readOnly
                from: itemData ? ((itemData.properties && itemData.properties.minValue !== undefined) ? itemData.properties.minValue * 100 : -214748364) : -214748364
                to: itemData ? ((itemData.properties && itemData.properties.maxValue !== undefined) ? itemData.properties.maxValue * 100 : 214748364) : 214748364
                stepSize: itemData ? ((itemData.properties && itemData.properties.step !== undefined) ? itemData.properties.step * 100 : 10) : 10
                editable: true
                
                property int decimals: 2
                property real realValue: value / 100.0
                
                validator: DoubleValidator {
                    bottom: Math.min(doubleSpinBox.from, doubleSpinBox.to)
                    top: Math.max(doubleSpinBox.from, doubleSpinBox.to)
                    decimals: doubleSpinBox.decimals
                }
                
                textFromValue: function(value, locale) {
                    return Number(value / 100.0).toLocaleString(locale, 'f', decimals)
                }
                
                valueFromText: function(text, locale) {
                    return Number.fromLocaleString(locale, text) * 100
                }
                
                onValueModified: saveValue(realValue)
                
                background: Rectangle {
                    color: ThemeManager.surfaceColor
                    border.color: ThemeManager.borderColor
                    border.width: 1
                    radius: 4
                }
                
                contentItem: TextInput {
                    text: doubleSpinBox.textFromValue(doubleSpinBox.value, doubleSpinBox.locale)
                    font.pixelSize: 13
                    color: ThemeManager.textColor
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                    readOnly: !doubleSpinBox.editable
                    validator: doubleSpinBox.validator
                    inputMethodHints: Qt.ImhFormattedNumbersOnly
                }
            }
            
            // String (type 3)
            TextField {
                id: stringTextField
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                visible: itemData && itemData.type === "string"
                enabled: itemData && !itemData.readOnly
                placeholderText: itemData && itemData.properties ? itemData.properties.placeholder : ""
                font.pixelSize: 13
                color: ThemeManager.textColor
                echoMode: itemData && itemData.isSecret ? TextInput.Password : TextInput.Normal
                
                onEditingFinished: saveValue(text)
                
                background: Rectangle {
                    color: ThemeManager.surfaceColor
                    border.color: stringTextField.activeFocus ? ThemeManager.primaryColor : ThemeManager.borderColor
                    border.width: 1
                    radius: 4
                }
            }
            
            // Selection (type 4)
            ComboBox {
                id: selectionComboBox
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                visible: itemData && itemData.type === "selection"
                enabled: itemData && !itemData.readOnly
                model: itemData && itemData.properties ? itemData.properties.options : []
                font.pixelSize: 13
                
                onActivated: saveValue(currentText)
                
                background: Rectangle {
                    color: ThemeManager.surfaceColor
                    border.color: ThemeManager.borderColor
                    border.width: 1
                    radius: 4
                }
                
                contentItem: Text {
                    leftPadding: 10
                    rightPadding: selectionComboBox.indicator.width + selectionComboBox.spacing
                        // Guard against undefined displayText causing assignment warnings
                        text: selectionComboBox.displayText !== undefined ? selectionComboBox.displayText : ""
                    font: selectionComboBox.font
                    color: ThemeManager.textColor
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                }
            }
            
            // MultiSelection (type 5)
            ColumnLayout {
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                visible: itemData && itemData.type === "multiselection"
                spacing: 5
                
                Repeater {
                    id: multiSelectionRepeater
                    
                    delegate: CheckBox {
                        text: modelData.label
                        checked: modelData.checked
                        enabled: itemData && !itemData.readOnly
                        font.pixelSize: 12
                        
                        onToggled: {
                            var selectedValues = []
                            for (var i = 0; i < multiSelectionRepeater.count; i++) {
                                var item = multiSelectionRepeater.itemAt(i)
                                if (item && item.checked) {
                                    selectedValues.push(item.text)
                                }
                            }
                            saveValue(selectedValues)
                        }
                        
                        contentItem: Text {
                            text: parent.text
                            font: parent.font
                            color: ThemeManager.textColor
                            leftPadding: parent.indicator.width + parent.spacing
                            verticalAlignment: Text.AlignVCenter
                        }
                    }
                }
            }
            
            // Color (type 6)
            RowLayout {
                anchors.verticalCenter: parent.verticalCenter
                visible: itemData && itemData.type === "color"
                spacing: 10
                
                Rectangle {
                    id: colorPreview
                    width: 36
                    height: 36
                    color: colorButton.selectedColor
                    border.color: ThemeManager.borderColor
                    border.width: 1
                    radius: 4
                }
                
                Button {
                    id: colorButton
                    text: "Choose Color"
                    enabled: itemData && !itemData.readOnly
                    
                    property color selectedColor: "#FFFFFF"
                    
                    onClicked: colorDialog.open()
                    
                    background: Rectangle {
                        color: colorButton.pressed ? Qt.darker(ThemeManager.primaryColor, 1.2) : ThemeManager.primaryColor
                        radius: 4
                    }
                    
                    contentItem: Text {
                        text: colorButton.text
                        font.pixelSize: 12
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                
                ColorDialog {
                    id: colorDialog
                    title: "Choose a color"
                    selectedColor: colorButton.selectedColor
                    onAccepted: {
                        colorButton.selectedColor = selectedColor
                        saveValue(selectedColor.toString())
                    }
                }
            }
            
            // File/Directory (type 7/8)
            RowLayout {
                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                visible: itemData && (itemData.type === "file" || itemData.type === "directory")
                spacing: 5
                
                TextField {
                    id: fileTextField
                    Layout.fillWidth: true
                    enabled: itemData && !itemData.readOnly
                    placeholderText: itemData && itemData.type === "file" ? "Select file..." : "Select directory..."
                    font.pixelSize: 13
                    color: ThemeManager.textColor
                    
                    onEditingFinished: saveValue(text)
                    
                    background: Rectangle {
                        color: ThemeManager.surfaceColor
                        border.color: fileTextField.activeFocus ? ThemeManager.primaryColor : ThemeManager.borderColor
                        border.width: 1
                        radius: 4
                    }
                }
                
                Button {
                    text: "Browse"
                    enabled: itemData && !itemData.readOnly
                    
                    onClicked: {
                        if (itemData.type === "file") {
                            fileDialog.selectFolder = false
                        } else {
                            fileDialog.selectFolder = true
                        }
                        fileDialog.open()
                    }
                    
                    background: Rectangle {
                        color: parent.pressed ? Qt.darker(ThemeManager.primaryColor, 1.2) : ThemeManager.primaryColor
                        radius: 4
                    }
                    
                    contentItem: Text {
                        text: parent.text
                        font.pixelSize: 12
                        color: "white"
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
                
                FileDialog {
                    id: fileDialog
                    title: itemData && itemData.type === "file" ? "Select File" : "Select Directory"
                    currentFolder: fileTextField.text || ""
                    onAccepted: {
                        var path = selectedFile.toString().replace("file:///", "")
                        fileTextField.text = path
                        saveValue(path)
                    }
                }
            }
            
            // Custom (type 9) - Placeholder
            Text {
                anchors.verticalCenter: parent.verticalCenter
                visible: itemData && itemData.type === "custom"
                text: "Custom widget not implemented"
                font.pixelSize: 12
                // Use textSecondaryColor (correct property name) with fallback
                color: ThemeManager.textSecondaryColor || ThemeManager.textColor
                font.italic: true
            }
        }
        
        // Unit label (if provided)
        Text {
            text: itemData ? itemData.unit : ""
            font.pixelSize: 12
            // Use textSecondaryColor (correct property name) with fallback
            color: ThemeManager.textSecondaryColor || ThemeManager.textColor
            visible: itemData && itemData.unit !== ""
            Layout.alignment: Qt.AlignVCenter
        }
    }
}
