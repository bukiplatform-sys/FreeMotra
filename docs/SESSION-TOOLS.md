# FreeMotra 0.3 — oturum araçları

## Kullanım

- **Kayıtlı cihazlar ve favoriler:** Bağlan kartından listeyi açın, ad ve LAN
  davetini kaydedin. Yıldızlı cihazlar önce görünür. Cihazı seçmek daveti doldurur;
  güncel tek kullanımlık şifre ve karşı bilgisayarın onayı gerekir. Şifreler
  kaydedilmez. Aynı adı kaydetmek daveti günceller. Windows kullanıcısının LAN
  sertifika anahtarı DPAPI ile korunur, sertifika ve dinleme portu tekrar kullanılır.
  Böylece dinleyiciyi yeniden başlatmak kaydı bozmaz. IP adresi değişirse, port
  çakışırsa veya sertifika bir yıllık geçerliliğinin sonuna gelirse daveti güncelleyin.
- **İzinler:** Ev sahibi ekran, kontrol, dosya, yazılı sohbet ve sesli görüşmeyi
  ayrı ayrı seçer. Kontrol ekran iznine bağlıdır. Ekran kapalı, dosya açık
  oturumlarda ekran yakalama başlatılmaz. İzinler oturum kapanınca sıfırlanır.
- **Dosyalar:** Oturum paneline veya uzak ekrana bir normal dosya bırakın ya da
  dosya seç düğmesini kullanın. Alıcı her dosyayı ayrıca kabul eder ve hedef
  klasörü seçer. Var olan dosyanın üzerine yazılmaz. Aynı anda her yönde bir
  aktarım desteklenir. Klasör aktarımı ve çoklu dosya kuyruğu bu sürüme dahil değil.
- **Devam etme:** Kopma veya iptal sonrası aynı dosyayı tekrar gönderip aynı
  klasörde kabul edin. `.freemotra-<SHA256>.part` dosyası kaldığı yerden kullanılır.
  Tamamlanınca bütün dosyanın SHA-256 değeri doğrulanır. Bozuk bir kısmi dosya
  doğrulamadan geçmez ve silinir; dosyayı tekrar gönderin. Kısmi dosyalar kullanıcı
  iptalinde tutulur; gerekmiyorsa seçtiğiniz klasörden silinebilir.
- **Monitör:** Uzak pencerenin üst listesinden DXGI ekranı seçilir. Kontrol
  koordinatları seçili ekranın sanal masaüstü konumuna çevrilir. Önceki ekrana
  ait girdiler uygulanmaz. Döndürülmüş ekranlar mevcut yakalama motorunda hâlâ
  desteklenmiyor; ekran topolojisi değişirse yeniden bağlantı kurun.
- **Otomatik kalite:** Ayarlarda Otomatik seçin. Kare teslim onay süresinin
  hareketli ortalamasına göre JPEG genişliği (800/1280/1600), kalite (40/65/80)
  ve yakalama aralığı (250/150/100 ms) değişir. Bunlar hedeflerdir; gerçek FPS
  ekran değişimi, kodlama ve bağlantı kapasitesine bağlıdır. Manuel modlar korunur.
- **Sohbet:** Oturum başına en son 200 mesaj gösterilir, mesaj başına 2000 karakter
  kabul edilir; içerik düz metindir. Oturum kapanınca mesajlar temizlenir.
- **Ses:** Ses izni verildikten sonra iki taraf da Mikrofon ve Karşı tarafı dinle
  anahtarlarını kendi panelinden açar. Mikrofon kendiliğinden açılmaz. Ses
  16 kHz / mono / PCM16 olarak mevcut DTLS veri kanalı üzerinden taşınır. Bu
  formatı destekleyen ses giriş/çıkışı gerekir; eksik cihazda açıklama gösterilir.
  Kulaklık kullanın: yankı giderme, gürültü azaltma ve Opus bu sürümde yoktur.

## Sınırlar ve doğrulama

Bağlantı hâlâ **aynı özel ağ/LAN** içindir. STUN/TURN internet bağlantısı bu
çalışmanın parçası değildir. Dosya parçaları sıralı teslim onaylarıyla ilerler;
ses paketleri kuyruk büyüdüğünde düşürülür. Ses, video ve dosya aynı güvenilir
kanalı paylaşır; yoğun aktarımda gecikme yaşanabilir. Büyük dosyalarda başlangıç
ve bitiş SHA-256 hesaplaması arka planda yapılır; bu aşamada hazırlama/doğrulama
bilgisi gösterilir. Alıcıdaki doğrulama için en fazla 10 dakika beklenir.

`session_features` testi iki gerçek TLS/WebRTC eşini kullanarak bağımsız
izinleri, izinsiz mesaj/dosya reddini, çift yönlü sohbeti, gerçek aktarımın
iptal edilip devam etmesini, dosya bütünlüğünü, boş dosyayı, var olan dosyayı
korumayı ve tehlikeli dosya adlarını denetler. Windows derlemesi ve QML açılış
kontrolleri CI'da çalışır. Fiziksel iki Windows cihazında mikrofon/hoparlör,
çoklu monitör ve sürükle-bırak ayrıca denenmelidir.


## Bu geliştirmede yapılan kontroller

- Qt 6.8.3 ile gerçek taşıma ve oturum aracı kaynakları Linux üzerinde derlendi.
- Gerçek QML dosyaları örnek uygulama durumu ile ana ekran, ev sahibi paneli,
  uzak panel, ekran kapalı oturum, izin penceresi ve tema/dil değişimlerinde
  uyarısız açıldı. Bu kontrol Windows ekran yakalamayı veya cihaz sürücülerini
  doğrulamaz.
- `adaptive_quality`: yavaş bağlantıya uyarlama, toparlanma ve kısa süreli
  gecikme sıçramalarının süzülmesi geçti.
- `session_protocol_in_memory`: gerçek SessionTools koduyla, soket kullanmayan
  test taşıyıcısı üzerinden dört senaryo geçti: sohbet/oturum temizliği, kapalı
  özelliklerin reddi, dosya iptal/devam/ad doğrulama ve bozuk kısmi/boş dosya.
  Bu test TLS/WebRTC el sıkışmasını doğrulamaz; `session_features` bunun için
  ayrıca gerçek taşıma kullanır.
- Gerçek ağ testlerinde bu ortamda WebRTC oturumu kurulamadı; testler başarılı
  sayılmadı. Windows derlemesi, DPAPI testi ve gerçek ağ testleri bekliyor.
- Sponsor Studio'nun mevcut altı testi geçti.
