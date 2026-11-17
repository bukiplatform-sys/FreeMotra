import QtQuick
Rectangle {
    property color surface: "#111d34"
    property color edge: "#243755"
    color: surface
    border.color: edge
    border.width: 1
    radius: 18
    Behavior on color { ColorAnimation { duration: 180 } }
}
