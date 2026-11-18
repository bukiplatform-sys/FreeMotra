import QtQuick
import QtQuick.Controls
Button {
    id: control
    property bool primary: true
    property color ink: "#dce9ff"
    implicitHeight: 48
    implicitWidth: 160
    hoverEnabled: true
    font.family: "Segoe UI"
    font.pixelSize: 14
    font.weight: Font.DemiBold
    padding: 16
    contentItem: Text { textFormat: Text.PlainText;
        text: control.text
        font: control.font
        color: control.primary ? "white" : control.ink
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    background: Rectangle {
        radius: 10
        color: control.primary ? (control.down ? "#175bd9" : control.hovered ? "#438dff" : "#2878f3") : (control.hovered ? "#294569" : "#192c49")
        border.color: control.activeFocus ? "#91c2ff" : (control.primary ? "#5095ff" : "#31486a")
        border.width: control.activeFocus ? 2 : 1
        Behavior on color { ColorAnimation { duration: 120 } }
        Rectangle { height: 1; anchors.top: parent.top; anchors.topMargin: 1; anchors.left: parent.left; anchors.right: parent.right; anchors.margins: 10; color: "#35ffffff" }
    }
}
