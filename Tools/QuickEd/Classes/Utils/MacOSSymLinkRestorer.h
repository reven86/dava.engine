#pragma once

#include <QString>
#include <QVector>

class MacOSSymLinkRestorer
{
public:
    MacOSSymLinkRestorer(const QString& directory);

    QString RestoreSymLinkInFilePath(const QString& origFilePath) const;

private:
    QVector<QPair<QString, QString>> symlinks; // first: real path, second: symbolic link
};