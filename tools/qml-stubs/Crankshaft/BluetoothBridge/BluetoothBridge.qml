import QtQuick 2.15

QtObject {
    property var devices: [];
    property int discoveryTimeoutMs: 5000;
    property bool scanning: false;
    property bool callIncoming: false;
    property bool callActive: false;

    function scan(timeout) { }
    function pair(device) { }
    function connectDevice(device) { }
    function disconnectDevice(device) { }
    function answerCall() { }
    function rejectCall() { }
    function endCall() { }
    function dial(number) { }
}
import QtQuick 2.15

QtObject {
    // Signals and minimal methods used by UI - runtime provides real implementation
    signal devicesUpdated(var devices, bool scanning);
    signal scanStarted(int timeoutMs);
    signal pairedChanged(string address, bool paired);
    signal connectedChanged(string address, bool connected);
    signal disconnected(string address);
    signal callStatus(bool has, string number, string contact, bool incoming, bool active);

    function scan(timeoutMs) { }
    function pair(address) { }
    function connectDevice(address) { }
    function disconnectDevice(address) { }
    function answerCall() { }
    function rejectCall() { }
    function endCall() { }
    function dial(number) { }
}
