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
