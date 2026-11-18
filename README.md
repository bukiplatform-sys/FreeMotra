# FreeMotra 0.4.1 — Windows internet betası

C++20 / Qt 6.8 / QML. Son kullanıcı Qt, Python veya derleyici kurmaz.
[Windows sürümünü indir](https://github.com/bukiplatform-sys/FreeMotra/releases/latest).
ZIP kullanıyorsanız tüm dosyaları çıkarın; bin/FreeMotra.exe dosyasını açın.

İnternet daveti oluşturun, hazır olduğunda cihaz kimliği ve geçici şifreyi paylaşın.
Ev sahibi görüntüleme, kontrol, dosya, yazılı sohbet ve ses izinlerini seçer.
Aynı ağ için LAN daveti de kullanılabilir. Gözetimsiz erişim ve UAC atlatma yoktur.
[İnternet rehberi](docs/INTERNET.md) · [LAN](docs/LAN.md) · [Oturum araçları](docs/SESSION-TOOLS.md).

Kayıtlı cihazlar/favoriler, devam edebilir dosya aktarımı, çoklu monitör ve otomatik
kalite bulunur. H.264 donanım kodlama denenir, yazılım/JPEG geri dönüşü vardır.
Ses Opus ile sıkıştırılır; SpeexDSP yankı ve gürültü azaltma kullanılır.
Kontrol, dosya ve ses ayrı WebRTC kanallarındadır.

Supabase yalnızca şifreli bağlantı kurulumu ve reklam yönetimi içindir. Doğrudan
WebRTC bağlantısı röle trafiği kullanmaz. Kısıtlı ağlar için sahibine ait ücretsiz
TURN hesabı özel yönetim panelinden tanımlanmalıdır. Hesap henüz bağlı değildir;
UDP kapalı ağlar bu sürümde desteklenmez. Ücretsiz planlar sınırlıdır.

0.3.2 ve sonrası yeni sürümü otomatik denetler; SHA-256 ve boyut doğrulanır.
Kurulum açık/bekleyen oturumda başlatılmaz ve ayrıca kullanıcı onayı ister.
[Güncelleme](docs/UPDATES.md) · [Reklam yönetimi](docs/SPONSOR-CLOUD.md).

Windows CI birim/entegrasyon, gerçek Supabase bağlantısı, QML açılışı, SDK olmadan
çalışma ve ayrı kurulum/kaldırma testleri yapar. CodeQL ve GitHub/Sigstore derleme
kökeni doğrulaması vardır. Bunlar Authenticode veya bağımsız güvenlik incelemesi
yerine geçmez. Fiziksel iki bilgisayar pilotu, röle sağlayıcı hesabı ve Windows
kod imzası tamamlanmadan 1.0 kabul edilmez.
[Durum](docs/DURUM.md) · [1.0 koşulları](docs/ROADMAP-1.0.md) · [Pilot](docs/PILOT-TEST.md).
Lisans ve üçüncü taraf bildirimleri LICENSE ve NOTICE.md içindedir.
