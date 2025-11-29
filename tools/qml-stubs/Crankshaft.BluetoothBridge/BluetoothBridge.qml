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
