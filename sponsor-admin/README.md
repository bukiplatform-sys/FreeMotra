# FreeMotra Sponsor Studio

FreeMotra EXE'den ayrı web yönetim panelidir. Node.js 22+ kullanır; ek paket
bağımlılığı yoktur. Yönetici şifresi, HTTP-only SameSite cookie, CSRF kontrolü,
istek boyutu sınırları ve 5 deneme/15 dakika hız sınırlaması içerir.
Şifre hash'i scrypt'tir; düz şifre dosyaya yazılmaz.

## Yerel çalıştırma

Güçlü bir şifreyi `node hash-password.mjs` komutuna standart girdiden verin
(komut satırı argümanı kullanmayın). Çıktıyı ADMIN_PASSWORD_HASH ortam değişkenine
koyun. PUBLIC_ORIGIN=http://127.0.0.1:3000 ve NODE_ENV=development ayarlayın.
`npm start` ile http://127.0.0.1:3000/admin açılır. Yerel geliştirme HTTP'dir.
Üretimde TLS reverse proxy, NODE_ENV=production ve gerçek HTTPS PUBLIC_ORIGIN
zorunludur. Proxy Origin başlığını korumalıdır. HOST ve PORT ile bind ayarlanır.
Docker örneği dahildir; /app/data kalıcı volume'a bağlanmalıdır.
Tek servis instance çalıştırın; çoklu instance dağıtımı desteklenmez.

Sponsor adı, TR/EN başlık ve açıklama, HTTPS bağlantısı, PNG görsel,
başlangıç/bitiş tarihleri ve yayın durumu düzenlenir. Görsel en fazla 1 MB,
1600×1600 PNG olmalıdır. Silmek önce listeden kaldırır, yayınla kalıcı kaydeder.
Oturumlar 30 dakikada biter, sunucu yeniden başlayınca çıkış yapılır.
Servis hiçbir ekran, dosya, uzaktan erişim şifresi veya cihaz kimliği kabul etmez.

## Uygulamaya bağlama

Windows uygulamasının kullanıcı QSettings kayıtlarına sponsors/feedUrl olarak
bu sunucunun HTTPS /api/sponsors adresi eklenir. Uygulamada yönetici paneli yoktur.
Varsayılan URL boş bırakılır; bu paket henüz internete dağıtılmış bir servis değildir.
Yönetici şifresi/oturum cookie'si EXE'ye konmaz. Public feed yalnızca yayındaki
sponsorları döndürür. En fazla 15 dakikada yenilenir; aktif oturumda reklam yoktur.
