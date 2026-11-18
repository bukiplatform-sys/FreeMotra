# 0.3 geliştirme durumu

- Yerel ağ TLS signaling, davet sertifikası doğrulama, tek kullanımlık şifre,
  yerel kullanıcı onayı ve WebRTC DTLS/SCTP taşıması kodlandı.
- Desktop Duplication -> JPEG -> WebRTC ekran akışı; izinli SendInput; görünür
  oturum durumu ve bağlantıyı bitirme arayüze bağlandı.
- H.264, medya SRTP, STUN/TURN internet servisi, Türkçe özel tuşlar ve oturum geçmişi henüz tamamlanmadı.
- Sponsor Studio ayrı web servisidir. Kimlik doğrulama, CSRF, şifre deneme
  sınırlaması, yayında/taslak ayrımı ve güvenlik başlıklarını kapsayan altı Node
  testi yerelde ve ilk CI çalışmasında geçti.
- Windows CI LAN testi aynı runner içinde iki LanLink nesnesiyle TLS, yanlış
  şifre/pin reddi, izin bekleme, WebRTC veri mesajı ve kopmayı doğrular.
  En son koşunun gerçek sonuçlarını Actions'tan kontrol edin; kod yazılması
  testin geçtiği anlamına gelmez.
- İki fiziksel Windows bilgisayarı arasında ekran ve fare/klavye testi bu
  ortamda yapılamadı. Bu yüzden bu kullanım doğrulanmış sayılmaz.
- Reklam paneli henüz bir canlı sunucuya dağıtılmadı. Docker ve kurulum rehberi
  dahildir. EXE'de yönetici paneli veya yönetici şifresi bulunmaz.

- 0.3: kayıtlı cihazlar/favoriler, kabul ve SHA-256 doğrulamalı devam edebilir
  dosya aktarımı, DXGI monitör seçimi, ACK süresine göre otomatik kalite, ayrı
  ekran/kontrol/dosya/sohbet/ses izinleri ve yazılı + PCM sesli sohbet eklendi.
  Kullanım ve sınırlar için [SESSION-TOOLS.md](SESSION-TOOLS.md) dosyasına bakın.
