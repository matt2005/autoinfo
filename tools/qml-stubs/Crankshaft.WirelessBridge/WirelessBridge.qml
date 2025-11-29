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
