# FreeMotra 0.3.2 — LAN kullanımı

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

Bu sürüm aynı özel ağ/LAN içindir. VPN varsa davetteki özel IPv4 adresi doğru
arayüzle değiştirilebilir; sertifika parmak izi korunmalıdır. Cihaz kimliği
tek başına internet yönlendirme servisi değildir. İnternet signaling/STUN/TURN
henüz yoktur; Supabase reklam bağlantısı bunu değiştirmez.

Ekran aktarımı DXGI → JPEG → WebRTC DTLS/SCTP kullanır. Monitör değiştirilebilir;
döndürülmüş ekranlar henüz desteklenmez. Türkçe özel klavye tuşları/sistem
kısayolları tamamlanmadı. UAC ve daha yüksek yetkili pencereler kontrol edilemez.
[Dosya, ses ve diğer araçların ayrıntıları](SESSION-TOOLS.md).

LAN testleri aynı Windows runner içindeki iki bağımsız taşıma nesnesini dener.
Fiziksel iki bilgisayarda görüntü/klavye/ses uçtan uca testi ayrıca gereklidir.
