# FreeMotra 1.0 çıkış koşulları

Mevcut sürüm 0.4.1 internet betasıdır. Şifreli signaling, doğrudan WebRTC,
H.264, Opus, yankı azaltma ve ayrı taşıma kanalları uygulanmıştır.
Röle sağlayıcı hesabı, fiziksel pilot ve kod imzası beklemektedir. Reklam servisi Supabase üzerinde canlıdır;
bu servis uzak masaüstü trafiğini veya internet oturumlarını taşımaz.

1. İnternet bağlantısı: uygulanmış kimlik/signaling/STUN akışının pilotu; TURN hesabı,
   kısa ömürlü relay yetkileri, farklı NAT/CGNAT ve kurumsal ağ testleri.
   Supabase cihaz kaydı/signaling için kullanılabilir; sürekli görüntü ve
   TURN aktarma trafiği için ayrıca altyapı ve kapasite bütçesi gerekir.
2. Güvenlik: bağımsız protokol/kod incelemesi, tekrar saldırısı ve kimlik
   doğrulama kontrolleri, kötüye kullanım sınırları, Windows Authenticode
   imzası ve yayın bütünlüğü. Denetimsiz erişim eklenirse ayrı tehdit modeli.
3. Performans: H.264 donanım kodlama, akışların birbirini tıkamaması,
   dosya aktarımı sırasında kontrol gecikmesi ölçümü. PCM yerine Opus,
   yankı giderme ve mikrofon cihazı değişimini destekleme.
4. Windows uyumu: Türkçe klavye, DPI/ölçek farkları, döndürülmüş monitör,
   ekran takıp çıkarma, uyku/uyanma, bağlantı kopması ve antivirüs kontrolleri.
   UAC/yüksek yetkili pencere desteği için açık ve kontrollü yetki tasarımı.
5. Gerçek cihaz testleri: en az iki fiziksel Windows 10/11 bilgisayar,
   farklı internet sağlayıcıları ve ağ koşulları. Mevcut runner testleri,
   fiziksel ekran/fare/klavye, mikrofon ve sürükle bırak testlerinin yerine geçmez.
6. İşletim: tercihe bağlı ve kişisel içerik taşımayan hata raporu,
   bağlantı tanılama, servis sağlık takibi, yedekleme/geri dönüş planı,
   güncelleme başarısızlığı ve önceki sürüme dönüş denemeleri.
7. Ürün: anlaşılır ilk bağlantı rehberi, site/indirme sayfası, gizlilik
   açıklaması, lisans/üçüncü taraf bildirimleri, destek ve hata bildirme kanalı.

Önerilen sıra: internet bağlantısı → fiziksel cihaz pilotu → performans ve
güvenlik sertleştirmesi → imzalı kurulum/güncelleme → sınırlı beta → 1.0.
