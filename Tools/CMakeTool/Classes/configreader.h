#ifndef CONFIGREADER_H
#define CONFIGREADER_H

#include <QObject>

class ConfigReader : public QObject
{
    Q_OBJECT
public:
    explicit ConfigReader(QObject *parent = 0);
    void ReadConfig(const QString &configPath, QString &err);

};

#endif // CONFIGREADER_H
