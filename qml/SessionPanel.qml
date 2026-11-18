import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import QtQuick.Dialogs

Rectangle {
    id: panel
    color: "#101d33"
    implicitWidth: 340
    implicitHeight: 580
    FileDialog { id: sendDialog; title: "Gönderilecek dosyayı seç / Select a file to send"; onAccepted: Tools.sendFile(selectedFile) }
    FolderDialog { id: receiveDialog; title: "Dosyanın kaydedileceği klasör / Destination folder"; onAccepted: Tools.acceptFile(selectedFolder) }
    ScrollView {
        anchors.fill: parent; anchors.margins: 14
        contentWidth: availableWidth
        ColumnLayout {
            width: parent.width; spacing: 12
            Label { text: "Oturum araçları / Session tools"; color: "#e1ebff"; font.bold: true }
            Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; color: "#9db4d6"; text: "Dosya, sohbet ve ses izinlerini bağlantıyı kabul eden kişi belirler. / Permissions are set by the host." }
            Label { text: "Dosya aktarımı / File transfer"; color: "#e1ebff" }
            Rectangle {
                Layout.fillWidth: true; implicitHeight: 80; radius: 10; color: "#19314b"; border.color: "#3f7fa1"
                Label { anchors.fill: parent; anchors.margins: 10; text: Tools.filesAllowed ? "Buraya bir dosya bırak\nDrop one file here" : "Dosya aktarım izni kapalı / Files disabled"; color: "#cceaff"; wrapMode: Text.WordWrap; horizontalAlignment: Text.AlignHCenter; verticalAlignment: Text.AlignVCenter }
                DropArea { anchors.fill: parent; enabled: Tools.filesAllowed; onDropped: function(drop) { if(drop.hasUrls && drop.urls.length) { Tools.sendFile(drop.urls[0]); drop.acceptProposedAction() } } }
            }
            Button { Layout.fillWidth: true; text: "Dosya seç / Choose file"; enabled: Tools.filesAllowed; onClicked: sendDialog.open() }
            Label { visible: Tools.pendingFile.length > 0; Layout.fillWidth: true; textFormat: Text.PlainText; text: "Gelen / Incoming: " + Tools.pendingFile; color: "#d1ebff"; wrapMode: Text.WrapAnywhere }
            RowLayout {
                visible: Tools.pendingFile.length > 0
                Button { text: "Kabul et / Accept"; onClicked: receiveDialog.open() }
                Button { text: "Reddet / Reject"; onClicked: Tools.rejectFile() }
            }
            ProgressBar { Layout.fillWidth: true; value: Tools.progress }
            Label { Layout.fillWidth: true; textFormat: Text.PlainText; text: Tools.transferStatus; wrapMode: Text.WordWrap; color: "#aec5e8" }
            Button { text: "Aktarımı durdur / Cancel transfer"; enabled: Tools.filesAllowed; onClicked: Tools.cancelTransfer() }
            Label { Layout.fillWidth: true; text: "Devam etmek için aynı dosyayı yeniden gönderin ve aynı hedef klasörde kabul edin. / To resume, resend the same file and accept in the same folder."; color: "#9db4d6"; wrapMode: Text.WordWrap; font.pixelSize: 11 }
            Label { text: "Sesli görüşme / Voice call"; color: "#e1ebff"; font.bold: true }
            Switch { text: "Mikrofon / Microphone"; enabled: Tools.voiceAllowed; checked: Tools.microphone; palette.windowText: "#e1ebff"; onToggled: Tools.setMicrophone(checked) }
            Switch { text: "Karşı tarafı dinle / Listen to peer"; enabled: Tools.voiceAllowed; checked: Tools.listening; palette.windowText: "#e1ebff"; onToggled: Tools.setListening(checked) }
            Label { Layout.fillWidth: true; text: Tools.audioStatus; wrapMode: Text.WordWrap; color: "#aec5e8" }
            Label { text: "Yazılı sohbet / Text chat"; color: "#e1ebff"; font.bold: true }
            ListView {
                id: chatList
                Layout.fillWidth: true; Layout.preferredHeight: 210
                clip: true; spacing: 8; model: Tools.messages
                onCountChanged: positionViewAtEnd()
                delegate: Label {
                    required property var modelData
                    width: chatList.width; padding: 8
                    textFormat: Text.PlainText; wrapMode: Text.WrapAnywhere
                    color: modelData.local ? "#8be6c1" : "#d1e2ff"
                    text: modelData.time + (modelData.local ? " · Siz / You\n" : " · Karşı taraf / Peer\n") + modelData.text
                    background: Rectangle { color: "#182c45"; radius: 8 }
                }
            }
            TextArea { id: chatEntry; Layout.fillWidth: true; placeholderText: "Mesaj / Message"; enabled: Tools.chatAllowed; wrapMode: TextEdit.Wrap; selectByMouse: true; color: "#e1ebff"; background: Rectangle { color: "#19314b"; radius: 8 } onTextChanged: { if(text.length>2000)text=text.slice(0,2000) } }
            Button { Layout.fillWidth: true; text: "Gönder / Send"; enabled: Tools.chatAllowed && chatEntry.text.trim().length > 0; onClicked: { Tools.sendChat(chatEntry.text); chatEntry.clear() } }
        }
    }
}
