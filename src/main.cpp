#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <iostream>

int main(int argc, char *argv[]) {
    std::cout << "initializing fraktalized..." << std::endl;

    // initialize qt context
    QGuiApplication app(argc, argv);

    // setup qml loading engine
    QQmlApplicationEngine engine;

    const QUrl url(QStringLiteral("qrc:/fraktalizedModule/qml/main.qml"));

    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);

    engine.load(url);

    // run the app
    return app.exec();
}