#include "QtTools/ProjectInformation/ProjectStructure.h"

#include "Debug/DVAssert.h"
#include "FileSystem/FilePath.h"

#include <QFileInfo>
#include <QDirIterator>
#include <QFile>
#include <QDir>
#include <QSet>
#include <QFileSystemWatcher>

using namespace DAVA;

//this function is reimplemented for QSet<QFileInfo> in the class Impl
uint qHash(const QFileInfo& fi, uint seed)
{
    return qHash(fi.absoluteFilePath(), seed);
}

struct ProjectStructure::Impl : public QObject
{
    Impl();
    void AddProjectDirectory(const FilePath& directory);
    void ClearProjectDirectories();

    Vector<FilePath> GetFiles(const String& extension) const;

private slots:
    void OnDirChanged(const QString& path);
    void OnFileChanged(const QString& path);

private:
    void AddFilesRecursively(const QFileInfo& dirInfo);

    QSet<QFileInfo> projectFiles;
    QFileSystemWatcher watcher;
};

ProjectStructure::ProjectStructure()
    : impl(new Impl())
{
}

ProjectStructure::~ProjectStructure() = default;

void ProjectStructure::AddProjectDirectory(const FilePath& directory)
{
    impl->AddProjectDirectory(directory);
}

void ProjectStructure::ClearProjectDirectories()
{
    impl->ClearProjectDirectories();
}

DAVA::Vector<DAVA::FilePath> ProjectStructure::GetFiles(const DAVA::String& extension) const
{
    return impl->GetFiles(extension);
}

ProjectStructure::Impl::Impl()
    : QObject(nullptr)
{
    QObject::connect(&watcher, &QFileSystemWatcher::fileChanged, this, &Impl::OnFileChanged);
    QObject::connect(&watcher, &QFileSystemWatcher::directoryChanged, this, &Impl::OnDirChanged);
}

void ProjectStructure::Impl::AddProjectDirectory(const FilePath& directory)
{
    DVASSERT(!directory.IsEmpty());
    QString directoryStr = QString::fromStdString(directory.GetAbsolutePathname());
    QFileInfo projectDirFileInfo(directoryStr);
    DVASSERT(projectDirFileInfo.isDir());
    if (!watcher.directories().contains(directoryStr))
    {
        watcher.addPath(directoryStr);
        AddFilesRecursively(projectDirFileInfo);
    }
}

void ProjectStructure::Impl::ClearProjectDirectories()
{
    projectFiles.clear();
    const QStringList& directories = watcher.directories();
    if (!directories.isEmpty())
    {
        watcher.removePaths(directories);
    }
    DVASSERT(watcher.files().isEmpty());
}

Vector<FilePath> ProjectStructure::Impl::GetFiles(const String& extension) const
{
    Vector<FilePath> files;
    QString extensionStr = QString::fromStdString(extension);
    for (const QFileInfo& fileInfo : projectFiles)
    {
        if (fileInfo.suffix() == extensionStr)
        {
            files.push_back(fileInfo.absoluteFilePath().toStdString());
        }
    }
    return files;
}

void ProjectStructure::Impl::OnFileChanged(const QString& path)
{
    QFileInfo changedFileInfo(path);
    DVASSERT(changedFileInfo.isFile());
    if (changedFileInfo.exists())
    {
        projectFiles.insert(changedFileInfo);
    }
    else
    {
        projectFiles.remove(changedFileInfo);
    }
}

void ProjectStructure::Impl::OnDirChanged(const QString& path)
{
    QFileInfo changedDirInfo(path);
    DVASSERT(changedDirInfo.isDir());
    QMutableSetIterator<QFileInfo> iter(projectFiles);
    while (iter.hasNext())
    {
        QFileInfo fileInfo = iter.next();
        QString absoluteFilePath = fileInfo.absoluteFilePath();
        if (absoluteFilePath.startsWith(path) && !QFile::exists(absoluteFilePath))
        {
            iter.remove();
        }
    }

    QStringList watchedDirectories = watcher.directories();
    for (const QString& dirPath : watchedDirectories)
    {
        QFileInfo fileInfo(dirPath);
        if (!fileInfo.isDir())
        {
            watcher.removePath(dirPath);
        }
    }

    if (changedDirInfo.exists())
    {
        AddFilesRecursively(changedDirInfo);
    }
}

void ProjectStructure::Impl::AddFilesRecursively(const QFileInfo& dirInfo)
{
    DVASSERT(dirInfo.isDir());
    QStringList watchedDirectories = watcher.directories();

    QString absDirPath(dirInfo.absoluteFilePath());
    QDirIterator dirIterator(absDirPath, QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
    while (dirIterator.hasNext())
    {
        dirIterator.next();
        QString tmp = dirIterator.filePath();
        QFileInfo fileInfo(dirIterator.fileInfo());
        QString absFilePath = fileInfo.absoluteFilePath();
        if (fileInfo.isDir() && !watchedDirectories.contains(absFilePath))
        {
            watcher.addPath(absFilePath);
            watchedDirectories.append(absFilePath);
        }
        if (fileInfo.isFile())
        {
            projectFiles.insert(fileInfo);
        }
    }
}
