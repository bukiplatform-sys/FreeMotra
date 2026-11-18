import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ApplicationWindow {
    id: win
    width: 1460; height: 940
    minimumWidth: 980; minimumHeight: 700
    visible: true
    title: "FreeMotra"
    color: bg
    font.family: "Segoe UI"
    property bool english: App.language === "en"
    property color bg: App.dark ? "#080f20" : "#eef3fb"
    property color surface: App.dark ? "#101c32" : "#ffffff"
    property color edge: App.dark ? "#243551" : "#d3dfef"
    property color ink: App.dark ? "#edf3ff" : "#172944"
    property color muted: App.dark ? "#93a4c0" : "#59708e"
    property color blue: App.dark ? "#76aaff" : "#216be0"
    property int page: 0
    function t(tr, en) { return english ? en : tr }

    Rectangle {
        anchors.top: parent.top; anchors.right: parent.right
        width: parent.width * 0.75; height: parent.height * 0.7
        opacity: App.dark ? 0.65 : 0.15
        gradient: Gradient { GradientStop { position: 0; color: "#14294d" } GradientStop { position: 1; color: "#00080f20" } }
    }

    RowLayout {
        anchors.fill: parent; spacing: 0
        Rectangle {
            Layout.fillHeight: true
            Layout.preferredWidth: win.width < 1180 ? 205 : 240
            color: App.dark ? "#0c1528" : "#e5edf8"
            Rectangle { width: 1; anchors.right: parent.right; height: parent.height; color: edge }
            ColumnLayout {
                anchors.fill: parent; anchors.margins: 22; spacing: 10
                RowLayout {
                    Layout.topMargin: 20; spacing: 10
                    Rectangle {
                        width: 36; height: 38; radius: 10; color: "#2878f3"
                        Glyph { anchors.centerIn: parent; name: "connect"; ink: "white"; width: 26; height: 26 }
                    }
                    Text { text: "FreeMotra"; color: ink; font.pixelSize: 23; font.weight: Font.Bold; font.letterSpacing: -0.9 }
                }
                Rectangle {
                    Layout.topMargin: 9; Layout.leftMargin: 46
                    implicitWidth: freeLabel.implicitWidth + 18; height: 23; radius: 6
                    color: "#163c39"; border.color: "#235a4d"
                    Text { id: freeLabel; anchors.centerIn: parent; text: t("HERKES İÇİN ÜCRETSİZ", "FREE FOR EVERYONE"); color: "#78ddbb"; font.pixelSize: 8; font.weight: Font.Bold; font.letterSpacing: 0.7 }
                }
                Text { Layout.topMargin: 42; Layout.leftMargin: 12; text: t("ÇALIŞMA ALANI", "WORKSPACE"); font.pixelSize: 10; font.letterSpacing: 1.5; font.weight: Font.Bold; color: muted }
                Repeater {
                    model: [ ["connect",t("Uzaktan Erişim","Remote Access")], ["history",t("Bağlantılarım","My Connections")], ["settings",t("Ayarlar","Settings")], ["info",t("Hakkında","About")] ]
                    delegate: ItemDelegate {
                        id: nav
                        required property int index
                        required property var modelData
                        Layout.fillWidth: true; implicitHeight: 49
                        onClicked: win.page = index
                        background: Rectangle { radius: 10; color: win.page === nav.index ? (App.dark ? "#19355c" : "#d1e3ff") : nav.hovered ? (App.dark ? "#15243b" : "#dbe6f6") : "transparent"; border.color: win.page === nav.index ? "#2e5485" : "transparent" }
                        contentItem: RowLayout {
                            spacing: 13
                            Glyph { Layout.leftMargin: 8; name: nav.modelData[0]; ink: win.page === nav.index ? blue : muted; width: 20; height: 20 }
                            Text { Layout.fillWidth: true; text: nav.modelData[1]; color: win.page === nav.index ? ink : muted; font.pixelSize: 13; font.weight: win.page === nav.index ? Font.DemiBold : Font.Normal }
                        }
                    }
                }
                Item { Layout.fillHeight: true }
                Panel {
                    Layout.fillWidth: true; Layout.preferredHeight: 150
                    surface: App.dark ? "#101f38" : "#f5f9ff"; edge: win.edge
                    ColumnLayout {
                        anchors.fill: parent; anchors.margins: 17; spacing: 8
                        Glyph { name: "heart"; ink: blue; width: 22; height: 22 }
                        Text { Layout.fillWidth: true; text: t("Uzaklar yakın olsun.","Distance, reimagined."); color: ink; font.pixelSize: 14; font.weight: Font.DemiBold; wrapMode: Text.WordWrap }
                        Text { Layout.fillWidth: true; text: t("Herkes için ücretsiz\nuzaktan erişim.","Free remote access.\nFor everyone."); color: muted; font.pixelSize: 12; lineHeight: 1.4 }
                    }
                }
                Text { Layout.topMargin: 12; text: "FreeMotra  /  v0.1.0"; color: muted; font.pixelSize: 10 }
            }
        }
        ColumnLayout {
            Layout.fillWidth: true; Layout.fillHeight: true; spacing: 0
            Rectangle {
                Layout.fillWidth: true; Layout.preferredHeight: 76
                color: "transparent"
                RowLayout {
                    anchors.fill: parent; anchors.leftMargin: 32; anchors.rightMargin: 30
                    Text { text: [t("Uzaktan Erişim","Remote Access"),t("Bağlantılarım","My Connections"),t("Ayarlar","Settings"),t("Hakkında","About")][win.page]; color: muted; font.pixelSize: 13 }
                    Item { Layout.fillWidth: true }
                    ActionButton { visible: App.sessionActive; implicitWidth: 140; text: t("Oturumu bitir", "End session"); onClicked: App.endSession() }
                    Rectangle { width: 6; height: 6; radius: 3; color: "#edbb64" }
                    Text { text: (App.sessionStatus.length ? App.sessionStatus : t("v0.2 · Yerel ağ / LAN", "v0.2 · Local network / LAN")); color: muted; font.pixelSize: 11 }
                    ActionButton { Layout.leftMargin: 16; implicitWidth: 65; implicitHeight: 32; primary: false; text: english ? "TR" : "EN"; onClicked: App.language = english ? "tr" : "en" }
                }
                Rectangle { anchors.bottom: parent.bottom; height: 1; width: parent.width; color: edge }
            }
            ScrollView {
                id: scroller
                Layout.fillWidth: true; Layout.fillHeight: true
                contentWidth: availableWidth; clip: true
                ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
                ColumnLayout {
                    width: scroller.availableWidth
                    spacing: 24
                    Item { Layout.preferredHeight: 8 }
                    ColumnLayout {
                        Layout.fillWidth: true; Layout.leftMargin: 32; Layout.rightMargin: 32; spacing: 9
                        Text { text: [t("FreeMotra’ya Hoş Geldiniz", "Welcome to FreeMotra"),t("Bağlantı geçmişiniz", "Your connection history"),t("Size göre FreeMotra", "Make FreeMotra yours"),t("Özgürce bağlanın.", "Connect freely.")][win.page]; color: ink; font.pixelSize: win.width < 1180 ? 26 : 32; font.weight: Font.DemiBold; font.letterSpacing: -0.6 }
                        Text { Layout.fillWidth: true; text: t("Bilgisayarınıza uzaktan bağlanın veya başka bir cihaza güvenli şekilde erişin.", "Connect to your computer or securely access another device."); color: muted; font.pixelSize: 13; wrapMode: Text.WordWrap }
                    }
                    ColumnLayout {
                        visible: win.page === 0 && !App.sessionActive
                        Layout.fillWidth: true; Layout.leftMargin: 32; Layout.rightMargin: 32; spacing: 24
                        GridLayout {
                            Layout.fillWidth: true; columns: win.width < 1270 ? 1 : 2
                            columnSpacing: 20; rowSpacing: 20
                            GridLayout {
                                Layout.fillWidth: true; columns: 2; columnSpacing: 20
                                Panel {
                                    Layout.fillWidth: true; Layout.preferredWidth: 380; Layout.minimumWidth: 300; Layout.preferredHeight: 490
                                    surface: win.surface; edge: win.edge
                                    ColumnLayout {
                                        anchors.fill: parent; anchors.margins: 24; spacing: 15
                                        RowLayout {
                                            Glyph { name: "screen"; width: 24; height: 24 }
                                            Text { Layout.fillWidth: true; text: t("Bu Cihazın Bağlantı Bilgileri", "This Device"); color: ink; font.pixelSize: 15; font.weight: Font.DemiBold; wrapMode: Text.WordWrap }
                                        }
                                        Rectangle { Layout.fillWidth: true; height: 1; color: edge }
                                        Text { text: t("Bağlantı kimliğiniz", "Your device identity"); color: muted; font.pixelSize: 12 }
                                        Rectangle {
                                            Layout.fillWidth: true; Layout.preferredHeight: 58; radius: 10
                                            color: App.dark ? "#0a1427" : "#f0f5fd"; border.color: edge
                                            RowLayout {
                                                anchors.fill: parent; anchors.margins: 10
                                                Text { Layout.fillWidth: true; text: App.deviceId; color: ink; font.family: "Consolas"; font.pixelSize: 12; wrapMode: Text.WrapAnywhere; Accessible.name: t("Cihaz kimliği", "Device identity") }
                                                ToolButton {
                                                    implicitWidth: 28; implicitHeight: 32; onClicked: { App.copyId(); copyTip.open() }
                                                    contentItem: Glyph { name: "copy"; width: 18; height: 18 }
                                                    ToolTip { id: copyTip; text: t("Kopyalandı", "Copied"); timeout: 1200 }
                                                    Accessible.name: t("Kimliği kopyala", "Copy identity")
                                                }
                                            }
                                        }
                                        RowLayout {
                                            Text { Layout.fillWidth: true; text: t("Tek kullanımlık şifre", "One-time password"); color: muted; font.pixelSize: 12 }
                                            Text { text: t("5 dk", "5 min"); color: muted; font.pixelSize: 10 }
                                        }
                                        RowLayout {
                                            spacing: 8
                                            Field { Layout.fillWidth: true; text: App.password; readOnly: true; echoMode: reveal.checked ? TextInput.Normal : TextInput.Password; ink: win.ink; surface: App.dark ? "#0a1427" : "#f0f5fd"; font.family: "Consolas"; font.pixelSize: 17; Accessible.name: t("Tek kullanımlık şifre", "One-time password") }
                                            ToolButton { id: reveal; checkable: true; implicitWidth: 26; text: checked ? "●" : "○"; palette.buttonText: blue; Accessible.name: t("Şifreyi göster", "Show password") }
                                            ToolButton { implicitWidth: 26; implicitHeight: 30; onClicked: App.refreshPassword(); contentItem: Glyph { name: "refresh" } Accessible.name: t("Şifreyi yenile", "Refresh password") }
                                        }
                                        ActionButton { Layout.fillWidth: true; text: App.invitation.length ? t("Daveti kopyala", "Copy invitation") : t("LAN bağlantısına izin ver", "Enable LAN connections"); onClicked: { if (App.invitation.length) App.copyInvitation(); else App.startHosting() } }
                                        Text { Layout.fillWidth: true; visible: App.invitation.length > 0; text: App.invitation; color: blue; font.pixelSize: 10; wrapMode: Text.WrapAnywhere }
                                        Item { Layout.fillHeight: true }
                                        RowLayout {
                                            Glyph { name: "lock"; ink: muted; width: 13; height: 13 }
                                            Text { Layout.fillWidth: true; text: t("Yalnızca güvendiğiniz kişilerle paylaşın.", "Share only with people you trust."); color: muted; font.pixelSize: 10; wrapMode: Text.WordWrap }
                                        }
                                    }
                                }
                                Panel {
                                    Layout.fillWidth: true; Layout.preferredWidth: 380; Layout.minimumWidth: 300; Layout.preferredHeight: 490
                                    surface: win.surface; edge: "#31558a"
                                    ColumnLayout {
                                        anchors.fill: parent; anchors.margins: 24; spacing: 15
                                        RowLayout {
                                            Glyph { name: "connect"; width: 24; height: 24 }
                                            Text { Layout.fillWidth: true; text: t("Uzak Cihaza Bağlan", "Connect to a Device"); color: ink; font.pixelSize: 15; font.weight: Font.DemiBold; wrapMode: Text.WordWrap }
                                        }
                                        Rectangle { Layout.fillWidth: true; height: 1; color: edge }
                                        Text { text: t("Karşı cihazın bağlantı kimliği", "Remote device identity"); color: muted; font.pixelSize: 12 }
                                        Field { id: remoteId; Layout.fillWidth: true; placeholderText: "fm2://192.168…"; ink: win.ink; surface: App.dark ? "#0a1427" : "#f0f5fd"; font.family: "Consolas"; maximumLength: 128; onAccepted: App.connectLan(text, remotePassword.text); Accessible.name: t("Uzak cihaz kimliği", "Remote device identity") }
                                        Field { id: remotePassword; Layout.fillWidth: true; placeholderText: t("Karşı cihazın tek kullanımlık şifresi", "Remote one-time password"); echoMode: TextInput.Password; ink: win.ink; surface: App.dark ? "#0a1427" : "#f0f5fd"; onAccepted: App.connectLan(remoteId.text,text) }
                                        RowLayout {
                                            Glyph { name: "shield"; ink: "#67d7b3"; width: 15; height: 15 }
                                            Text { Layout.fillWidth: true; text: t("Her oturumda karşı tarafın onayı gerekir.", "Every session requires the owner's approval."); color: muted; font.pixelSize: 11; wrapMode: Text.WordWrap }
                                        }
                                        ActionButton { Layout.fillWidth: true; text: t("Bağlan  →", "Connect  →"); onClicked: App.connectLan(remoteId.text, remotePassword.text) }
                                        Item { Layout.fillHeight: true }
                                        Text { Layout.fillWidth: true; text: t("Aynı ağda LAN daveti + şifre. İlk sürüm: JPEG/WebRTC ekran aktarımı.", "Same network: LAN invitation + password. Initial version: JPEG/WebRTC screen streaming."); color: "#c4a46d"; font.pixelSize: 10; wrapMode: Text.WordWrap }
                                    }
                                }
                            }
                            Panel {
                                Layout.fillWidth: win.width < 1270
                                Layout.preferredWidth: win.width < 1270 ? -1 : 205
                                Layout.preferredHeight: win.width < 1270 ? 175 : 364
                                surface: App.dark ? "#142746" : "#e3edff"; edge: "#315178"
                                clip: true
                                Rectangle { width: 230; height: 230; radius: 115; x: parent.width - 140; y: 90; color: App.dark ? "#1c3e6c" : "#c6dcff"; opacity: 0.5 }
                                ColumnLayout {
                                    anchors.fill: parent; anchors.margins: 22; spacing: 12
                                    Text { text: t("SPONSORLU · ÖRNEK", "SPONSORED · SAMPLE"); color: muted; font.pixelSize: 8; font.letterSpacing: 1.1 }
                                    RowLayout { Glyph { name: "cloud"; width: 26; height: 26 } Text { text: "CloudBox"; color: ink; font.pixelSize: 19; font.weight: Font.DemiBold } }
                                    Text { visible: win.width >= 1270; Layout.fillWidth: true; text: t("Dosyalarınız\nher zaman\ngüvende!", "Your files.\nAlways\nprotected."); color: ink; font.pixelSize: 25; font.weight: Font.DemiBold; lineHeight: 1.12 }
                                    Text { Layout.fillWidth: true; text: t("Güvenli bulut depolama ile verilerinize her yerden erişin.", "Secure cloud storage. Access your data from anywhere."); color: muted; font.pixelSize: 11; wrapMode: Text.WordWrap; lineHeight: 1.35 }
                                    Item { Layout.fillHeight: true }
                                    Text { text: t("Örnek sponsor alanı", "Sample sponsor placement"); color: blue; font.pixelSize: 10 }
                                }
                            }
                        }
                        RowLayout {
                            Layout.fillWidth: true
                            Text { text: t("BAĞLANMANIN DAHA ÖZGÜR HALİ", "A FREER WAY TO CONNECT"); color: muted; font.pixelSize: 10; font.weight: Font.DemiBold; font.letterSpacing: 1.3 }
                            Item { Layout.fillWidth: true }
                            Text { text: t("Planlanan özellikler", "Planned features"); color: muted; font.pixelSize: 10 }
                        }
                        GridLayout {
                            Layout.fillWidth: true; columns: 4; columnSpacing: 14
                            Repeater {
                                model: [ ["screen",t("Uzaktan Kontrol","Remote Control"),t("Bilgisayarınızı uzaktan yönetin.","Your desktop, from anywhere.")], ["file",t("Dosya Aktarımı","File Transfer"),t("Hızlı ve güvenli dosya paylaşımı.","Fast and secure file sharing.")], ["bolt",t("Hızlı Bağlantı","Fast Connection"),t("Düşük gecikme, yüksek performans.","Low latency. High performance.")], ["shield",t("Güvenli Erişim","Secure Access"),t("Şifreli ve kullanıcı onaylı oturumlar.","Encrypted, consent-based sessions.")] ]
                                delegate: Panel {
                                    required property var modelData
                                    Layout.fillWidth: true; Layout.preferredWidth: 200; Layout.preferredHeight: 174
                                    surface: win.surface; edge: win.edge
                                    ColumnLayout {
                                        anchors.fill: parent; anchors.margins: 18; spacing: 10
                                        Glyph { name: modelData[0]; width: 29; height: 29 }
                                        Text { Layout.fillWidth: true; text: modelData[1]; color: ink; font.pixelSize: 13; font.weight: Font.DemiBold; wrapMode: Text.WordWrap }
                                        Text { Layout.fillWidth: true; text: modelData[2]; color: muted; font.pixelSize: 11; wrapMode: Text.WordWrap; lineHeight: 1.3 }
                                        Item { Layout.fillHeight: true }
                                    }
                                }
                            }
                        }
                        Panel {
                            Layout.fillWidth: true; Layout.preferredHeight: 84
                            surface: App.dark ? "#102132" : "#e5f5ef"; edge: App.dark ? "#22473e" : "#b7dacb"
                            RowLayout {
                                anchors.fill: parent; anchors.margins: 20; spacing: 15
                                Glyph { name: "heart"; ink: "#6cd3b2"; width: 24; height: 24 }
                                ColumnLayout {
                                    Layout.fillWidth: true; spacing: 5
                                    Text { Layout.fillWidth: true; text: t("Ücretsiz. Sınırsız. Herkes için.", "Free. Unlimited. For everyone."); color: ink; font.pixelSize: 13; font.weight: Font.DemiBold; wrapMode: Text.WordWrap }
                                    Text { Layout.fillWidth: true; text: t("FreeMotra, sponsor desteğiyle ücretsiz kalmayı hedefler. Aktif oturumlarda reklam yok.", "FreeMotra aims to stay free with sponsor support. No ads during active sessions."); color: muted; font.pixelSize: 11; wrapMode: Text.WordWrap }
                                }
                                ToolButton { text: t("Daha fazla bilgi ↗", "Learn more ↗"); palette.buttonText: blue; onClicked: win.page = 3 }
                            }
                        }
                    }
                    Panel {
                        visible: App.sessionActive
                        Layout.fillWidth: true; Layout.leftMargin: 32; Layout.rightMargin: 32; Layout.preferredHeight: 230
                        surface: win.surface; edge: "#40d4a0"
                        ColumnLayout {
                            anchors.fill: parent; anchors.margins: 28
                            Text { text: t("● AKTİF UZAK OTURUM", "● ACTIVE REMOTE SESSION"); color: "#74e0b1"; font.pixelSize: 24 }
                            Text { Layout.fillWidth: true; text: App.hosting ? t("Ekranınız paylaşılıyor. Kontrol izni: ", "Your screen is being shared. Control permission: ") + App.remoteControl : t("Uzak ekran ayrı pencerede açık.", "The remote screen is open in a separate window."); color: ink; wrapMode: Text.WordWrap }
                            ActionButton { text: t("Hemen sonlandır", "Disconnect now"); onClicked: App.endSession() }
                        }
                    }
                    Panel {
                        visible: win.page === 1
                        Layout.fillWidth: true; Layout.leftMargin: 32; Layout.rightMargin: 32; Layout.preferredHeight: 320
                        surface: win.surface; edge: win.edge
                        ColumnLayout {
                            anchors.centerIn: parent; width: parent.width - 70; spacing: 16
                            Glyph { Layout.alignment: Qt.AlignHCenter; name: "history"; width: 42; height: 42 }
                            Text { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; text: t("Henüz bir bağlantı yok", "No connections yet"); color: ink; font.pixelSize: 20 }
                            Text { Layout.fillWidth: true; horizontalAlignment: Text.AlignHCenter; text: t("Oturum geçmişi bağlantı motoruyla birlikte eklenecek.", "Session history will be added with the connection engine."); color: muted; font.pixelSize: 13; wrapMode: Text.WordWrap }
                        }
                    }
                    Panel {
                        visible: win.page === 2
                        Layout.fillWidth: true; Layout.leftMargin: 32; Layout.rightMargin: 32; Layout.preferredHeight: settingsContent.implicitHeight + 54
                        surface: win.surface; edge: win.edge
                        ColumnLayout {
                            id: settingsContent
                            anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top; anchors.margins: 27; spacing: 23
                            Text { text: t("Görünüm ve tercihler", "Appearance and preferences"); color: ink; font.pixelSize: 18; font.weight: Font.DemiBold }
                            RowLayout {
                                Text { Layout.fillWidth: true; text: t("Dil", "Language"); color: ink }
                                ActionButton { implicitWidth: 200; primary: false; text: english ? "English  →  Türkçe" : "Türkçe  →  English"; onClicked: App.language = english ? "tr" : "en" }
                            }
                            RowLayout {
                                Text { Layout.fillWidth: true; text: t("Koyu tema", "Dark theme"); color: ink }
                                Switch { checked: App.dark; onToggled: App.dark = checked; Accessible.name: t("Koyu tema", "Dark theme") }
                            }
                            RowLayout {
                                Text { Layout.fillWidth: true; text: t("Bildirim tercihi", "Notification preference"); color: ink }
                                Switch { checked: App.notifications; onToggled: App.notifications = checked; Accessible.name: t("Bildirimler", "Notifications") }
                            }
                            Text { Layout.fillWidth: true; text: t("Görüntü kalitesi tercihi · Motor hazır olduğunda uygulanacak", "Video quality preference · Applies when the engine is available"); color: muted; wrapMode: Text.WordWrap }
                            RowLayout {
                                Repeater {
                                    model: [["speed",t("Hız", "Speed")],["balanced",t("Dengeli", "Balanced")],["quality",t("Kalite", "Quality")]]
                                    delegate: ActionButton { required property var modelData; Layout.fillWidth: true; primary: App.quality === modelData[0]; text: modelData[1]; onClicked: App.quality = modelData[0] }
                                }
                            }
                            Rectangle { Layout.fillWidth: true; height: 1; color: edge }
                            Text { text: t("Bağlantı güvenliği", "Connection security"); color: ink; font.pixelSize: 18; font.weight: Font.DemiBold }
                            Text { Layout.fillWidth: true; text: t("Kullanıcı onayı zorunlu. Gözetimsiz erişim kapalı. Bu sürüm ağ portu açmaz.\nCihaz kimliği bir yetkilendirme anahtarı değildir.", "Owner consent is mandatory. Unattended access is disabled. This build opens no network ports.\nA device identity is not an authorization key."); color: muted; wrapMode: Text.WordWrap; lineHeight: 1.5 }
                            Text { Layout.fillWidth: true; text: t("Ayarlar bu Windows kullanıcısı için otomatik kaydedilir.", "Settings are saved automatically for this Windows user."); color: blue; font.pixelSize: 11 }
                        }
                    }
                    Panel {
                        visible: win.page === 3
                        Layout.fillWidth: true; Layout.leftMargin: 32; Layout.rightMargin: 32; Layout.preferredHeight: aboutContent.implicitHeight + 60
                        surface: win.surface; edge: win.edge
                        ColumnLayout {
                            id: aboutContent
                            anchors.left: parent.left; anchors.right: parent.right; anchors.top: parent.top; anchors.margins: 30; spacing: 22
                            Text { text: "FreeMotra 0.1.0"; color: ink; font.pixelSize: 28; font.weight: Font.Bold }
                            Text { Layout.fillWidth: true; text: t("Kişisel ve ticari kullanım için ücretsiz uzaktan erişim hedefiyle geliştiriliyor. Bu ilk sürüm, yerel arayüz ve güvenlik altyapısını içerir; uzaktan bağlantı henüz sunulmaz.", "Built toward free remote access for personal and commercial use. This first version contains the local interface and security foundation; remote connections are not available yet."); color: muted; wrapMode: Text.WordWrap; lineHeight: 1.5 }
                            Text { text: t("Sponsor politikamız", "Our sponsor policy"); color: ink; font.pixelSize: 18; font.weight: Font.DemiBold }
                            Text { Layout.fillWidth: true; text: t("Reklam izleme zorunluluğu, açılır reklam ve geri sayım yok. Bağlantı sırasında reklam gösterilmeyecek. Ekran, dosya ve şifreler reklam hedeflemesinde kullanılmayacak. CloudBox yalnızca kurgusal bir örnektir; veri toplamaz.", "No mandatory ad viewing, pop-ups or countdowns. No ads during connections. Screens, files and passwords will never be used for ad targeting. CloudBox is a fictional sample; it collects no data."); color: muted; wrapMode: Text.WordWrap; lineHeight: 1.5 }
                            Text { text: t("Lisanslar", "Licenses"); color: ink; font.pixelSize: 18; font.weight: Font.DemiBold }
                            Text { Layout.fillWidth: true; text: t("FreeMotra: MIT. Qt 6: LGPLv3 kapsamında dinamik bağlantı. Ayrıntılar paketin NOTICE.md ve docs klasöründedir.", "FreeMotra: MIT. Qt 6: dynamically linked under LGPLv3. See NOTICE.md and the docs folder in the package."); color: muted; wrapMode: Text.WordWrap; lineHeight: 1.5 }
                        }
                    }
                    Text { Layout.leftMargin: 32; Layout.rightMargin: 32; Layout.bottomMargin: 25; text: t("© 2026 FreeMotra  ·  İzin sizde. Kontrol sizde.", "© 2026 FreeMotra  ·  Your permission. Your control."); color: muted; font.pixelSize: 10 }
                }
            }
        }
    }
    Popup {
        id: messagePopup
        anchors.centerIn: Overlay.overlay
        width: Math.min(win.width - 70, 490)
        padding: 28; modal: true; focus: true
        closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
        background: Panel { surface: win.surface; edge: win.edge }
        contentItem: ColumnLayout {
            spacing: 22
            RowLayout { Glyph { name: "info" } Text { text: t("Bağlantı bilgisi", "Connection information"); color: ink; font.pixelSize: 19; font.weight: Font.DemiBold } }
            Text { Layout.fillWidth: true; text: App.message; color: muted; font.pixelSize: 14; wrapMode: Text.WordWrap; lineHeight: 1.5 }
            ActionButton { Layout.fillWidth: true; text: t("Tamam", "OK"); onClicked: messagePopup.close() }
        }
        onClosed: App.dismissMessage()
    }
    RemoteWindow { }
    Popup {
        id: consentPopup
        anchors.centerIn: Overlay.overlay; width: Math.min(550,win.width-70); padding: 28
        modal: true; focus: true; closePolicy: Popup.NoAutoClose
        background: Panel { surface: win.surface; edge: "#4e91ff" }
        contentItem: ColumnLayout {
            spacing: 20
            Text { text: t("Gelen bağlantı isteği", "Incoming connection request"); color: ink; font.pixelSize: 22 }
            Text { Layout.fillWidth: true; text: App.incomingPeer; color: blue; wrapMode: Text.WrapAnywhere }
            Text { Layout.fillWidth: true; text: t("Bu kişi ekranınızı görmek istiyor. Gösterilen ad karşı tarafın beyanıdır. Yalnızca tanıdığınız ve davet ettiğiniz kişiye izin verin.", "This person wants to see your screen. The displayed name is self-declared. Allow only someone you know and invited."); color: ink; wrapMode: Text.WordWrap }
            ActionButton { Layout.fillWidth: true; text: t("Yalnızca ekranı paylaş", "Allow viewing only"); onClicked: { App.approve(false); consentPopup.close() } }
            ActionButton { Layout.fillWidth: true; text: t("Ekran + fare ve klavye izni ver", "Allow screen + mouse and keyboard"); onClicked: { App.approve(true); consentPopup.close() } }
            ActionButton { Layout.fillWidth: true; primary: false; text: t("Reddet", "Reject"); onClicked: { App.endSession(); consentPopup.close() } }
        }
    }
    Connections { target: App; function onSessionChanged() { if (App.incomingPeer.length) consentPopup.open(); else consentPopup.close() } }
    Connections { target: App; function onMessageChanged() { if (App.message.length > 0) messagePopup.open() } }
}
