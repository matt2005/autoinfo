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
// BluetoothView.qml
// High-level UI for managing Bluetooth adapters and devices.
// Integrates with BluetoothExtension via Event capability events:
//  Emitted by extension: devices_updated, scan_started, paired, connected, disconnected, call_status
//  Commands to emit (UI -> extension): scan, pair, connect, disconnect, dial, answerCall, rejectCall, endCall
// NOTE: Actual event bridge to C++ capability is not yet implemented; placeholder
// functions emit() / subscribe() must be wired by the core UI layer.

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    width: 800; height: 600

    // Adapter & discovery state
    property var adapters: []            // Filled externally
    property string currentAdapter: ""   // Active adapter
    property int discoveryTimeoutMs: 10000
    property bool scanning: false

    // Device list model (array of {address,name,paired,connected,rssi})
    property var devices: []

    // Call state
    property bool hasActiveCall: false
    property string callNumber: ""
    property string callContact: ""
    property bool callIncoming: false
    property bool callActive: false

    // Wiring to BluetoothBridge singleton
    Connections {
        target: root.BluetoothBridge
        function onDevicesUpdated(devList, scanFlag) { root.devices = devList; root.scanning = scanFlag }
        function onScanStarted(timeoutMs) { root.scanning = true }
        function onPaired(address, paired) { /* feedback hook */ }
        function onConnected(address, connected) { /* highlight hook */ }
        function onDisconnected(address) { /* disconnect hook */ }
        function onCallStatus(has, number, contact, incoming, active) {
            root.hasActiveCall = has; root.callNumber = number; root.callContact = contact; root.callIncoming = incoming; root.callActive = active;
        }
    }

    // Layout
    ColumnLayout {
        anchors.fill: parent
        spacing: 12
        padding: 12

        // Header / Adapter selection
        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            Label { text: "Bluetooth"; font.pixelSize: 24; Layout.alignment: Qt.AlignVCenter }
            ComboBox {
                id: adapterCombo
                Layout.preferredWidth: 200
                model: root.adapters
                currentIndex: Math.max(0, root.adapters.indexOf(root.currentAdapter))
                onActivated: {
                    root.currentAdapter = root.adapters[currentIndex]
                    // Adapter selection currently not implemented in bridge
                }
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
                Accessible.name: "Scan"
                onClicked: root.BluetoothBridge.scan(root.discoveryTimeoutMs)
            }
        }

        // Device list
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
                        id: rowRect
                        height: 56
                        Layout.fillWidth: true
                        color: index % 2 ? Qt.rgba(0,0,0,0.04) : Qt.rgba(0,0,0,0.08)
                        property var d: modelData;
                        RowLayout {
                            anchors.fill: parent
                            anchors.margins: 8
                            spacing: 12
                            Label { text: rowRect.d.name || rowRect.d.address; Layout.fillWidth: true }
                            Label { text: rowRect.d.paired ? "Paired" : ""; color: rowRect.d.paired ? "#2b8" : "#888" }
                            Label { text: rowRect.d.connected ? "Connected" : ""; color: rowRect.d.connected ? "#28a" : "#888" }
                            Label { text: rowRect.d.rssi !== undefined ? rowRect.d.rssi + " dBm" : "" }
                            Button {
                                text: rowRect.d.paired ? (rowRect.d.connected ? "Disconnect" : "Connect") : "Pair"
                                Accessible.name: rowRect.d.paired ? (rowRect.d.connected ? "Disconnect device" : "Connect device") : "Pair device"
                                onClicked: {
                                    if (!rowRect.d.paired) root.BluetoothBridge.pair(rowRect.d.address)
                                    else if (!rowRect.d.connected) root.BluetoothBridge.connectDevice(rowRect.d.address)
                                    else root.BluetoothBridge.disconnectDevice(rowRect.d.address)
                                }
                            }
                        }
                    }
                    placeholderText: root.devices.length === 0 ? (root.scanning ? "Scanning for devices..." : "No devices found") : ""
                }
            }
        }

        // Call controls
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

        // Dial pad (simple)
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
