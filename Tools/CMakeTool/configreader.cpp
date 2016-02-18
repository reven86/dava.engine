#include "configreader.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

ConfigReader::ConfigReader(QObject *parent) : QObject(parent)
{

}

void ConfigReader::ReadConfig(const QString &configPath, QString &err)
{
    QFile configFile(configPath);
    if(!configFile.open(QFile::ReadOnly))
    {
        err = tr("Can not open file %1").arg(configPath);
        return;
    }
    QByteArray data = configFile.readAll();
    QJsonParseError jsonError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &jsonError);
    if(jsonError.error != QJsonParseError::NoError)
    {
        err = tr("JSON parse error: %1").arg(jsonError.errorString());
        return;
    }
    QJsonObject rootObject = jsonDoc.object();
    QJsonValue platformsValue = rootObject.value("Platforms");
    if(!platformsValue.isObject())
    {
        err = tr("Platforms section not found in file %1").arg(configPath);
    }


}

