# FreeMotra 0.4.1 durumu

Uygulandı: şifreli Supabase internet signaling, cihaz kimliği + tek kullanımlık
şifre + yerel onay, WebRTC doğrudan internet bağlantısı; yönetici panelinden
TURN/UDP tanımlama; H.264 donanım/yazılım/JPEG geri dönüşü; Opus ve yankı azaltma;
ayrı kontrol/dosya/ses kanalları; Unicode metin, ekran döndürme ve yakalama
kaybında yeniden açma; favoriler, dosya devamı, monitörler, kalite, izinler,
yazılı/sesli sohbet; reklam yönetimi ve uygulama içi güncelleme.

Windows CI ve gerçek Supabase iki uç testi geçmiştir (0.4.0). 0.4.1 sonuçları
Actions kaydından izlenir. CodeQL 0.4.0 bulgu sayısı sıfırdır; kurulum/açılış/
kaldırma testi geçmiştir. Bu sonuçlar fiziksel iki cihaz testinin yerine geçmez.

TURN sağlayıcı hesabı henüz bağlı değildir. Ortak ücretsiz OpenRelay probu
başarısız olmuştur ve bu bağımlılık kaldırılmıştır. Ücretsiz ExpressTURN veya
Cloudflare hesabının TURN kimliği özel panelden girilmelidir. Relay-only test
hesap yoksa SKIP verir. TURN/TCP/TLS ve UDP kapalı ağlar desteklenmez.

Bekleyenler: gerçek farklı ağ pilotu, GPU/mikrofon/monitör/klavye testleri,
uzun oturum ve kota kontrolü; bağımsız insan güvenlik incelemesi; Windows
Authenticode imzası (SignPath başvuru taslağı hazırlanmıştır, başvurulmamıştır),
gizlilik/destek yayın bilgileri. Henüz 1.0 değildir.
