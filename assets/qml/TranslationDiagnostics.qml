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
import CrankshaftReborn.I18n 1.0
import CrankshaftReborn.UI 1.0

Item {
    id: root
    property var status: I18nManager.extensionTranslationStatus();

    Connections {
        target: I18nManager
        function onExtensionTranslationsChanged() {
            status = I18nManager.extensionTranslationStatus();
        }
        function onLanguageChanged() {
            status = I18nManager.extensionTranslationStatus();
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 8

        Text {
            text: qsTr("Current Locale: ") + I18nManager.currentLocale
            font.pixelSize: 16
            color: Theme.text
        }

        TableView {
            id: table
            Layout.fillWidth: true
            Layout.fillHeight: true
            columnSpacing: 4
            rowSpacing: 2
            model: status
            clip: true

            delegate: RowLayout {
                width: table.width
                spacing: 12
                Text { text: modelData.id; color: Theme.text }
                Text { text: modelData.loadedLocale; color: Theme.textSecondary }
                Text { text: modelData.fallbackUsed ? qsTr("(fallback)") : ""; color: Theme.accent }
            }

            ScrollBar.vertical: ScrollBar { }
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 6
            Text { text: qsTr("Reload:"); color: Theme.text }
            StyledButton {
                text: qsTr("Refresh Translations")
                Accessible.name: qsTr("Refresh Translations")
                onClicked: I18nManager.refreshTranslations();
            }
        }
    }
}
