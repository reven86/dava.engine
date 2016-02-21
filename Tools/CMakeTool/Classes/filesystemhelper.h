#ifndef FILESYSTEMHELPER_H
#define FILESYSTEMHELPER_H

#include <QObject>
#include <QVariant>

class FileSystemHelper : public QObject
{
    Q_OBJECT
    
public:
    explicit FileSystemHelper(QObject *parent = 0);
    Q_INVOKABLE QVariant resolveUrl(QVariant url);
    Q_INVOKABLE QVariant isDirExists(QVariant dirPath);
};

#endif // FILESYSTEMHELPER_H
