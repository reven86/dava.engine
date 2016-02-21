#ifndef CONFIGSTORAGE_H
#define CONFIGSTORAGE_H

#include <QObject>
#include <QString>

class ConfigStorage : public QObject
{
    Q_OBJECT
    
public:
    explicit ConfigStorage(QObject *parent = 0);
    Q_INVOKABLE QString GetJSONTextFromConfigFile() const;
    Q_INVOKABLE void SaveToConfigFile(QString config);

private:
    QString configFilePath;
};


#endif // CONFIGSTORAGE_H
