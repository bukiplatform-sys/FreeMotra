# FreeMotra 0.3.2 geliştirme durumu

- Yerel ağ TLS signaling, davet sertifikası doğrulama, tek kullanımlık şifre,
  yerel kullanıcı onayı ve WebRTC DTLS/SCTP taşıması bulunur.
- Desktop Duplication → JPEG → WebRTC ekran akışı, izinli SendInput,
  görünür oturum durumu ve bağlantıyı bitirme arayüze bağlıdır.
- Kayıtlı cihazlar/favoriler, SHA-256 doğrulamalı devam edebilir dosya aktarımı,
  monitör seçimi, otomatik kalite ve ayrı ekran/kontrol/dosya/sohbet/ses izinleri vardır.
- Yazılı sohbet ve PCM ses görüşmesi vardır; Opus/yankı giderme henüz yoktur.
- Sponsor Studio Supabase üzerinde canlıdır. 0.3.1 ve sonrası reklamları
  varsayılan salt okunur yayından alır; yönetim yetkileri EXE içinde bulunmaz.
- 0.3.2 uygulama içi güncelleme denetleme, doğrulanmış indirme ve kullanıcı
  onayıyla Windows kurulumunu başlatmayı ekler. [UPDATES.md](UPDATES.md).

## Henüz tamamlanmayanlar

İnternet signaling/STUN/TURN, donanım H.264, Türkçe özel klavye tuşları,
döndürülmüş ekranlar, bağımsız güvenlik incelemesi, Authenticode kod imzası
ve gerçek cihaz pilotu. [1.0 planı](ROADMAP-1.0.md).

## Testlerin kapsamı

Windows runner testleri aynı makinede TLS/WebRTC eşleri, yanlış şifre/pin,
izinler, sohbet, dosya aktarımının iptal/devamı ve bütünlüğü, adaptif kalite,
reklam ayrıştırma, güncelleme sürüm/adres/hash/boyut sınırları ve QML açılışını
kapsar. Bozuk/yarım indirme, iptal/yeniden deneme, kurulum öncesi dosya
kurcalaması ve açık oturumda kurulumun engellenmesi ayrıca denenir.
Yayın sonrası gerçek GitHub paketi aynı güncelleme koduyla indirilip doğrulanır;
test kurucuyu çalıştırmaz. Sonuçların kaynağı ilgili Actions koşusudur.

İki fiziksel Windows bilgisayarı arasında ekran, fare/klavye, mikrofon,
monitör ve sürükle bırak testi bu ortamda yapılamadı. Runner testleri bunun
yerine geçmez. UAC ve daha yüksek yetkili pencereler kontrol edilemez.
