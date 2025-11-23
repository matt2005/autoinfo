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

Item {
  id: root
  anchors.fill: parent

  Rectangle {
    anchors.fill: parent
    color: ThemeManager.backgroundColor
  }

  ColumnLayout {
    anchors.centerIn: parent
    spacing: ThemeManager.spacing * 2
    width: 360

    Text {
      text: "Dialler"
      font.pixelSize: 24
      font.bold: true
      color: ThemeManager.textColor
      horizontalAlignment: Text.AlignHCenter
      Layout.alignment: Qt.AlignHCenter
    }

    Rectangle {
      Layout.fillWidth: true
      height: 56
      radius: ThemeManager.cornerRadius
      color: ThemeManager.surfaceColor
      border.color: ThemeManager.borderColor
      border.width: 1

      RowLayout {
        anchors.fill: parent
        anchors.margins: ThemeManager.padding
        spacing: ThemeManager.spacing

        TextField {
          id: numberField
          Layout.fillWidth: true
          placeholderText: "Enter number"
          text: ""
        }

        Button {
          text: "⌫"
          onClicked: numberField.text = numberField.text.slice(0, -1)
        }
      }
    }

    GridLayout {
      columns: 3
      columnSpacing: ThemeManager.spacing
      rowSpacing: ThemeManager.spacing
      Layout.fillWidth: true

      function key(label) {
        return label
      }

      Repeater {
        model: ["1","2","3","4","5","6","7","8","9","*","0","#"]
        Button {
          text: modelData
          onClicked: numberField.text += text
          Layout.fillWidth: true
          Layout.preferredHeight: 48
        }
      }
    }

    RowLayout {
      Layout.fillWidth: true
      spacing: ThemeManager.spacing
      Button {
        text: "Call"
        Layout.fillWidth: true
        onClicked: {
          console.log("Dialler: Call requested", numberField.text)
          // Emit a public dial event; Bluetooth listens on "*.phone.dial"
          EventBridge.publish("dialer.phone.dial", { number: numberField.text })
        }
      }
      Button {
        text: "Clear"
        Layout.fillWidth: true
        onClicked: numberField.text = ""
      }
    }
  }
}
