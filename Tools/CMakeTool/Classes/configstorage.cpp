#include "configstorage.h"
#include <QFile>
#include <QMessageBox>
#include <QApplication>

ConfigStorage::ConfigStorage(QObject *parent)
    : QObject(parent)
{
#ifdef Q_OS_WIN
    configFilePath = "../CMakeTool/Data/config_windows.txt";
#elif defined Q_OS_MAC
    configFilePath = qApp->applicationDirPath() + "/../Resources/Data/config_mac.txt";
#else
    qCritical() << "application started on undefined platform";
    return 1;
#endif //platform
}

QString ConfigStorage::GetJSONTextFromConfigFile() const
{
    if(!QFile::exists(configFilePath))
    {
        QMessageBox::critical(nullptr, QObject::tr("Config file not available!"), QObject::tr("Can not find config file %1").arg(configFilePath));
        exit(0);
    }
    QFile configFile(configFilePath);
    if(configFile.open(QIODevice::ReadOnly))
    {
        return configFile.readAll();
    }
    else
    {
        QMessageBox::critical(nullptr, QObject::tr("Failed to open config file!"), QObject::tr("Failed to open config file %1").arg(configFilePath));
        exit(0);
    }
    return QString();
}

void ConfigStorage::SaveToConfigFile(QString config)
{
    QFile configFile(configFilePath);
    if(configFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
    {
        configFile.write(config.toUtf8());
    }
    else
    {
        auto button = QMessageBox::question(nullptr, tr("Can't open file"), tr("Can not open config file %1\n. do you want to repeat?").arg(configFilePath));
        if(button == QMessageBox::Yes)
        {
            SaveToConfigFile(config);
        }
    }
}
