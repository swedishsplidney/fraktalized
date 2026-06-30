#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QDirIterator>
#include <QDebug>
#include <QQmlContext>
#include "tutorialmanager.h"

int main(int argc, char *argv[]) {
    // force native opengl
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    qputenv("QSG_RENDER_LOOP", "basic");

    QGuiApplication app(argc, argv);

    // debug stuff
    qDebug() << "--- virtual resource map ---";
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        qDebug() << it.next();
    }
    qDebug() << "----------------------------";

    QQmlApplicationEngine engine;

    TutorialManager tutorialManager;

    engine.rootContext()->setContextProperty("tutorialManager", &tutorialManager);

    engine.load(QUrl(QStringLiteral("qml/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}