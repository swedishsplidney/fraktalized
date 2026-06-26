#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>
#include <QDirIterator>
#include <QDebug>

int main(int argc, char *argv[]) {
    // force native opengl
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    qputenv("QSG_RENDER_LOOP", "basic");

    QGuiApplication app(argc, argv);

    // debug stuff
    qDebug() << "--- Embedded Virtual Resource Map ---";
    QDirIterator it(":", QDirIterator::Subdirectories);
    while (it.hasNext()) {
        qDebug() << it.next();
    }
    qDebug() << "-------------------------------------";

    QQmlApplicationEngine engine;

    // fallback load target
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/fraktalized/qml/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}