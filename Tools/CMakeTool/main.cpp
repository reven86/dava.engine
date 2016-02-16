#include <QApplication>
#include <QString>
#include <QQmlApplicationEngine>
#include <QFile>
#include <QJsonDocument>
#include <QMessageBox>
#include <QQmlContext>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QString configPath;
#ifdef Q_OS_WIN
    configPath = ":/Resources/config_windows.txt";
#elif Q_OS_MAC
    configPath = ":/Resources/config_mac.txt";
#else
    qCritical() << "application started on undefined platform";
    return 1;
#endif //platform
    if(!QFile::exists(configPath))
    {
        QMessageBox::critical(nullptr, QObject::tr("Config file not available!"), QObject::tr("Can not find config file %1").arg(configPath));
        return 0;
    }
    QFile configFile(configPath);
    QVariant configuration;
    if(configFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = configFile.readAll();
        QJsonParseError err;
        QJsonDocument::fromJson(data, &err);
        if(err.error != QJsonParseError::NoError)
        {
            QMessageBox::critical(nullptr, QObject::tr("Config file corrupted!"), QObject::tr("Failed to parse config file: error %1").arg(err.errorString()));
            return 0;
        }
        configuration.setValue(data);
    }
    else
    {
        QMessageBox::critical(nullptr, QObject::tr("Failed to open config file!"), QObject::tr("Failed to open config file %1").arg(configPath));
        return 0;
    }
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("configuration", configuration);
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}

