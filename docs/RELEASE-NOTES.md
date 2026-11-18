FreeMotra 0.4.0 — internet bağlantısı ve oturum iyileştirmeleri

- İnternet daveti: Supabase üzerinden AES-256-GCM ile şifrelenmiş bağlantı mesajları, WebRTC/STUN doğrudan bağlantı ve opsiyonel ücretsiz topluluk TURN rölesi.
- Tek kullanımlık şifre ve ev sahibi onayı korunur. İnternet daveti beş dakika içinde kullanılmalıdır; bağlantı kurulunca genel signaling kanalı kapatılır.
- H.264: uygun sistemde GPU kodlaması denenir; yazılımsal H.264 ve JPEG geri dönüşü vardır. Gerçek GPU/sürücü sonuçları cihaza bağlıdır.
- Opus ses, SpeexDSP yankı/gürültü azaltma ve ayrı düşük gecikmeli ses kanalı.
- Fare/klavye trafiği büyük dosya ve görüntü paketlerinden ayrıldı.
- Türkçe/Unicode metin girişi, döndürülmüş monitörler ve ekran yakalama kaybında yeniden hazırlama.
- CodeQL güvenlik analizi ve GitHub/Sigstore paket kaynak doğrulaması. Bu, Windows Authenticode imzası değildir.

İki tarafta da 0.4.0 kullanılması önerilir. İnternet modunda STUN/TURN adresleri IP adresinizi görebilir; içerik taşıması DTLS şifrelidir. Genel ücretsiz röle bir garanti/SLA sunmaz ve UDP'yi engelleyen bazı ağlarda çalışmayabilir. Özel TURN hesabı gerektiğinde yapılandırılmalıdır.

Code signing policy: https://github.com/bukiplatform-sys/FreeMotra/blob/main/docs/CODE-SIGNING.md
