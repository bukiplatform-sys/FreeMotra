#include "AppController.h"
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
int main(int argc, char* argv[]) {
    QQuickStyle::setStyle("Basic");
    qInstallMessageHandler([](QtMsgType, const QMessageLogContext&, const QString& message) {
        QFile file("freemotra-diagnostics.log");
        if (file.open(QIODevice::WriteOnly | QIODevice::Append)) { QTextStream out(&file); out << message << "\n"; }
    });
    QGuiApplication app(argc, argv);
    QCoreApplication::setOrganizationName("FreeMotra");
    QCoreApplication::setApplicationName("FreeMotra");
    QCoreApplication::setApplicationVersion("0.1.0");
    const bool screenshot = app.arguments().contains("--screenshot");
    const bool smoke = app.arguments().contains("--smoke-test");
    if (smoke) QCoreApplication::setApplicationName("FreeMotra-SmokeTest");
    try {
        AppController controller;
        QQmlApplicationEngine engine;
        bool qmlWarning = false;
        QObject::connect(&engine, &QQmlEngine::warnings, &app, [&](const QList<QQmlError>&) { qmlWarning = true; });
        engine.rootContext()->setContextProperty("App", &controller);
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
                window->resize(1040, 720);
            });
            QTimer::singleShot(1600, &app, [&, window] {
                controller.setLanguage("tr"); controller.setDark(true);
                window->resize(1536, 1024);
            });
            QTimer::singleShot(2500, &app, [&] { app.exit(qmlWarning ? 4 : 0); });
        }
        return app.exec();
    } catch (const std::exception& e) { std::fprintf(stderr, "%s\n", e.what()); return 1; }
}
