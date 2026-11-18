# FreeMotra 0.3.2 — Windows LAN denemesi

C++20 / Qt 6.8 / QML. Son kullanıcı Qt, Python veya derleyici kurmaz.
[En son Windows sürümü](https://github.com/bukiplatform-sys/FreeMotra/releases/latest)
üzerinden kurulum EXE’sini veya taşınabilir ZIP’i indirin. ZIP’i tümüyle
çıkarıp bin/FreeMotra.exe dosyasını açın; DLL’leri ayırmayın.

## Bağlantı ve oturum araçları

Bağlantı şu an aynı özel yerel ağ/LAN içindir. Davet + geçici şifre + açık
kullanıcı onayı gerekir. İnternet signaling/STUN/TURN ve H.264 henüz yoktur.
Uygulama gözetimsiz erişim veya UAC atlatma içermez.

Kayıtlı cihazlar/favoriler, devam edebilir dosya aktarımı, monitör seçimi,
otomatik görüntü kalitesi, ayrı izinler ve yazılı/sesli sohbet bulunur.
[LAN rehberi](docs/LAN.md) · [Oturum araçları ve sınırlar](docs/SESSION-TOOLS.md)

## Güncelleme

0.3.2 yeni sürümleri otomatik denetler. Uygulama içinden indirilen kurulum
dosyası SHA-256 ve boyutla doğrulanır. Kurulum ayrıca kullanıcı onayı ister;
açık veya bekleyen oturum varken başlatılmaz. Eski sürümlerden 0.3.2’ye ilk
geçiş elle yapılır. [Güncelleme ve yayınlama](docs/UPDATES.md).

## Reklam yönetimi

Sponsor Studio’nun Supabase sürümü canlıdır. Yönetim paneli ayrı bir özel
web sitesidir; EXE’de yönetici şifresi veya sunucu gizli anahtarı bulunmaz.
0.3.1 ve sonrası varsayılan olarak herkese açık salt okunur reklam yayınına
bağlıdır. [Bulut mimarisi](docs/SPONSOR-CLOUD.md).
Yerel Node.js sürümü sponsor-admin altında geliştirme seçeneği olarak korunur.

## Doğrulama ve çıkış planı

Windows CI; taşıma, oturum araçları, güncelleme dosyası doğrulaması, QML açılışı
ve Qt SDK olmadan çalışmayı denetler. Fiziksel iki bilgisayar testi ayrıca
gereklidir. Kod imzası ve bağımsız güvenlik incelemesi henüz tamamlanmadı.
[Mevcut durum](docs/DURUM.md) · [1.0 çıkış koşulları](docs/ROADMAP-1.0.md)
Lisans ve üçüncü taraf yükümlülükleri LICENSE ve NOTICE.md içindedir.
