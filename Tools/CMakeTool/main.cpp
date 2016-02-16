#include <QApplication>
#include <QQmlApplicationEngine>
#include <QFile>
#include <QJsonDocument>
#include <QMessageBox>

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
        QMessageBox::critical(nullptr, tr("Config file not available!"), tr("Can not find config file %1").arg(configPath));
        return;
    }
    QFile configFile(fileName);
    if(configFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = configFile.readAll();
        QJsonParseError err;
        QJsonDocument::fromJson(data, &err);
        if(err.error != QJsonParseError::NoError)
        {
            QMessageBox::critical(nullptr, tr("Config file corrupted!"), tr("Failed to parse config file: error %1").arg(err.errorString()));
            return;
        }
    }
    else
    {
        QMessageBox::critical(nullptr, tr("Failed to open config file!"), tr("Failed to open config file %1").arg(configPath));
        return;
    }
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));


    return app.exec();
}

