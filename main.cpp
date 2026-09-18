#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QTranslator>
#include <QFile>
#include "AppController.h"
#include "ShomCoastDownloader.h"

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QCoreApplication::setApplicationName("CoastWarn");
    QCoreApplication::setApplicationVersion("1.0");
    QCoreApplication::setOrganizationName("alefbet");
    QCoreApplication::setOrganizationDomain(".net");

    //Loads the translation
    auto language = QLocale::languageToCode(QLocale::system().language());
    qDebug() << "Current language:" << language;

    QTranslator translator;
    auto filename = QString(":/i18n/%1.qm").arg(language);
    QFile f(filename);
    if(f.open(QIODevice::ReadOnly)) {
        qDebug() << f.readAll();
        f.close();
    } else {
        qWarning() << f.errorString();
    }
    if(translator.load(filename)) {
        qInfo() << "Translation loaded for" << QLocale::system().language();
        qDebug() << "Translation file=" << filename;

        if(QCoreApplication::installTranslator(&translator)) {
            qInfo() << "Translation set";
        } else {
            qWarning() << "Translation not set";
        }
    } else {
        qWarning() << "Translation not loaded for" << QLocale::system().language();
    }

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
