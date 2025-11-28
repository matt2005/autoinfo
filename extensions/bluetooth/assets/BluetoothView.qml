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
        target: root.BluetoothBridge
        function onDevicesUpdated(devList, scanFlag) { root.devices = devList; root.scanning = scanFlag }
        function onScanStarted(timeoutMs) { root.scanning = true }
        function onPaired(address, paired) {}
        function onConnected(address, connected) {}
        function onDisconnected(address) {}
        function onCallStatus(has, number, contact, incoming, active) {
            root.hasActiveCall = has; root.callNumber = number; root.callContact = contact; root.callIncoming = incoming; root.callActive = active;
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
                model: root.adapters
                currentIndex: Math.max(0, root.adapters.indexOf(root.currentAdapter))
                onActivated: root.currentAdapter = root.adapters[currentIndex]
            }
            SpinBox {
                id: timeoutBox
                from: 2; to: 60; value: root.discoveryTimeoutMs/1000
                suffix: " s"
                onValueChanged: root.discoveryTimeoutMs = value * 1000
            }
            Button {
                text: root.scanning ? "Scanning..." : "Scan"
                enabled: !root.scanning
                onClicked: root.BluetoothBridge.scan(root.discoveryTimeoutMs)
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
                    model: root.devices
                    clip: true
                    delegate: Rectangle {
                        id: deviceRow
                        height: 56
                        width: parent.width
                        color: index % 2 ? Qt.rgba(0,0,0,0.04) : Qt.rgba(0,0,0,0.08)
                        property var d: modelData;
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 12
                            Label { text: deviceRow.d.name || deviceRow.d.address; Layout.fillWidth: true }
                            Label { text: deviceRow.d.paired ? "Paired" : ""; color: deviceRow.d.paired ? "#2b8" : "#888" }
                            Label { text: deviceRow.d.connected ? "Connected" : ""; color: deviceRow.d.connected ? "#28a" : "#888" }
                            Label { text: deviceRow.d.rssi !== undefined ? deviceRow.d.rssi + " dBm" : "" }
                            Button {
                                text: deviceRow.d.paired ? (deviceRow.d.connected ? "Disconnect" : "Connect") : "Pair"
                                Accessible.name: deviceRow.d.paired ? (deviceRow.d.connected ? "Disconnect" : "Connect") : "Pair"
                                onClicked: {
                                    if (!deviceRow.d.paired) root.BluetoothBridge.pair(deviceRow.d.address);
                                    else if (!deviceRow.d.connected) root.BluetoothBridge.connectDevice(deviceRow.d.address);
                                    else root.BluetoothBridge.disconnectDevice(deviceRow.d.address);
                                }
                            }
                        }
                    }
                    property string devicesPlaceholder: root.devices.length === 0 ? (root.scanning ? "Scanning for devices..." : "No devices found") : ""
                }
            }
        }

        Frame {
            Layout.fillWidth: true
            visible: root.hasActiveCall || root.callIncoming
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 12
                Label { text: root.callIncoming ? "Incoming call" : (root.callActive ? "Active call" : "Call") }
                                 Label { text: root.callContact !== "" ? root.callContact : root.callNumber }
                Item { Layout.fillWidth: true }
                Button { text: "Answer"; Accessible.name: "Answer call"; visible: root.callIncoming; onClicked: root.BluetoothBridge.answerCall() }
                Button { text: "Reject"; Accessible.name: "Reject call"; visible: root.callIncoming; onClicked: root.BluetoothBridge.rejectCall() }
                Button { text: "End"; Accessible.name: "End call"; visible: root.callActive; onClicked: root.BluetoothBridge.endCall() }
            }
        }

        Frame {
            Layout.fillWidth: true
            RowLayout {
                anchors.fill: parent
                anchors.margins: 8
                spacing: 8
                TextField { id: dialNumber; placeholderText: "Dial number"; Layout.preferredWidth: 200 }
                Button { text: "Dial"; Accessible.name: "Dial number"; onClicked: if (dialNumber.text.length>0) root.BluetoothBridge.dial(dialNumber.text) }
            }
        }
    }
}
