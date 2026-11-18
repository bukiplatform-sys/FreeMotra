# FreeMotra 0.2 — LAN denemesi

İki bilgisayarda aynı 0.2 sürümünü açın. Ekranı paylaşacak bilgisayarda
“LAN bağlantısına izin ver” seçeneğine basın. Daveti ve beş dakikalık şifreyi
bağlanacak kişiye güvenli bir kanaldan iletin. Davet değiştirilemez sertifika
parmak izi içerir; şifre olmadan veya kullanıcı onayı olmadan paylaşım başlamaz.

Diğer bilgisayarda fm2:// davetini ve şifreyi girip Bağlan'a basın.
Ekranı paylaşan kişi yalnızca görüntüleme veya fare/klavye iznini seçer.
Kişi adı beyana dayanır; fiziksel olarak tanıdığınız kişiyi doğrulayın.
Her iki tarafta Oturumu bitir bağlantıyı keser. İzleyici Esc ile de kesebilir.
F11 tam ekrana geçer. Windows Güvenlik Duvarı sorarsa yalnızca Özel ağ izni verin.
Uygulama güvenlik duvarını otomatik değiştirmez ve yönetici olarak çalışmaz.

Bu sürüm LAN içindir. VPN/sanal ağ varsa ilk bulunan özel IPv4 adresi seçilebilir;
fm2 davetindeki IP'yi doğru LAN IPv4 adresiyle değiştirip aynı parmak izini koruyabilirsiniz.
İnternet signaling, STUN/TURN, dosya aktarımı ve H.264 henüz yoktur.
İlk ekran aktarımı Desktop Duplication -> JPEG -> WebRTC DTLS/SCTP üzerinden yapılır.
Bu H.264 değildir; kalite/performans optimizasyonu sonraki aşamadır.
İlk bağlı ekran aktarılır; döndürülmüş ekranlar ve ekran değiştirme henüz desteklenmez.
Türkçe klavye özel karakterleri/sistem kısayolları tam desteklenmez.
UAC ekranı ve daha yüksek yetkili uygulamalar kontrol edilemez.

Cihaz kimliği tek başına yönlendirme servisi değildir. Davet gereklidir.
LAN testleri bir Windows runner içinde iki bağımsız taşıma nesnesiyle gerçek
TLS/WebRTC bağlantısını dener. Bu, iki fiziksel Windows bilgisayarı testinin
ve görüntü/klavye uçtan uca testinin yerine geçmez.
