#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "AppController.h"
#include "ShomCoastDownloader.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    AppController ctrl;
    qmlRegisterSingletonInstance("CoastWarn", 1, 0, "AppController", &ctrl);
    qmlRegisterSingletonInstance("CoastWarn", 1, 0, "ShomCoastDownloader", ShomCoastDownloader::instance());

    QQmlApplicationEngine engine;
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreationFailed,
        &app,
        []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.loadFromModule("CoastWarn", "Main");

    ctrl.start();

    return QGuiApplication::exec();
}
