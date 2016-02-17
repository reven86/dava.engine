#include <QApplication>
#include <QString>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "configstorage.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QString configPath;
#ifdef Q_OS_WIN
    configPath = ":/Resources/config_windows.txt";
#elif defined Q_OS_MAC
    configPath = ":/Resources/config_mac.txt";
#else
    qCritical() << "application started on undefined platform";
    return 1;
#endif //platform
    ConfigStorage configStorage(configPath);
    QString configuration = configStorage.GetJSONTextFromConfig();
    if(configuration.isEmpty())
    {
        return 0;
    }
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("configuration", configuration);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
    QObject::connect(engine.rootObjects().at(0), SIGNAL(dataReadyToSave(QString)), &configStorage, SLOT(SaveJSONTestToConfig(QString)));
    return app.exec();
}

