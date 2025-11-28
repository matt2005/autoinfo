import QtQuick 2.12
import QtQuick.Controls 2.12

Item {
  id: wrapper
  property string text: ""
  Button {
    id: btn
    anchors.fill: parent
    text: wrapper.text
    Accessible.name: btn.text ? btn.text : ""
  }
}
