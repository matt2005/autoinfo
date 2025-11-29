import QtQuick 2.15

QtObject {
    // Event bridge stub
    signal eventOccurred(string name, var payload);
    function emitEvent(name, payload) { }
}
