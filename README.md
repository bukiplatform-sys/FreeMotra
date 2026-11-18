# FreeMotra 0.3 — Windows LAN denemesi

C++20 / Qt 6.8 / QML. Kaynaklar CMake ve GitHub Actions ile Windows x64 için
derlenir. Son kullanıcı Qt, Python veya derleyici kurmaz. Portable ZIP'i tümüyle
çıkarıp bin/FreeMotra.exe dosyasını açın; DLL'leri ayırmayın.

LAN kullanımı: [docs/LAN.md](docs/LAN.md). Davet + geçici şifre + açık kullanıcı
onayı gerekir. İlk ekran aktarımı JPEG/WebRTC DTLS/SCTP kullanır. H.264/SRTP
medya yolu ve internet STUN/TURN henüz yoktur.
Uygulama gözetimsiz erişim veya UAC atlatma içermez.

Sponsor Studio [sponsor-admin](sponsor-admin) altında ayrı Node.js web servisidir.
FreeMotra EXE içinde yönetici girişi yoktur. Sponsor yayını yapılandırmak için:
`FreeMotra.exe --sponsor-feed https://sizin-sunucunuz/api/sponsors`
Bu seçenek kullanıcı ayarına yalnızca public feed adresini kaydeder. Yönetici
şifresi uygulamaya girilmez. Servis henüz canlı sunucuya dağıtılmış değildir.

Windows paketlerini Actions artifact bölümünden indirin. Testsiz veya başarısız
build çıktıları kullanılmamalıdır. Çalıştırılan testlerin sınırları docs/DURUM.md
ve LAN.md içinde açıklanır. Kod imzası yoksa SmartScreen uyarısı çıkabilir.
Dağıtımdan önce NOTICE.md ve Qt kaynak/lisans yükümlülüklerini tamamlayın.

## 0.3 oturum araçları

Kayıtlı cihazlar ve favoriler, sürükle-bırak ve devam edebilir dosya aktarımı,
monitör seçimi, otomatik görüntü kalitesi, ayrı izinler ve yazılı/sesli sohbet.
[Kullanım, güvenlik ve sınırlar](docs/SESSION-TOOLS.md). Qt Multimedia 6.8 gereklidir.
