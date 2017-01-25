#pragma once

#include <QStringList>
#include <memory>

class FileSystemCache final
{
public:
    FileSystemCache(const QStringList& supportedExtensions);
    ~FileSystemCache();

    void TrackDirectory(const QString& directory);
    void UntrackDirectory(const QString& directory);
    void UntrackAllDirectories();
    const QStringList& GetTrackedDirectories() const;

    QStringList GetFiles(const QString& extension) const;

private:
    class Impl;
    std::unique_ptr<Impl> impl;
};
