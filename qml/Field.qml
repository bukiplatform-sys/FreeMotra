import QtQuick
import QtQuick.Controls
TextField {
    id: input
    property color ink: "#e9f0ff"
    property color surface: "#0a1427"
    implicitHeight: 54
    leftPadding: 16
    rightPadding: 16
    color: ink
    selectionColor: "#2878f3"
    placeholderTextColor: "#778aa8"
    font.family: "Segoe UI"
    font.pixelSize: 15
    selectByMouse: true
    maximumLength: 128
    background: Rectangle {
        radius: 10; color: input.surface
        border.color: input.activeFocus ? "#448dff" : "#2b3d5b"
        border.width: input.activeFocus ? 2 : 1
    }
}
