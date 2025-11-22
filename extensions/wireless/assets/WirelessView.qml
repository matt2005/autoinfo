import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

Item {
    id: root
    
    property var networks: []
    property string currentSsid: ""
    property bool isConnected: false
    property bool showPasswordDialog: false
    property string selectedSsid: ""
    property bool showApDialog: false
    
    Component.onCompleted: {
        // Subscribe to events
        if (typeof WirelessBridge !== 'undefined') {
            WirelessBridge.networksUpdated.connect(onNetworksUpdated)
            WirelessBridge.connectionStateChanged.connect(onConnectionStateChanged)
        }
        
        // Request initial scan
        requestScan()
    }
    
    function requestScan() {
        if (typeof WirelessBridge !== 'undefined') {
            WirelessBridge.scan()
        }
    }
    
    function connectToNetwork(ssid, password) {
        if (typeof WirelessBridge !== 'undefined') {
            WirelessBridge.connect(ssid, password)
        }
    }
    
    function disconnect() {
        if (typeof WirelessBridge !== 'undefined') {
            WirelessBridge.disconnect()
        }
    }
    
    function configureAccessPoint(ssid, password) {
        if (typeof WirelessBridge !== 'undefined') {
            WirelessBridge.configureAP(ssid, password)
        }
    }
    
    function forgetNetwork(ssid) {
        if (typeof WirelessBridge !== 'undefined') {
            WirelessBridge.forget(ssid)
        }
    }
    
    function onNetworksUpdated(networkList) {
        root.networks = networkList
    }
    
    function onConnectionStateChanged(ssid, connected) {
        root.currentSsid = ssid
        root.isConnected = connected
    }
    
    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 15
        
        // Header
        RowLayout {
            Layout.fillWidth: true
            spacing: 10
            
            Label {
                text: "WiFi Networks"
                font.pixelSize: 24
                font.bold: true
                Layout.fillWidth: true
            }
            
            Button {
                text: "Scan"
                onClicked: requestScan()
            }
            
            Button {
                text: "Create Hotspot"
                onClicked: showApDialog = true
            }
            
            Button {
                text: isConnected ? "Disconnect" : "Disconnected"
                enabled: isConnected
                onClicked: disconnect()
            }
        }
        
        // Connection status
        Rectangle {
            Layout.fillWidth: true
            height: 60
            color: isConnected ? "#4CAF50" : "#757575"
            radius: 8
            visible: currentSsid !== ""
            
            RowLayout {
                anchors.fill: parent
                anchors.margins: 15
                
                Label {
                    text: "📶"
                    font.pixelSize: 24
                }
                
                Label {
                    text: isConnected ? "Connected to: " + currentSsid : "Disconnected"
                    font.pixelSize: 18
                    color: "white"
                    Layout.fillWidth: true
                }
                
                Button {
                    text: "×"
                    onClicked: disconnect()
                }
            }
        }
        
        // Network list
        ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            clip: true
            
            ListView {
                id: networkListView
                model: root.networks
                spacing: 8
                
                delegate: Rectangle {
                    width: networkListView.width
                    height: 80
                    color: modelData.isConnected ? "#E3F2FD" : "#FFFFFF"
                    border.color: "#CCCCCC"
                    border.width: 1
                    radius: 8
                    
                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 15
                        spacing: 15
                        
                        // Signal strength indicator
                        Label {
                            text: {
                                if (modelData.signalStrength > 75) return "📶"
                                if (modelData.signalStrength > 50) return "📶"
                                if (modelData.signalStrength > 25) return "📶"
                                return "📶"
                            }
                            font.pixelSize: 24
                            opacity: modelData.signalStrength / 100
                        }
                        
                        // Network info
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4
                            
                            Label {
                                text: modelData.ssid
                                font.pixelSize: 18
                                font.bold: modelData.isConnected
                            }
                            
                            RowLayout {
                                spacing: 10
                                
                                Label {
                                    text: modelData.securityType
                                    font.pixelSize: 12
                                    color: "#666666"
                                }
                                
                                Label {
                                    text: modelData.signalStrength + "%"
                                    font.pixelSize: 12
                                    color: "#666666"
                                }
                                
                                Label {
                                    text: (modelData.frequency / 1000).toFixed(1) + " GHz"
                                    font.pixelSize: 12
                                    color: "#666666"
                                }
                            }
                        }
                        
                        // Connect/Forget buttons
                        RowLayout {
                            spacing: 8
                            
                            Button {
                                text: modelData.isConnected ? "Connected" : "Connect"
                                enabled: !modelData.isConnected
                                onClicked: {
                                    if (modelData.isSecure) {
                                        selectedSsid = modelData.ssid
                                        showPasswordDialog = true
                                    } else {
                                        connectToNetwork(modelData.ssid, "")
                                    }
                                }
                            }
                            
                            Button {
                                text: "Forget"
                                visible: modelData.isConnected
                                onClicked: forgetNetwork(modelData.ssid)
                            }
                        }
                    }
                }
            }
        }
        
        // Empty state
        Label {
            text: "No networks found. Click Scan to search for networks."
            font.pixelSize: 16
            color: "#999999"
            horizontalAlignment: Text.AlignHCenter
            Layout.fillWidth: true
            visible: networks.length === 0
        }
    }
    
    // Password dialog
    Dialog {
        id: passwordDialog
        visible: showPasswordDialog
        modal: true
        title: "Connect to " + selectedSsid
        anchors.centerIn: parent
        width: 400
        
        ColumnLayout {
            spacing: 15
            width: parent.width
            
            Label {
                text: "Enter network password:"
                font.pixelSize: 14
            }
            
            TextField {
                id: passwordField
                Layout.fillWidth: true
                placeholderText: "Password"
                echoMode: TextInput.Password
                onAccepted: {
                    connectToNetwork(selectedSsid, passwordField.text)
                    showPasswordDialog = false
                    passwordField.text = ""
                }
            }
            
            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 10
                
                Button {
                    text: "Cancel"
                    onClicked: {
                        showPasswordDialog = false
                        passwordField.text = ""
                    }
                }
                
                Button {
                    text: "Connect"
                    onClicked: {
                        connectToNetwork(selectedSsid, passwordField.text)
                        showPasswordDialog = false
                        passwordField.text = ""
                    }
                }
            }
        }
    }
    
    // Access Point configuration dialog
    Dialog {
        id: apDialog
        visible: showApDialog
        modal: true
        title: "Configure WiFi Hotspot"
        anchors.centerIn: parent
        width: 400
        
        ColumnLayout {
            spacing: 15
            width: parent.width
            
            Label {
                text: "Hotspot SSID:"
                font.pixelSize: 14
            }
            
            TextField {
                id: apSsidField
                Layout.fillWidth: true
                placeholderText: "Crankshaft-Hotspot"
                text: "Crankshaft-Hotspot"
            }
            
            Label {
                text: "Password (min 8 characters):"
                font.pixelSize: 14
            }
            
            TextField {
                id: apPasswordField
                Layout.fillWidth: true
                placeholderText: "Password"
                echoMode: TextInput.Password
            }
            
            Label {
                text: "⚠️ This will disconnect from current network"
                font.pixelSize: 12
                color: "#FF9800"
                wrapMode: Text.Wrap
                Layout.fillWidth: true
            }
            
            RowLayout {
                Layout.alignment: Qt.AlignRight
                spacing: 10
                
                Button {
                    text: "Cancel"
                    onClicked: {
                        showApDialog = false
                        apPasswordField.text = ""
                    }
                }
                
                Button {
                    text: "Create Hotspot"
                    enabled: apSsidField.text !== "" && apPasswordField.text.length >= 8
                    onClicked: {
                        configureAccessPoint(apSsidField.text, apPasswordField.text)
                        showApDialog = false
                        apPasswordField.text = ""
                    }
                }
            }
        }
    }
}
