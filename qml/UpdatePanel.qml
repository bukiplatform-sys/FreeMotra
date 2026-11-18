import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Panel {
    id: panel
    property bool english: false
    property color ink: "white"
    property color muted: "#93a4c0"
    function t(tr, en) { return english ? en : tr }
    implicitHeight: content.implicitHeight + 32
    ColumnLayout {
        id: content
        anchors.fill: parent; anchors.margins: 16; spacing: 10
        Text {
            Layout.fillWidth: true; textFormat: Text.PlainText; color: panel.ink
            font.pixelSize: 15; font.bold: true; wrapMode: Text.WordWrap
            text: Updates.available ? panel.t("Yeni sürüm mevcut: ", "New version available: ") + Updates.latestVersion : "FreeMotra " + Updates.currentVersion
        }
        Text {
            Layout.fillWidth: true; textFormat: Text.PlainText; color: panel.muted; wrapMode: Text.WordWrap
            text: {
                switch (Updates.state) {
                case "checking": return panel.t("Güncellemeler kontrol ediliyor…", "Checking for updates…")
                case "downloading": return panel.t("İndiriliyor: %", "Downloading: ") + Math.round(Updates.progress * 100) + (panel.english ? "%" : "")
                case "ready": return panel.t("Dosya doğrulandı. Kurulum uygulamayı kapatacak.", "File verified. Installation will close the app.")
                case "current": return panel.t("En güncel sürümü kullanıyorsunuz.", "You are up to date.")
                case "error": return panel.t("Güncelleme tamamlanamadı. Bağlantınızı kontrol edip yeniden deneyin.", "Update failed. Check your connection and retry.")
                default: return panel.t("Yeni sürümler otomatik kontrol edilir. Kurulum sizin onayınızla başlar.", "New versions are checked automatically. Installation requires your approval.")
                }
            }
        }
        Text {
            Layout.fillWidth: true; visible: Updates.available && Updates.notes.length > 0
            textFormat: Text.PlainText; color: panel.muted; wrapMode: Text.WordWrap
            text: Updates.notes; maximumLineCount: 5; elide: Text.ElideRight
        }
        ProgressBar { Layout.fillWidth: true; visible: Updates.state === "downloading"; value: Updates.progress }
        Text {
            visible: Updates.blocked && Updates.state === "ready"; Layout.fillWidth: true
            textFormat: Text.PlainText; color: panel.muted; wrapMode: Text.WordWrap
            text: panel.t("Kurulum için önce bağlantıyı ve bekleyen oturum isteğini kapatın.", "End the connection and pending session request before installing.")
        }
        Flow {
            Layout.fillWidth: true; spacing: 8
            ActionButton {
                visible: Updates.available && Updates.state !== "ready" && Updates.state !== "downloading"
                text: panel.t("İndir ve güncelle", "Download update")
                onClicked: Updates.download()
            }
            ActionButton {
                visible: Updates.state === "ready"; enabled: !Updates.blocked
                text: panel.t("Güncellemeyi kur", "Install update"); onClicked: confirmInstall.open()
            }
            ActionButton {
                visible: Updates.state === "downloading"; primary: false
                text: panel.t("İptal", "Cancel"); onClicked: Updates.cancel()
            }
            ActionButton {
                visible: Updates.state !== "downloading" && Updates.state !== "ready"
                enabled: Updates.state !== "checking"; primary: false
                text: panel.t("Güncellemeleri denetle", "Check for updates"); implicitWidth: 215
                onClicked: Updates.check()
            }
        }
    }
    Dialog {
        id: confirmInstall
        objectName: "updateInstallDialog"
        parent: Overlay.overlay; anchors.centerIn: parent
        width: Math.min(460, parent.width - 40); implicitHeight: 240; modal: true
        title: panel.t("Güncelleme kurulumu", "Install update")
        standardButtons: Dialog.Ok | Dialog.Cancel
        contentItem: Text {
            textFormat: Text.PlainText; wrapMode: Text.WordWrap
            text: panel.t("FreeMotra kapanacak ve Windows kurulum penceresi açılacak. Devam edilsin mi?", "FreeMotra will close and the Windows installer will open. Continue?")
        }
        onAccepted: Updates.install()
    }
}
