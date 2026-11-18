# Ücretsiz internet bağlantısı (0.4)

Ev sahibi İnternet daveti oluştur düğmesine basar, durum Hazır olduktan sonra
fm3 davetini ve tek kullanımlık şifreyi güvenli bir kanaldan paylaşır. Diğer
bilgisayar daveti Bağlan alanına yapıştırır. Ev sahibi izinleri ayrıca onaylar.
Davetin tamamı gizlidir; sonundaki anahtarı yayınlamayın. Davet beş dakika
geçerlidir, kalıcı bir cihaz kimliği değildir; yeniden dinlemede yenilenir.
LAN favorileri korunur; internet davetleri kalıcı cihaz yer işareti olarak
kaydedilmez. Her iki bilgisayarda 0.4 kullanın.

Supabase Realtime yalnızca şifreli auth/SDP/ICE mesajlarını taşır. Rastgele
128-bit oda ve 256-bit AES-GCM anahtarı kullanılır. Her gönderenin sırası,
oda ve kimlik AAD ile doğrulanır; tekrar mesajları reddedilir. Sunucu anahtar
ve şifreyi almaz. Görüntü, ses, dosya ve girdiler WebRTC DTLS üzerinden doğrudan
veya TURN ile taşınır. Bağlantıdan üç saniye sonra signaling soketi kapatılır.
Bağlantı kesilince kontrol izinleri ve basılı tuşlar temizlenir; otomatik
olarak tekrar kontrol izni verilmez.

## Ücretsiz sınırlar

Supabase Free: 200 eşzamanlı Realtime bağlantısı, ayda 2 milyon mesaj; bu
kotalar proje genelinde paylaşılır. İçerik trafiği Supabase'e gönderilmez.
Açık bekleyen davetler de bağlantı kotasından kullanır; beş dakika sonunda
kapatılır. Kaynak: https://supabase.com/docs/guides/realtime/limits ve
https://supabase.com/docs/guides/realtime/pricing .

STUN: stun.cloudflare.com:3478. TURN: Open Relay'in resmi dokümantasyonunda
Nextcloud için yayınladığı staticauth.openrelay.metered.ca:80 ve public
openrelayprojectsecret ile bir saatlik TURN-REST kimlik bilgisi üretilir.
Bu anahtar bir kullanıcı hesabının gizli anahtarı değildir; sağlayıcının
bilerek yayınladığı ortak kullanım sırrıdır. Ortak hizmetin çalışması/kotası
FreeMotra'ya ait özel bir 20 GB hakkı anlamına gelmez. Topluluk rölesi QSettings
internet/communityRelay=false ile kapatılabilir. Kaynak:
https://www.metered.ca/tools/openrelay/ .

Daha güçlü ücretsiz seçenekler: OpenRelay hesabında yayınlanan 20 GB/ay planı,
veya Cloudflare Realtime'ın SFU/TURN ortak 1.000 GB/ay ücretsiz kotası.
Yeni hesap/kimlik bilgileri ve sağlayıcı koşulları gerekir; herhangi bir
ücretli plan etkinleştirilmemiştir. Cloudflare kotası aşılırsa ücret oluşabilir;
bu uygulamada otomatik Cloudflare hesabı veya abonelik açılmaz.
https://developers.cloudflare.com/realtime/sfu/platform/pricing/

Mevcut ICE kütüphanesinin libjuice arka ucu TURN/UDP destekler. TURN/TCP/TLS
bu derlemede yoktur; UDP kapalı kurumsal ağlar için tam bağlantı garantisi
verilmez. Ücretsiz topluluk rölesi kaldırılırsa özel röle hesabı gerekir.
Üretimde ölçek, kötüye kullanım sınırlaması ve hizmet devamlılığı ayrıca
izlenmelidir. Supabase genel yayın anahtarı yönetim/veritabanı yazma yetkisi
vermez; mevcut RLS kuralları korunur.

## Doğrulama

Windows CI gerçek Supabase üzerinde iki uçla şifreli mesaj ve WebRTC oturumunu
dener. Ayrıca relay-only ICE ile ortak TURN servisi üzerinde ayrı bir ağ
probu çalışır; bu dış hizmetin geçici erişilemezliği kaydedilir. Bu testler
iki fiziksel kullanıcı bilgisayarının farklı internetleri, NAT'ları, GPU ve
mikrofonlarını sınamış sayılmaz.
