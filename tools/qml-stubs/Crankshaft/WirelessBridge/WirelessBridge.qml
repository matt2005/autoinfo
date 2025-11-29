import QtQuick 2.15

QtObject {
    property var networks: [];
    property bool scanning: false;

    function scan() { }
    function connect(ssid, password) { }
    function disconnect(ssid) { }
    function forget(ssid) { }
    function configureAP(cfg) { }
}
import QtQuick 2.15

QtObject {
    // Minimal wireless bridge stub for linting
    signal networksUpdated(var list);
    signal connectionStateChanged(string ssid, bool connected);

    function scan() { }
    function connect(ssid, password) { }
    function disconnect() { }
    function configureAP(ssid, password) { }
    function forget(ssid) { }
}
