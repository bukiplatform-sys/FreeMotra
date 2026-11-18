# SignPath başvurusu için taslak bilgiler

Bu bir başvuru taslağıdır; gönderilmemiştir ve kabul/onay alınmamıştır.

- Proje: FreeMotra
- Depo: https://github.com/bukiplatform-sys/FreeMotra
- İndirmeler: https://github.com/bukiplatform-sys/FreeMotra/releases
- Lisans: MIT (bağımlılıkların lisansları NOTICE.md ve paket içindeki third-party altında)
- Bakımcı hesabı: bukiplatform-sys
- İşlev: Kullanıcı onayı ve ayrı izinlerle Windows ekran paylaşımı/kontrol,
  dosya aktarımı ve yazılı/sesli destek. Görünmez/gözetimsiz kontrol yoktur.
- Yapı: Windows GitHub Actions, C++/Qt, testler ve CodeQL analizi.
- Doğrulanabilir çıktı: EXE/ZIP SHA-256 ve GitHub/Sigstore kaynak attestasyonu.
- İmzalanması istenen kendi çıktıları: FreeMotra.exe ve Windows kurucusu.
  Üçüncü taraf DLL'leri FreeMotra'nın kendi kodu gibi yeniden imzalanmaz.
- Politika: docs/CODE-SIGNING.md (gizlilik ve sağlayıcı bağlantıları dahil).

Bakımcının tamamlaması gerekenler: proje kontrolünün doğrulanması, gerçek
iletişim bilgisi, bağımsız gözden geçiren ve sürüm onaylayıcı rolleri,
Foundation'ın uygunluk değerlendirmesi ve onaylı imzalama politikasının
kurulması. Bu bilgiler kullanıcı adına uydurulmaz.

Başvuru ve koşullar: https://signpath.org/ ve https://signpath.org/terms .
