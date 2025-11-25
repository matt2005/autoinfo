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
// BluetoothView.qml relocated into bluetooth extension assets.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    width: 800; height: 600

    property var adapters: []
    property string currentAdapter: ""
    property int discoveryTimeoutMs: 10000
    property bool scanning: false
    property var devices: []

    property bool hasActiveCall: false
    property string callNumber: ""
    property string callContact: ""
    property bool callIncoming: false
    property bool callActive: false

    Connections {
        target: BluetoothBridge
        function onDevicesUpdated(devList, scanFlag) { devices = devList; scanning = scanFlag }
        function onScanStarted(timeoutMs) { scanning = true }
        function onPaired(address, paired) {}
        function onConnected(address, connected) {}
        function onDisconnected(address) {}
        function onCallStatus(has, number, contact, incoming, active) {
            hasActiveCall = has; callNumber = number; callContact = contact; callIncoming = incoming; callActive = active;
        }
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 12
        padding: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Label { text: "Bluetooth"; font.pixelSize: 24; Layout.alignment: Qt.AlignVCenter }
            ComboBox {
                id: adapterCombo
                Layout.preferredWidth: 200
                model: adapters
                currentIndex: Math.max(0, adapters.indexOf(currentAdapter))
                onActivated: root.currentAdapter = adapters[currentIndex]
            }
            SpinBox {
                id: timeoutBox
                from: 2; to: 60; value: discoveryTimeoutMs/1000
                suffix: " s"
                onValueChanged: discoveryTimeoutMs = value * 1000
            }
            Button {
                text: scanning ? "Scanning..." : "Scan"
                enabled: !scanning
                onClicked: BluetoothBridge.scan(discoveryTimeoutMs)
            }
        }

        Frame {
            Layout.fillWidth: true
            Layout.fillHeight: true
            ColumnLayout {
                anchors.fill: parent
                spacing: 6
                ListView {
                    id: deviceList
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    model: devices
                    clip: true
                    delegate: Rectangle {
                        height: 56
                        width: parent.width
                        color: index % 2 ? Qt.rgba(0,0,0,0.04) : Qt.rgba(0,0,0,0.08)
                        property var d: modelData
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 12
                            Label { text: d.name || d.address; Layout.fillWidth: true }
                            Label { text: d.paired ? "Paired" : ""; color: d.paired ? "#2b8" : "#888" }
                            Label { text: d.connected ? "Connected" : ""; color: d.connected ? "#28a" : "#888" }
                            Label { text: d.rssi !== undefined ? d.rssi + " dBm" : "" }
                            Button {
                                text: d.paired ? (d.connected ? "Disconnect" : "Connect") : "Pair"
                                onClicked: {
                                    if (!d.paired) BluetoothBridge.pair(d.address)
                                    else if (!d.connected) BluetoothBridge.connectDevice(d.address)
                                    else BluetoothBridge.disconnectDevice(d.address)
                                }
                            }
                        }
                    }
                    placeholderText: devices.length === 0 ? (scanning ? "Scanning for devices..." : "No devices found") : ""
                }
            }
        }

        Frame {
            Layout.fillWidth: true
            visible: hasActiveCall || callIncoming
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 12
                Label { text: callIncoming ? "Incoming call" : (callActive ? "Active call" : "Call") }
                Label { text: callContact !== "" ? callContact : callNumber }
                Item { Layout.fillWidth: true }
                Button { text: "Answer"; visible: callIncoming; onClicked: BluetoothBridge.answerCall() }
                Button { text: "Reject"; visible: callIncoming; onClicked: BluetoothBridge.rejectCall() }
                Button { text: "End"; visible: callActive; onClicked: BluetoothBridge.endCall() }
            }
        }

        Frame {
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8
                TextField { id: dialNumber; placeholderText: "Dial number"; Layout.preferredWidth: 200 }
                Button { text: "Dial"; onClicked: if (dialNumber.text.length>0) BluetoothBridge.dial(dialNumber.text) }
            }
        }
    }
}
