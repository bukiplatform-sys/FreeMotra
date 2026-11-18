# Ücretsiz internet bağlantısı (0.4.1)

Ev sahibi İnternet daveti oluştur düğmesine basar, durum Hazır olduktan sonra
FM cihaz kimliğini (veya fm4 davetini) ve tek kullanımlık şifreyi güvenli bir kanaldan paylaşır. Diğer
bilgisayar daveti Bağlan alanına yapıştırır. Ev sahibi izinleri ayrıca onaylar.
Cihaz kimliği kalıcıdır ve favorilere kaydedilebilir; her bağlantıda güncel
tek kullanımlık şifre ve ev sahibi onayı gerekir. Dinleyici beş dakika
bekler, şifre yenilenirse yeniden açılmalıdır. Her iki bilgisayarda 0.4 kullanın.

Supabase Realtime yalnızca şifreli auth/SDP/ICE mesajlarını taşır. Rastgele
oda ve 256-bit AES-GCM anahtarı kullanılır. Kimlikle bağlantıda oda kimliğin
hashidir, anahtar 60-bit rastgele tek kullanımlık şifreden 310.000 turluk
PBKDF2-SHA256 ile türetilir. Rastgele fm3 test davetlerinde oda 128-bit,
anahtar 256-bit rastgeledir; bu özel davetin tamamı gizlidir. Her gönderenin sırası,
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

STUN: stun.cloudflare.com:3478. Çalışmayan ortak OpenRelay denemesi kaldırıldı.
Röle hesabı varsayılan olarak kapalıdır. Özel Sponsor Studio panelinde
“İnternet rölesi (TURN)” bölümüne sağlayıcınızın UDP adresi ve TURN kimliğini
kaydedin. Reklam ve internet ayarı aynı yönetici oturumunu kullanır; ayar
veritabanında RLS ile kapalıdır. REST ortak sırrı sunucuda AES-GCM ile şifreli
saklanır; uygulamaya yalnızca bir saatlik HMAC-SHA1 TURN kimliği verilir.
Sunucu service-role anahtarı değiştirilirse röle sırrını yeniden girin.
Sabit TURN şifresi modunda yalnızca TURN yetkili kimlik uygulamalara gönderilir;
sağlayıcı yönetim API anahtarını girmeyin. Ayar yeni bağlantılarda uygulanır.
Bir saatten uzun röle oturumları sağlayıcının kimlik süresi davranışına bağlıdır.

Ücretsiz hesap seçenekleri: ExpressTURN 1.000 GB/ay veya Cloudflare Realtime
SFU/TURN ortak 1.000 GB/ay kota. Hesap sahibi giriş yapıp TURN kimlik bilgilerini
oluşturmalıdır. Henüz sağlayıcı hesabı bağlanmadı ve ücretli plan açılmadı.
Kota bitince sağlayıcı engelleyebilir veya ücretli yapılandırmada ücret çıkabilir;
ücretsiz planda kalın, ücretli otomatik geçişi açmayın. Sınırsız ücretsiz röle
ve her ağda bağlantı garantisi yoktur.
https://www.expressturn.com/
https://developers.cloudflare.com/realtime/sfu/platform/pricing/

Mevcut ICE kütüphanesinin libjuice arka ucu TURN/UDP destekler. TURN/TCP/TLS
bu derlemede yoktur; UDP kapalı kurumsal ağlar için tam bağlantı garantisi
verilmez. Röle için sahibine ait hesap gerekir.
Üretimde ölçek, kötüye kullanım sınırlaması ve hizmet devamlılığı ayrıca
izlenmelidir. Supabase genel yayın anahtarı yönetim/veritabanı yazma yetkisi
vermez; mevcut RLS kuralları korunur.

## Doğrulama

Windows CI gerçek Supabase üzerinde iki uçla şifreli mesaj ve WebRTC oturumunu
dener. Relay-only testi yalnızca hesap yapılandırılmışsa çalışır; aksi halde
SKIP ile rölenin doğrulanmadığını açıkça bildirir. Yapılandırılmış röle başarısız
olursa yayın testi başarısız olur; hata başarıya çevrilmez. İlk ortak röle
probu zaman aşımına uğradı. Fiziksel farklı ağ/NAT, GPU, mikrofon ve uzun süreli
röle testleri ayrıca yapılmalıdır. [Pilot listesi](PILOT-TEST.md).
