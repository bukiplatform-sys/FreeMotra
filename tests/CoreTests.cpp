#include "Identity.h"
#include "SessionGate.h"
#include <QtTest>
#include <QTemporaryDir>
class CoreTests: public QObject {
 Q_OBJECT
private slots:
 void identity() { QTemporaryDir d; QSettings s(d.filePath("s.ini"),QSettings::IniFormat); fm::Identity a(s),b(s); QCOMPARE(a.deviceId(),b.deviceId()); auto p=a.refreshPassword(); QVERIFY(a.verifyAndConsume(p)); QVERIFY(!a.verifyAndConsume(p)); }
 void consent() { fm::SessionGate g; QVERIFY(!g.localConsent(true)); QVERIFY(g.authenticatedPeer(QString(64,'a'))); QVERIFY(!g.mayControl()); QVERIFY(g.localConsent(false)); QVERIFY(g.mayView()); QVERIFY(!g.mayControl()); g.end(); QVERIFY(!g.mayView()); }
};
QTEST_GUILESS_MAIN(CoreTests)
#include "CoreTests.moc"
