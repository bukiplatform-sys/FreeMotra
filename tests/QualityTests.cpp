#include "AdaptiveQuality.h"
#include <QtTest>
class QualityTests:public QObject {
 Q_OBJECT
private slots:
 void adaptsAndRecovers(){
  fm::AdaptiveQuality quality;
  for(int i=0;i<8;++i)quality.observe(900);
  QCOMPARE(quality.level(),0);QVERIFY(quality.averageMillis()>600);
  for(int i=0;i<20;++i)quality.observe(80);
  QCOMPARE(quality.level(),2);QVERIFY(quality.averageMillis()<170);
  quality.reset();QCOMPARE(quality.level(),1);
 }
 void smoothsShortSpikes(){
  fm::AdaptiveQuality quality;
  for(int i=0;i<10;++i)quality.observe(100);
  QCOMPARE(quality.level(),2);
  quality.observe(300);QCOMPARE(quality.level(),2);
  for(int i=0;i<12;++i)quality.observe(200);
  QCOMPARE(quality.level(),2); // Neutral band retains the current setting.
  for(int i=0;i<12;++i)quality.observe(350);
  QCOMPARE(quality.level(),1);
 }
};
QTEST_GUILESS_MAIN(QualityTests)
#include "QualityTests.moc"
