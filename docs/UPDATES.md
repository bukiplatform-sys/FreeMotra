# Güncellemeler (0.3.2)

0.3.2 ve sonraki sürümler, resmi bukiplatform-sys/FreeMotra GitHub deposunun
en son yayınını açılıştan 5 saniye sonra ve 6 saatte bir kontrol eder.
Ayarlar ve Hakkında sayfalarından elle denetlenebilir. Yeni sürüm kartı her
sayfada görünür. Oturumu kesen veya otomatik kurulum başlatan pencere açılmaz.

İndir ve güncelle dosyayı uygulama içinde indirir. GitHub'ın HTTPS API'sindeki
SHA-256 ve boyut bilgileriyle doğrular; eksik, bozuk, farklı depoya ait veya
ön sürüm paketlerini reddeder. Yönlendirmeler GitHub indirme sunucularıyla
sınırlıdır. GitHub hesabının veya yayınlama yetkisinin ele geçirilmesine
karşı bağımsız imza doğrulaması değildir. Authenticode imzası henüz yoktur.

Güncellemeyi kur düğmesi ayrıca onay ister. Aktif bağlantı veya bekleyen
bağlantı isteği varsa kapatılması gerekir. Dosya yeniden doğrulanır,
Windows kurulum penceresi başlatılır ve FreeMotra kapanır. Sessiz kurulum
yapılmaz; Windows UAC penceresi ve kurulum adımları kullanıcıya aittir.
İndirme iptal edilirse kısmi paket silinir. Kurucu başlatıldıktan sonra
kilitlenmemesi için önbellekteki paket tutulur.

0.3.1 ve daha eski sürümlerde güncelleme kodu yoktur; 0.3.2 bir kez elle
kurulmalıdır. Sonraki yeni sürümler bu mekanizmayla bildirilir.

## Yayınlama

CMake ve vcpkg sürümlerini aynı x.y.z değerine yükseltin ve RELEASE-NOTES.md
dosyasını güncelleyin. main dalındaki CI, web testleri, Windows testleri,
QML açılışı ve SDK olmadan kurulum denetimleri geçince EXE ve ZIP'i önce
GitHub taslağına yükler; GitHub'ın EXE SHA-256 değerini yerel dosyayla
karşılaştırıp yayını görünür yapar. Var olan yayın dosyaları değiştirilmez.
Yeni bir düzeltme için sürüm numarası yükseltilmelidir. Başarısız taslaklar
yayın sorumlusu tarafından incelenmelidir; otomatik olarak yayınlanmaz.
