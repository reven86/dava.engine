#include "configstorage.h"
#include <QFile>
#include <QJsonDocument>
#include <QMessageBox>

ConfigStorage::ConfigStorage(const QString &configFilePath_, QObject *parent)
    : QObject(parent)
    , configFilePath(configFilePath_)
{

}

QString ConfigStorage::GetJSONTextFromConfig() const
{
    if(!QFile::exists(configFilePath))
    {
        QMessageBox::critical(nullptr, QObject::tr("Config file not available!"), QObject::tr("Can not find config file %1").arg(configFilePath));
        return QString();
    }
    QFile configFile(configFilePath);
    if(configFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = configFile.readAll();
        QJsonParseError err;
        QJsonDocument::fromJson(data, &err);
        if(err.error != QJsonParseError::NoError)
        {
            QMessageBox::critical(nullptr, QObject::tr("Config file corrupted!"), QObject::tr("Failed to parse config file: error %1").arg(err.errorString()));
            return QString();
        }
        return data;
    }
    else
    {
        QMessageBox::critical(nullptr, QObject::tr("Failed to open config file!"), QObject::tr("Failed to open config file %1").arg(configFilePath));
        return QString();
    }
    return QString();
}

void ConfigStorage::SaveJSONTestToConfig(QString config)
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
            SaveJSONTestToConfig(config);
        }
    }
}
