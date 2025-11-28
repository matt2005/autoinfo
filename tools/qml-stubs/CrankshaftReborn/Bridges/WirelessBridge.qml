import QtQuick 2.12

QtObject {
  signal networksUpdated()
  function scan() { }
  function connect(ssid, password) { }
  function disconnect() { }
  function configureAP(ssid, password) { }
  function forget(ssid) { }
}
