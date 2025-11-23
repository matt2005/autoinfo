// []: # ```plaintext
// []: #  * Project: Crankshaft
// []: #  * This file is part of Crankshaft project.
// []: #  * Copyright (C) 2025 OpenCarDev Team
// []: #  *
// []: #  *  Crankshaft is free software: you can redistribute it and/or modify
// []: #  *  it under the terms of the GNU General Public License as published by
// []: #  *  the Free Software Foundation; either version 3 of the License, or
// []: #  *  (at your option) any later version.
// []: #  *
// []: #  *  Crankshaft is distributed in the hope that it will be useful,
// []: #  *  but WITHOUT ANY WARRANTY; without even the implied warranty of
// []: #  *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// []: #  *  GNU General Public License for more details.
// []: #  *
// []: #  *  You should have received a copy of the GNU General Public License
// []: #  *  along with Crankshaft. If not, see <http://www.gnu.org/licenses/>.
// []: # ```

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import CrankshaftReborn.UI 1.0

Item {
  id: overlay
  anchors.fill: parent
  visible: false
  z: 9999

  // Mappings (strings) provided by parent
  property string keyOpenSettings: "S"
  property string keyToggleTheme: "T"
  property string keyGoHome: "H"
  property string keyCycleLeft: "A"
  property string keyCycleRight: "D"
  property string keyShowHelp: "?"

  Keys.onPressed: {
    if (event.key === Qt.Key_Escape) {
      overlay.visible = false;
      event.accepted = true;
    }
  }

  // Backdrop
  Rectangle {
    anchors.fill: parent
    color: Theme.isDark ? "#80000000" : "#80000000"
    visible: overlay.visible

    MouseArea {
      anchors.fill: parent
      onClicked: overlay.visible = false
    }
  }

  // Panel
  Rectangle {
    id: panel
    width: Math.min(parent.width * 0.6, 640)
    height: implicitHeight
    radius: Theme.cornerRadius
    color: Theme.surface
    border.color: Theme.border
    border.width: 1
    anchors.horizontalCenter: parent.horizontalCenter
    anchors.verticalCenter: parent.verticalCenter
    visible: overlay.visible

    ColumnLayout {
      anchors.fill: parent
      anchors.margins: Theme.padding * 2
      spacing: Theme.spacing * 1.5

      RowLayout {
        Layout.fillWidth: true
        spacing: Theme.spacing

        Text {
          text: "⌨ Keyboard Shortcuts"
          font.pixelSize: 20
          font.bold: true
          color: Theme.text
          Layout.fillWidth: true
        }

        Button {
          text: "✖"
          flat: true
          onClicked: overlay.visible = false
          ToolTip.visible: hovered
          ToolTip.text: "Close"
        }
      }

      Rectangle {
        height: 1
        color: Theme.border
        Layout.fillWidth: true
      }

      GridLayout {
        id: grid
        columns: 2
        columnSpacing: Theme.spacing * 2
        rowSpacing: Theme.spacing
        Layout.fillWidth: true

        // Helper to render a row
        function row(label, keyText) {
          return [label, keyText];
        }

        Repeater {
          model: [
            grid.row("Open settings", overlay.keyOpenSettings),
            grid.row("Toggle theme", overlay.keyToggleTheme),
            grid.row("Go to Home", overlay.keyGoHome),
            grid.row("Cycle tabs left", overlay.keyCycleLeft),
            grid.row("Cycle tabs right", overlay.keyCycleRight),
            grid.row("Show this help", overlay.keyShowHelp)
          ]

          RowLayout {
            required property var modelData
            Layout.fillWidth: true

            Text {
              text: modelData[0]
              color: Theme.textSecondary
              font.pixelSize: 14
              Layout.fillWidth: true
            }

            Rectangle {
              radius: 6
              color: Theme.isDark ? "#222" : "#eee"
              border.color: Theme.border
              border.width: 1
              height: 28
              width: Math.max(48, keyText.implicitWidth + 16)

              Text {
                id: keyText
                anchors.centerIn: parent
                text: modelData[1]
                font.pixelSize: 14
                color: Theme.text
              }
            }
          }
        }
      }

      Text {
        text: "Press Esc or click outside to close"
        color: Theme.textSecondary
        font.pixelSize: 12
        Layout.alignment: Qt.AlignHCenter
      }
    }
  }
}
