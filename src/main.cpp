#include "AppController.h"
#include "SponsorFeed.h"
#include "UpdateManager.h"
#include <QSettings>
#include <QUrl>
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTimer>
#include <cstdio>
#include <QQuickStyle>
#include <QFile>
#include <QImage>
#include <QTextStream>
#include <QQuickImageProvider>
class RemoteImageProvider final:public QQuickImageProvider {
public:
 explicit RemoteImageProvider(AppController* app):QQuickImageProvider(QQuickImageProvider::Image),app_(app){}
 QImage requestImage(const QString&,QSize* size,const QSize&) override {auto image=app_->frame();if(size)*size=image.size();return image;}
private:AppController* app_;
};
int main(int argc, char* argv[]) {
    QQuickStyle::setStyle("Basic");
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString& message) {
        QFile file("freemotra-diagnostics.log");
        if (file.open(QIODevice::WriteOnly | QIODevice::Append)) { QTextStream out(&file); out << message << "\n"; }
    });
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("FreeMotra");
    QCoreApplication::setApplicationName("FreeMotra");
    QCoreApplication::setApplicationVersion(QStringLiteral(FREEMOTRA_VERSION));
    const bool screenshot = app.arguments().contains("--screenshot");
    const bool smoke = app.arguments().contains("--smoke-test");
    if (smoke) QCoreApplication::setApplicationName("FreeMotra-SmokeTest");
    try {
        const auto options=app.arguments();
        const int feedIndex=options.indexOf("--sponsor-feed");
        if(feedIndex>=0 && feedIndex+1<options.size()) {
            const QUrl url(options[feedIndex+1]);
            if(url.scheme()!="https"||url.host().isEmpty()||!url.userInfo().isEmpty())return 6;
            QSettings().setValue("sponsors/feedUrl",url.toString());
        }
        AppController controller;
        SponsorFeed sponsor;
        UpdateManager updates(!smoke && !screenshot);
        QObject::connect(&controller,&AppController::sessionChanged,&updates,[&]{ updates.setBlocked(controller.updateBlocked()); });
        QObject::connect(&controller,&AppController::sessionChanged,&sponsor,[&]{sponsor.pause(controller.sessionActive());});
        QQmlApplicationEngine engine;
        engine.addImageProvider("remote",new RemoteImageProvider(&controller));
        bool qmlWarning = false;
        QObject::connect(&engine, &QQmlEngine::warnings, &app, [&](const QList<QQmlError>&) { qmlWarning = true; });
        engine.rootContext()->setContextProperty("App", &controller);
        engine.rootContext()->setContextProperty("Tools", controller.tools());
        engine.rootContext()->setContextProperty("Sponsor", &sponsor);
        engine.rootContext()->setContextProperty("Updates", &updates);
        engine.loadFromModule("FreeMotra", "Main");
        if (engine.rootObjects().isEmpty()) return 2;
        if (screenshot) {
            auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
            if (!window) return 3;
            controller.setLanguage("tr"); controller.setDark(true);
            window->resize(1460, 940);
            QTimer::singleShot(1800, &app, [&, window] {
                const auto frame = window->grabWindow();
                app.exit(!frame.isNull() && frame.save("FreeMotra-preview.png") ? 0 : 5);
            });
        }
        if (smoke) {
            auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
            if (!window) return 3;
            QTimer::singleShot(800, &app, [&, window] {
                controller.setLanguage("en"); controller.setDark(false);
                window->setProperty("page",2);
                if(auto* dialog=window->findChild<QObject*>("updateInstallDialog"))dialog->setProperty("visible",true);
                window->resize(1040, 720);
            });
            QTimer::singleShot(1600, &app, [&, window] {
                controller.setLanguage("tr"); controller.setDark(true);
                window->setProperty("page",3);
                if(auto* dialog=window->findChild<QObject*>("updateInstallDialog"))dialog->setProperty("visible",false);
                window->resize(1536, 1024);
            });
            QTimer::singleShot(2500, &app, [&] { app.exit(qmlWarning ? 4 : 0); });
        }
        return app.exec();
    } catch (const std::exception& e) { std::fprintf(stderr, "%s\n", e.what()); return 1; }
}
