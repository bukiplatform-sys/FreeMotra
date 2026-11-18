# Fiziksel iki bilgisayar pilotu

Bu liste yapılmış testlerin raporu değildir. Sonuçlar iki gerçek bilgisayara
erişim sağlandığında doldurulmalıdır. Windows runner protokol ve kurulum
testleri gerçek GPU, mikrofon, DPI ve ev/kurumsal NAT koşullarının yerine geçmez.

| Kontrol | Beklenen sonuç | Sonuç |
| --- | --- | --- |
| Aynı ağda LAN daveti | Şifre ve ev sahibi onayı olmadan oturum açılmaz | Bekliyor |
| Farklı internetlerde FM kimliği | Güncel şifre ve onayla bağlantı; gerekirse röle | Bekliyor |
| Yalnızca izleme | Fare/klavye ve izinsiz dosya mesajları reddedilir | Bekliyor |
| Türkçe metin | ç, ğ, ı, İ, ö, ş, ü doğru yazılır | Bekliyor |
| Farklı DPI ve negatif monitör konumu | Fare görüntüdeki doğru noktaya gider | Bekliyor |
| 90/180/270 derece ekran | Görüntü ve kontrol yönleri eşleşir | Bekliyor |
| Monitör takıp çıkarma | Yakalama toparlanır veya açıklamayla güvenli kapanır | Bekliyor |
| H.264 destekli GPU | Kalite bilgisinde H.264 GPU; görüntüde bozulma yok | Bekliyor |
| GPU desteği olmayan cihaz | Yazılımsal H.264 veya JPEG çalışır | Bekliyor |
| Sesli görüşme | Opus, mikrofon açma/kapama ve yankı azaltma denenir | Bekliyor |
| Büyük dosya + ses + kontrol | Girdi/ses kuyruğu dosya akışından ayrıdır | Bekliyor |
| Dosya kesilmesi/devamı | Son SHA-256 eşleşir, var olan dosya korunur | Bekliyor |
| Ağ kopması/uyku | Kontrol ve basılı tuşlar bırakılır; yeniden onay gerekir | Bekliyor |
| Yeni sürüme geçiş | Kimlik ve LAN/favori ayarları korunur | Bekliyor |
| Önceki sürüme elle dönüş | Eski resmi kurulum çalışır; güncelleme ayarları bozulmaz | Bekliyor |

Her sonuçla Windows sürümü, ekran kartı/sürücü, monitör/DPI, internet türü ve
FreeMotra sürümü kaydedilmelidir. Parola, davet anahtarı, sohbet veya dosya
içeriği hata raporuna eklenmemelidir.
