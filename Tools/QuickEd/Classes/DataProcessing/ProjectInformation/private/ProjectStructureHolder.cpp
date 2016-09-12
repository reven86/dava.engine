#include "DataProcessing/ProjectInformation/ProjectStructureHolder.h"

#include "Debug/DVAssert.h"

#include <QFileInfo>
#include <QDirIterator>
#include <QFile>
#include <QDir>
#include <QSet>
#include <QFileSystemWatcher>

using namespace DAVA;

uint qHash(const QFileInfo& fi, uint seed)
{
    return qHash(fi.absoluteFilePath(), seed);
}

struct ProjectStructureHolder::Impl : public QObject
{
    Impl();
    void SetProjectDirectory(const String& directory);
    Vector<String> GetProjectYamlFiles() const;
    String GetProjectDirectory() const;

private slots:
    void OnDirChanged(const QString& path);
    void OnFileChanged(const QString& path);

private:
    QString YamlSuffix() const;
    void AddFilesRecursively(const QFileInfo& dirInfo);

    QFileInfo projectDirFileInfo;
    QSet<QFileInfo> yamlFiles;
    QFileSystemWatcher watcher;
};

ProjectStructureHolder::ProjectStructureHolder()
    : impl(new Impl())
{
}

ProjectStructureHolder::~ProjectStructureHolder() = default;

void ProjectStructureHolder::SetProjectDirectory(const String& directory)
{
    impl->SetProjectDirectory(directory);
}

Vector<String> ProjectStructureHolder::GetProjectYamlFiles() const
{
    return impl->GetProjectYamlFiles();
}

String ProjectStructureHolder::GetProjectDirectory() const
{
    return impl->GetProjectDirectory();
}

ProjectStructureHolder::Impl::Impl()
    : QObject(nullptr)
{
    QObject::connect(&watcher, &QFileSystemWatcher::fileChanged, this, &Impl::OnFileChanged);
    QObject::connect(&watcher, &QFileSystemWatcher::directoryChanged, this, &Impl::OnDirChanged);
}

void ProjectStructureHolder::Impl::SetProjectDirectory(const String& directory)
{
    projectDirFileInfo = QFileInfo();
    yamlFiles.clear();

    const QStringList& directories = watcher.directories();
    if (!directories.isEmpty())
    {
        watcher.removePaths(directories);
    }
    DVASSERT(watcher.files().isEmpty());

    if (directory.empty())
    {
        return;
    }

    QString directoryStr = QString::fromStdString(directory);
    projectDirFileInfo = QFileInfo(directoryStr);
    DVASSERT(projectDirFileInfo.isDir());

    watcher.addPath(directoryStr);

    AddFilesRecursively(projectDirFileInfo);
}

Vector<String> ProjectStructureHolder::Impl::GetProjectYamlFiles() const
{
    Vector<String> yamlFilesStrings;
    yamlFilesStrings.reserve(yamlFiles.size());
    for (const QFileInfo& fileInfo : yamlFiles)
    {
        yamlFilesStrings.push_back(fileInfo.absoluteFilePath().toStdString());
    }
    return yamlFilesStrings;
}

String ProjectStructureHolder::Impl::GetProjectDirectory() const
{
    QString filePath = projectDirFileInfo.absoluteFilePath();
    return projectDirFileInfo.absoluteFilePath().toStdString();
}

QString ProjectStructureHolder::Impl::YamlSuffix() const
{
    return "yaml";
}

void ProjectStructureHolder::Impl::OnFileChanged(const QString& path)
{
    QFileInfo changedFileInfo(path);
    DVASSERT(changedFileInfo.isFile());
    if (changedFileInfo.suffix() != YamlSuffix())
    {
        return;
    }
    if (changedFileInfo.exists())
    {
        yamlFiles.insert(changedFileInfo);
    }
    else
    {
        yamlFiles.remove(changedFileInfo);
    }
}

void ProjectStructureHolder::Impl::OnDirChanged(const QString& path)
{
    QFileInfo changedDirInfo(path);
    DVASSERT(changedDirInfo.isDir());
    if (changedDirInfo.exists())
    {
        AddFilesRecursively(changedDirInfo);
    }
    else
    {
        QMutableSetIterator<QFileInfo> iter(yamlFiles);
        while (iter.hasNext())
        {
            QFileInfo fileInfo = iter.next();
            if (fileInfo.absoluteFilePath().startsWith(path) && !fileInfo.exists())
            {
                iter.remove();
            }
        }
    }
}

void ProjectStructureHolder::Impl::AddFilesRecursively(const QFileInfo& dirInfo)
{
    DVASSERT(dirInfo.isDir());
    QDirIterator dirIterator(dirInfo.absoluteFilePath(), QDirIterator::Subdirectories);
    while (dirIterator.hasNext())
    {
        QFileInfo fileInfo(dirIterator.fileInfo());
        if (fileInfo.isFile() && fileInfo.suffix() == YamlSuffix())
        {
            yamlFiles.insert(fileInfo);
        }
        dirIterator.next();
    }
}
