# BUTAS — Sürüm Kaydı

## 2.0 — 2026-11-01

Para kaynakları sürümü.

- Uygulama: `org.butas.app`, versionName **2.0**, versionCode **4**
- APK: `/BUTAS/butas-v2.0.apk` (imzalı, arm64-v8a, minSdk 23, targetSdk 34)
- Kalıcılık: `state.json` sürüm 2 (uygulamanın özel veri dizini)

### Bu sürümde tamamlananlar

- **Para kaynakları:** Gelir/Gider butonlarının altında "Kaynaklar" paneli.
  Kaynak ekleme (＋), yeniden adlandırma ve silme. Satırlarda yalnızca ad
  görünür; eksi bakiyeli kaynak soluk gösterilir.
- **Nakit** varsayılan kaynak olarak 0,00 ₺ ile gelir; silinemez, yeniden
  adlandırılamaz ve eksiye düşemez. Başka kaynak "Nakit" adını alamaz
  (büyük/küçük harf ve Türkçe karakter duyarsız).
- **Kaynak bilgi kartı:** ortada kaynak adı, bakiye, son 5 işlem ve
  "Eksiye düşebilir mi?" Evet/Hayır anahtarı (anında uygulanır).
- **Toplam bütçe** artık kaynak bakiyelerinin toplamıdır; ayrı bakiye
  alanı kaldırıldı.
- **Para Ekle / Para Çıkar:** miktarın altında kaynak açılır listesi;
  "Kaynak Ekle" listenin en üstünde. Kaynak seçilmeden onaylanamaz
  ("Kaynak seçiniz"). Bakiye yetersizse işlem engellenir
  ("Yeterli bakiye bulunmuyor").
- Kaynak silinince bakiyesi bütçeden düşer; işlem geçmişi korunur.
- **1.0 → 2.0 geçişi:** temiz başlangıç; eski bakiye yok sayılır, işlemler
  ve düzenli gelir/giderler korunur.

## 1.0 — 2026-10-30

İlk sürüm. Tüm düzeltmeler tamamlandı.

- Uygulama: `org.butas.app`, versionName **1.0**, versionCode **3**
- APK: `/BUTAS/butas-v1.0.apk` (imzalı, arm64-v8a, minSdk 23, targetSdk 34)
- Kalıcılık: `state.json` (uygulamanın özel veri dizini)

### Bu sürümde tamamlananlar

- Tasarım (`code.html` / `DESIGN.md` / `screen.png`) birebir Qt Widgets karşılığı;
  tüm dp ölçüleri (px-7, pt-10, h-14 boşluklar, 46 px butonlar, 34 px işlem
  etiketleri, p-5 panel, Space Grotesk) korunuyor.
- Para Ekle / Para Çıkar, Gelir Ekle / Gider Ekle modalları ve silme onayı.
- Bakiye, son 10 işlem ve aylık düzenli gelir/giderler kalıcı (`state.json`);
  her değişiklikte kaydedilir, açılışta yüklenir.
- Silme onayı modalı onaydan sonra kapanır.
- `₺` / `$` / `€` simgeleri giriş kutusunun sağ kenarından içeri alındı.
- Gelir/Gider modalındaki TL/$/€ seçicileri eşit genişlikte ve dikey ortalı.
- Kaydırmada nesne kopyalanması/üst üste binme giderildi:
  QScroller "lastik bant" (overshoot) kapatıldı; `QEvent::Scroll` doğrudan
  işlenip viewport hiç taşınmıyor, kaydırma durunca tam yeniden çizim yapılıyor.
- Uygulama ikonu: koyu antrasit (#27272A) zemin + Space Grotesk Bold beyaz
  **BTA** (legacy + round + adaptive, tüm yoğunluklar).
- İlk kullanıcı için temiz başlangıç (boş veri).

### Sonraki sürüm

- Bundan sonraki tüm güncellemeler **2.0** olarak kaydedilecek
  (versionName `2.0`, versionCode `4` ve üzeri).
- v1.0 kaynak ağacı dondurulmuştur; geliştirme 2.0 üzerinden yürür.
