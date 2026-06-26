#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickWindow>

int main(int argc, char *argv[]) {
    // force native opengl
    QQuickWindow::setGraphicsApi(QSGRendererInterface::OpenGL);
    qputenv("QSG_RENDER_LOOP", "basic");

    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/fraktalized/qml/main.qml")));

    return app.exec();
}