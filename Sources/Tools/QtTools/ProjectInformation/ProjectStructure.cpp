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

class ProjectStructure::Impl : public QObject
{
public:
    Impl(const Vector<String>& supportedExtensions);
    void SetProjectDirectory(const FilePath& directory);
    FilePath GetProjectDirectory() const;

    Vector<FilePath> GetFiles(const String& extension) const;

private slots:
    void OnDirChanged(const QString& path);
    void OnFileChanged(const QString& path);

private:
    void AddFilesRecursively(const QDir& dir);

    QSet<QFileInfo> projectFiles;
    QFileSystemWatcher watcher;
    QStringList supportedExtensions;
    QDir projectDir;
};

ProjectStructure::ProjectStructure(const Vector<String>& supportedExtensions)
    : impl(new Impl(supportedExtensions))
{
}

ProjectStructure::~ProjectStructure() = default;

void ProjectStructure::SetProjectDirectory(const FilePath& directory)
{
    impl->SetProjectDirectory(directory);
}

FilePath ProjectStructure::GetProjectDirectory() const
{
    return impl->GetProjectDirectory();
}

DAVA::Vector<DAVA::FilePath> ProjectStructure::GetFiles(const DAVA::String& extension) const
{
    return impl->GetFiles(extension);
}

ProjectStructure::Impl::Impl(const Vector<String>& supportedExtensions_)
    : QObject(nullptr)
{
    QObject::connect(&watcher, &QFileSystemWatcher::fileChanged, this, &Impl::OnFileChanged);
    QObject::connect(&watcher, &QFileSystemWatcher::directoryChanged, this, &Impl::OnDirChanged);
    for (const String& extension : supportedExtensions_)
    {
        supportedExtensions << QString::fromStdString(extension).toLower();
    }
}

void ProjectStructure::Impl::SetProjectDirectory(const FilePath& directory)
{
    projectDir = QDir();
    projectFiles.clear();
    const QStringList& directories = watcher.directories();
    if (!directories.isEmpty())
    {
        watcher.removePaths(directories);
    }
    DVASSERT(watcher.files().isEmpty());

    if (!directory.IsEmpty())
    {
        QString directoryStr = QString::fromStdString(directory.GetAbsolutePathname());
        projectDir = QDir(directoryStr);
        DVASSERT(projectDir.exists());
        DVASSERT(!watcher.directories().contains(directoryStr))

        watcher.addPath(directoryStr);
        AddFilesRecursively(projectDir);
    }
}

FilePath ProjectStructure::Impl::GetProjectDirectory() const
{
    QString absPath = projectDir.absolutePath();
    return FilePath(absPath.toStdString());
}

Vector<FilePath> ProjectStructure::Impl::GetFiles(const String& extension) const
{
    Vector<FilePath> files;
    QString extensionStr = QString::fromStdString(extension);
    for (const QFileInfo& fileInfo : projectFiles)
    {
        if (fileInfo.suffix().toLower() == extensionStr.toLower())
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
    QDir changedDir(path);
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

    if (changedDir.exists())
    {
        AddFilesRecursively(changedDir);
    }
}

void ProjectStructure::Impl::AddFilesRecursively(const QDir& dir)
{
    DVASSERT(dir.exists());
    QStringList watchedDirectories = watcher.directories();

    QString absDirPath(dir.absolutePath());
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
        if (fileInfo.isFile() && supportedExtensions.contains(fileInfo.suffix().toLower()))
        {
            projectFiles.insert(fileInfo);
        }
    }
}
