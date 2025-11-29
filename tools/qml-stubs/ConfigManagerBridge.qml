import QtQuick 2.15

QtObject {
    // Global config bridge stub used by linting when ConfigManagerBridge is referenced as a global
    function getValue(key, defaultValue) { return defaultValue; }
    function setValue(key, value) { }
    function getConfigPage(name) { return null; }
}
