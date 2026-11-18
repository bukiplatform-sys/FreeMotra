#include "VoiceCodec.h"
#include <QtTest>
#include <cmath>
#include <array>
class VoiceTests:public QObject {
 Q_OBJECT
private slots:
 void voiceRoundTrip(){
  fm::VoiceCodec sender,receiver;std::array<opus_int16,320> samples{};
  for(int frame=0;frame<30;++frame){for(int n=0;n<320;++n)samples[n]=opus_int16(std::sin((frame*320+n)*0.17)*5000);QByteArray raw(reinterpret_cast<const char*>(samples.data()),640);const auto compressed=sender.encode(raw);QVERIFY(!compressed.isEmpty());QVERIFY(compressed.size()<640);const auto restored=receiver.decode(compressed);QCOMPARE(restored.size(),640);sender.playback(restored);}
  sender.reset();receiver.reset();QVERIFY(sender.encode("bad").isEmpty());QVERIFY(receiver.decode({}).isEmpty());QVERIFY(receiver.decode(QByteArray(1276,'x')).isEmpty());
 }
};
QTEST_GUILESS_MAIN(VoiceTests)
#include "VoiceTests.moc"
