import QtQuick 2.15

Item {
    // Lint-only stub for StyledButton used by UI files
    property string text: ""
    property bool enabled: true
    signal clicked()
    function click() { clicked(); }
}
import QtQuick 2.15

QtObject {
    // Minimal lint stub for a styled button-like control
    property string text: ""
    property bool pressed: false
    property bool enabled: true
    function click() {}
}
