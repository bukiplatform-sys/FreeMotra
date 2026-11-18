# FreeMotra 0.4.1 — LAN kullanımı

İki bilgisayarda aynı sürümü açın. Ekranı paylaşacak bilgisayarda
“LAN bağlantısına izin ver” seçeneğine basın. Daveti ve beş dakikalık tek
kullanımlık şifreyi bağlanacak kişiye güvenli bir kanaldan iletin.
Davet sertifika parmak izi içerir; şifre ve yerel kullanıcı onayı olmadan
paylaşım başlamaz.

Diğer bilgisayarda fm2:// davetini ve şifreyi girip Bağlan’a basın.
Ev sahibi görüntüleme, kontrol, dosya, sohbet ve ses izinlerini seçer.
Kişi adı beyana dayanır; karşı tarafın kimliğini ayrıca doğrulayın.
Her iki tarafta Oturumu bitir bağlantıyı keser. İzleyici Esc ile de kesebilir.
F11 tam ekrana geçer. Güvenlik duvarı sorarsa yalnızca özel ağ izni verin.
Uygulama güvenlik duvarını otomatik değiştirmez ve yönetici olarak çalışmaz.

LAN daveti özel ağ içindir. İnternet üzerinden cihaz kimliğiyle bağlantı için
[İnternet rehberini](INTERNET.md) kullanın. VPN arayüzünü seçerken sertifika
parmak izini koruyun. Ekran aktarımı DXGI → H.264/JPEG → WebRTC DTLS/SCTP
kullanır. Unicode metin ve döndürülmüş ekran desteği vardır; fiziksel farklı
klavye/DPI/monitör pilotu beklemektedir. UAC ve daha yüksek yetkili pencereler
kontrol edilemez.
[Dosya, ses ve diğer araçların ayrıntıları](SESSION-TOOLS.md).

LAN testleri aynı Windows runner içindeki iki bağımsız taşıma nesnesini dener.
Fiziksel iki bilgisayarda görüntü/klavye/ses uçtan uca testi ayrıca gereklidir.
