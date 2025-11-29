import QtQuick 2.15
import QtQuick.Controls 2.15

Button {
    // Minimal stub for linting. Behaviour provided at runtime.
}
import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
  id: wrapper
  property string text: ""
  width: 1; height: 1
  Button {
    id: control
    anchors.fill: parent
    text: wrapper.text
    Accessible.name: control.text ? control.text : ""
  }
}
