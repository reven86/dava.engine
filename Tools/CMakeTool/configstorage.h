#ifndef CONFIGSTORAGE_H
#define CONFIGSTORAGE_H

#include <QObject>
#include <QString>

class ConfigStorage : public QObject
{
    Q_OBJECT
public:
    explicit ConfigStorage(const QString &configFilePath, QObject *parent = 0);
    QString GetJSONTextFromConfig() const;

public slots:
    void SaveJSONTestToConfig(QString config);

private:
    QString configFilePath;
};


#endif // CONFIGSTORAGE_H
