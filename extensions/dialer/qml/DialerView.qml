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
import CrankshaftReborn.Events 1.0

pragma ComponentBehavior: Bound

Item {
  id: root
  anchors.fill: parent

  // Theme convenience properties to avoid unqualified-access warnings in nested components
  readonly property color backgroundColor: ThemeManager.backgroundColor
  readonly property int spacingSize: ThemeManager.spacing
  readonly property int paddingSize: ThemeManager.padding
  readonly property color textColor: ThemeManager.textColor
  readonly property color surfaceColor: ThemeManager.surfaceColor
  readonly property int cornerRadius: ThemeManager.cornerRadius
  readonly property color borderColor: ThemeManager.borderColor

  Rectangle {
    anchors.fill: parent
    color: root.backgroundColor
  }

  ColumnLayout {
    anchors.centerIn: parent
    spacing: root.spacingSize * 2
    width: 360

      Text {
      text: "Dialler"
      font.pixelSize: 24
      font.bold: true
      color: root.textColor
      horizontalAlignment: Text.AlignHCenter
      Layout.alignment: Qt.AlignHCenter
    }

    Rectangle {
      Layout.fillWidth: true
      height: 56
      radius: root.cornerRadius
      color: root.surfaceColor
      border.color: root.borderColor
      border.width: 1

      RowLayout {
        anchors.fill: parent
        anchors.margins: root.paddingSize
        spacing: root.spacingSize

        TextField {
          id: numberField
          Layout.fillWidth: true
          placeholderText: "Enter number"
          text: ""
          Accessible.name: "Dialler number field"
        }

        Button {
          text: "⌫"
          Accessible.name: "Backspace"
          onClicked: numberField.text = numberField.text.slice(0, -1)
        }
      }
    }

    GridLayout {
      columns: 3
      columnSpacing: root.spacingSize
      rowSpacing: root.spacingSize
      Layout.fillWidth: true

      function key(label) {
        return label
      }

      Repeater {
        model: ["1","2","3","4","5","6","7","8","9","*","0","#"]
        Button {
          text: modelData
          Accessible.name: "Key " + modelData
          onClicked: numberField.text += text
          Layout.fillWidth: true
          Layout.preferredHeight: 48
        }
      }
    }

    RowLayout {
      Layout.fillWidth: true
      spacing: root.spacingSize
      Button {
        text: "Call"
        Accessible.name: "Call"
        Layout.fillWidth: true
        onClicked: {
          console.log("Dialler: Call requested", numberField.text);
          // Emit a public dial event; Bluetooth listens on "*.phone.dial"
          EventBridge.publish("dialer.phone.dial", { number: numberField.text })
        }
      }
      Button {
        text: "Clear"
        Accessible.name: "Clear"
        Layout.fillWidth: true
        onClicked: numberField.text = ""
      }
    }
  }
}
