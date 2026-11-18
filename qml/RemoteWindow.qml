import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
Window {
    id: remoteWindow
    visible: App.sessionActive && !App.hosting
    width: 1400; height: 850
    minimumWidth: 1000; minimumHeight: 650
    title: "FreeMotra — " + (App.remoteControl ? "Uzaktan kontrol / Remote control" : (App.viewAllowed ? "Yalnızca görüntüleme / View only" : "Oturum araçları / Session tools"))
    color: "#080f20"
    onClosing: App.endSession()
    onActiveChanged: { if (!active) App.releaseRemoteKeys() }
    property var heldKeys: ({})
    function vk(key) {
        if ((key >= 32 && key <= 90) || (key >= 96 && key <= 126)) return key
        let map = {}; map[Qt.Key_Control]=17; map[Qt.Key_Shift]=16; map[Qt.Key_Alt]=18
        map[Qt.Key_Return]=13; map[Qt.Key_Enter]=13; map[Qt.Key_Backspace]=8; map[Qt.Key_Tab]=9
        map[Qt.Key_Left]=37; map[Qt.Key_Up]=38; map[Qt.Key_Right]=39; map[Qt.Key_Down]=40
        map[Qt.Key_Delete]=46; map[Qt.Key_Home]=36; map[Qt.Key_End]=35
        map[Qt.Key_PageUp]=33; map[Qt.Key_PageDown]=34; map[Qt.Key_Insert]=45
        if (key >= Qt.Key_F1 && key <= Qt.Key_F12) return 112 + key - Qt.Key_F1
        return map[key] || 0
    }
    Shortcut { sequence: "Escape"; enabled: remoteWindow.visible; context: Qt.WindowShortcut; onActivated: App.endSession() }
    Shortcut { sequence: "F11"; enabled: remoteWindow.visible; context: Qt.WindowShortcut; onActivated: remoteWindow.visibility = remoteWindow.visibility === Window.FullScreen ? Window.Windowed : Window.FullScreen }
    ColumnLayout {
        anchors.fill: parent; spacing: 0
        Rectangle {
            Layout.fillWidth: true; Layout.preferredHeight: 60; color: "#132d43"
            RowLayout {
                anchors.fill: parent; anchors.margins: 10
                Text { textFormat: Text.PlainText; Layout.fillWidth: true; color: "#b3f5d2"; text: "● WebRTC DTLS · " + (App.remoteControl ? "Kontrol izni açık / Control allowed" : (App.viewAllowed ? "Yalnızca görüntüleme / View only" : "Oturum araçları / Session tools")) }
                ComboBox {
                    Layout.preferredWidth: 190
                    visible: App.viewAllowed
                    model: App.monitors; textRole: "name"; currentIndex: App.selectedMonitor
                    onActivated: function(index) { App.selectMonitor(index) }
                }
                ActionButton { implicitWidth: 130; text: "Tam ekran / F11"; primary: false; onClicked: remoteWindow.visibility = remoteWindow.visibility === Window.FullScreen ? Window.Windowed : Window.FullScreen }
                ActionButton { implicitWidth: 160; text: "Bağlantıyı kes / Esc"; onClicked: App.endSession() }
            }
        }
        Label { Layout.fillWidth: true; padding: 6; color: "#aac6de"; textFormat: Text.PlainText; text: App.effectiveQuality; visible: App.viewAllowed }
        RowLayout {
          Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0
          Image {
            id: remoteImage
            Layout.fillWidth: true; Layout.fillHeight: true
            visible: App.viewAllowed
            source: App.frameSerial > 0 ? "image://remote/frame/" + App.frameSerial : ""
            fillMode: Image.PreserveAspectFit; cache: false; asynchronous: false
            DropArea { anchors.fill: parent; enabled: Tools.filesAllowed; onDropped: function(drop) { if(drop.hasUrls && drop.urls.length) { Tools.sendFile(drop.urls[0]); drop.acceptProposedAction() } } }
            Text { textFormat: Text.PlainText; visible: App.viewAllowed && remoteImage.status !== Image.Ready; anchors.centerIn: parent; color: "#c8dbff"; text: "İlk ekran görüntüsü bekleniyor / Waiting for screen…" }
            MouseArea {
                anchors.centerIn: parent; width: remoteImage.paintedWidth; height: remoteImage.paintedHeight
                hoverEnabled: true; acceptedButtons: Qt.LeftButton | Qt.RightButton
                enabled: App.remoteControl; focus: true
                onPressed: function(mouse) { forceActiveFocus(); App.remoteInput("move",mouse.x/width,mouse.y/height); App.remoteInput("button",mouse.button === Qt.RightButton ? 1 : 0,1) }
                onReleased: function(mouse) { App.remoteInput("button",mouse.button === Qt.RightButton ? 1 : 0,0) }
                onPositionChanged: function(mouse) { App.remoteInput("move",mouse.x/width,mouse.y/height) }
                onWheel: function(wheel) { App.remoteInput("wheel",Math.max(-1200,Math.min(1200,wheel.angleDelta.y))) }
                onCanceled: App.releaseRemoteKeys()
                onActiveFocusChanged: { if (!activeFocus) App.releaseRemoteKeys() }
                Keys.onPressed: function(event) {
                    if (event.key === Qt.Key_Escape) { App.endSession(); event.accepted=true; return }
                    if (event.key === Qt.Key_F11) { remoteWindow.visibility=remoteWindow.visibility === Window.FullScreen ? Window.Windowed : Window.FullScreen; event.accepted=true; return }
                    if(event.text.length && !(event.modifiers & (Qt.ControlModifier | Qt.AltModifier | Qt.MetaModifier)) && event.key >= Qt.Key_Space && event.key < Qt.Key_Escape) {
                        App.remoteText(event.text); event.accepted=true; return
                    }
                    let code=remoteWindow.vk(event.key); if(code) { remoteWindow.heldKeys[event.key]=code; App.remoteInput("key",code,1); event.accepted=true }
                }
                Keys.onReleased: function(event) { let code=remoteWindow.heldKeys[event.key]; if(code) { delete remoteWindow.heldKeys[event.key]; App.remoteInput("key",code,0); event.accepted=true } }
            }
          }
          SessionPanel { Layout.preferredWidth: 350; Layout.fillHeight: true; Layout.fillWidth: !App.viewAllowed }
        }
    }
}
