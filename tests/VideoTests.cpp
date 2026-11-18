#include "windows/VideoCodec.h"
#include <QtTest>
class VideoTests:public QObject {
 Q_OBJECT
private slots:
 void encodedRoundTrip(){
  fm::VideoCodec encoder,decoder;if(!decoder.decoderAvailable())QSKIP("Windows Media Feature Pack is unavailable; application uses JPEG fallback");
  QImage source(640,360,QImage::Format_RGB32);for(int y=0;y<source.height();++y)for(int x=0;x<source.width();++x)source.setPixel(x,y,qRgb(40+x/4,30+y/3,120));
  QByteArray bytes;QImage restored;
  for(int frame=0;frame<60&&restored.isNull();++frame){bytes=encoder.encode(source,1000000);QVERIFY2(!encoder.failed(),"Media Foundation encoder failed");if(bytes.isEmpty()){QTest::qWait(10);bytes=encoder.poll();}if(!bytes.isEmpty())restored=decoder.decode(bytes,encoder.width(),encoder.height());QTest::qWait(5);}
  QVERIFY2(!restored.isNull(),"No H.264 frame could be decoded");QCOMPARE(restored.size(),source.size());
  qint64 difference=0;for(int y=20;y<340;y+=20)for(int x=20;x<620;x+=20){const auto a=source.pixel(x,y),b=restored.pixel(x,y);difference+=qAbs(qRed(a)-qRed(b))+qAbs(qGreen(a)-qGreen(b))+qAbs(qBlue(a)-qBlue(b));}
  QVERIFY2(difference/(16*30*3)<35,"Decoded color/stride mismatch");QVERIFY(bytes.size()<source.sizeInBytes());
  QVERIFY(decoder.decode("invalid",0,0).isNull());QVERIFY(decoder.decode("invalid",3000,2000).isNull());
  encoder.reset();decoder.reset();
 }
};
QTEST_GUILESS_MAIN(VideoTests)
#include "VideoTests.moc"
