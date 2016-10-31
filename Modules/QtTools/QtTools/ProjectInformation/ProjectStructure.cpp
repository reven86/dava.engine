#include "QtTools/ProjectInformation/ProjectStructure.h"

#include "Debug/DVAssert.h"
#include "FileSystem/FilePath.h"

#include <QFileInfo>
#include <QDirIterator>
#include <QFile>
#include <QDir>
#include <QSet>
#include <QFileSystemWatcher>
#include <memory>

using namespace DAVA;

//this function is reimplemented for QSet<QFileInfo> in the class Impl
uint qHash(const QFileInfo& fi, uint seed)
{
    return qHash(fi.absoluteFilePath().toLower(), seed);
}

class ProjectStructure::Impl : public QObject
{
public:
    Impl(const QStringList& supportedExtensions);
    void AddProjectDirectory(const QString& directory);
    void RemoveProjectDirectory(const QString& directory);
    void RemoveAllProjectDirectories();

    QStringList GetProjectDirectories() const;

    QStringList GetFiles(const QString& extension) const;

private slots:
    void OnDirChanged(const QString& path);
    void OnFileChanged(const QString& path);

private:
    std::tuple<QStringList, QSet<QFileInfo>> CollectFilesAndDirectories(const QDir& dir) const;
    void AddFilesRecursively(const QDir& dir);

    bool ForceRemovePaths(const QStringList& directories);

    std::unique_ptr<QFileSystemWatcher> watcher;
    QSet<QFileInfo> projectFiles;

    QStringList supportedExtensions;
    QStringList projectDirectories;
};

ProjectStructure::ProjectStructure(const QStringList& supportedExtensions)
    : impl(new Impl(supportedExtensions))
{
}

ProjectStructure::~ProjectStructure() = default;

void ProjectStructure::AddProjectDirectory(const QString& directory)
{
    impl->AddProjectDirectory(directory);
}

void ProjectStructure::RemoveProjectDirectory(const QString& directory)
{
    impl->RemoveProjectDirectory(directory);
}

void ProjectStructure::RemoveAllProjectDirectories()
{
    impl->RemoveAllProjectDirectories();
}

QStringList ProjectStructure::GetProjectDirectories() const
{
    return impl->GetProjectDirectories();
}

QStringList ProjectStructure::GetFiles(const QString& extension) const
{
    return impl->GetFiles(extension);
}

ProjectStructure::Impl::Impl(const QStringList& supportedExtensions_)
    : QObject(nullptr)
    , watcher(new QFileSystemWatcher(this))
{
    QObject::connect(watcher.get(), &QFileSystemWatcher::fileChanged, this, &Impl::OnFileChanged);
    QObject::connect(watcher.get(), &QFileSystemWatcher::directoryChanged, this, &Impl::OnDirChanged);
    for (const QString& extension : supportedExtensions_)
    {
        supportedExtensions << extension.toLower();
    }
}

void ProjectStructure::Impl::AddProjectDirectory(const QString& directory)
{
    DVASSERT(!directory.isEmpty());
    if (directory.isEmpty())
    {
        return;
    }

    bool alreadyAdded = projectDirectories.contains(directory);
    DVASSERT(!alreadyAdded);
    if (alreadyAdded)
    {
        return;
    }

    QFileInfo fileInfo(directory);
    DVASSERT(fileInfo.exists() && fileInfo.isDir());
    DVASSERT(!watcher->directories().contains(directory));
    projectDirectories << directory;

    QStringList subDirectories;
    QSet<QFileInfo> filesInDirectory;

    std::tie(subDirectories, filesInDirectory) = CollectFilesAndDirectories(QDir(directory));

    watcher->addPaths(subDirectories << directory);
    projectFiles += filesInDirectory;

    //projectDirectories = QDir();
    //we delete watcher because "remove paths" us not work. It was reproduced in DF-11828
    //delete watcher;
    //watcher = nullptr;
    //     if (!directory.IsEmpty())
    //     {
    //         watcher = new QFileSystemWatcher(this);
    //         QObject::connect(watcher, &QFileSystemWatcher::fileChanged, this, &Impl::OnFileChanged);
    //         QObject::connect(watcher, &QFileSystemWatcher::directoryChanged, this, &Impl::OnDirChanged);

    //         QString directoryStr = QString::fromStdString(directory.GetAbsolutePathname());
    //         projectDir = QDir(directoryStr);
    //         DVASSERT(projectDir.exists());
    //         DVASSERT(!watcher->directories().contains(directoryStr))
    //watcher->addPath(directoryStr);
    //AddFilesRecursively(QDir(directory));
    //    }
}

void ProjectStructure::Impl::RemoveProjectDirectory(const QString& directory)
{
    DVASSERT(!directory.isEmpty());
    if (directory.isEmpty())
    {
        return;
    }

    bool alreadyAdded = projectDirectories.contains(directory);
    DVASSERT(alreadyAdded);
    if (!alreadyAdded)
    {
        return;
    }

    QStringList subDirectories;
    QSet<QFileInfo> filesInDirectory;

    std::tie(subDirectories, filesInDirectory) = CollectFilesAndDirectories(QDir(directory));

    projectDirectories.removeOne(directory);
    ForceRemovePaths(subDirectories << directory);
    projectFiles -= filesInDirectory;
}

void ProjectStructure::Impl::RemoveAllProjectDirectories()
{
    ForceRemovePaths(watcher->directories());
    projectDirectories.clear();
    projectFiles.clear();
}

QStringList ProjectStructure::Impl::GetProjectDirectories() const
{
    //QString absPath = projectDir.absolutePath();
    //return FilePath(absPath.toStdString());
    return projectDirectories;
}

QStringList ProjectStructure::Impl::GetFiles(const QString& extension) const
{
    QStringList files;
    for (const QFileInfo& fileInfo : projectFiles)
    {
        if (fileInfo.suffix().toLower() == extension.toLower())
        {
            files << fileInfo.absoluteFilePath();
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
    DVASSERT(watcher != nullptr);
    QDir changedDir(path);
    QMutableSetIterator<QFileInfo> iter(projectFiles); //TODO check this code
    while (iter.hasNext())
    {
        QFileInfo fileInfo = iter.next();
        QString absoluteFilePath = fileInfo.absoluteFilePath();
        if (absoluteFilePath.startsWith(path) && !QFile::exists(absoluteFilePath))
        {
            iter.remove();
        }
    }

    QStringList watchedDirectories = watcher->directories();
    for (const QString& dirPath : watchedDirectories)
    {
        QFileInfo fileInfo(dirPath);
        if (!fileInfo.isDir())
        {
            watcher->removePath(dirPath);
        }
    }

    if (changedDir.exists())
    {
        QStringList subDirectories;
        QSet<QFileInfo> filesInDirectory;

        std::tie(subDirectories, filesInDirectory) = CollectFilesAndDirectories(changedDir);
        watcher->addPaths(subDirectories);

        projectFiles += filesInDirectory;
    }
}

void ProjectStructure::Impl::AddFilesRecursively(const QDir& dir)
{
    DVASSERT(dir.exists());
    DVASSERT(watcher != nullptr);
    QStringList watchedDirectories = watcher->directories();

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
            watcher->addPath(absFilePath);
            watchedDirectories.append(absFilePath);
        }
        if (fileInfo.isFile() && supportedExtensions.contains(fileInfo.suffix().toLower()))
        {
            projectFiles.insert(fileInfo);
        }
    }
}

std::tuple<QStringList, QSet<QFileInfo>> ProjectStructure::Impl::CollectFilesAndDirectories(const QDir& dir) const
{
    DVASSERT(dir.exists());

    QStringList directoriesList;
    QSet<QFileInfo> filesList;
    QString absDirPath(dir.absolutePath());
    QDirIterator dirIterator(absDirPath, QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files | QDir::Hidden, QDirIterator::Subdirectories);
    while (dirIterator.hasNext())
    {
        dirIterator.next();
        QString tmp = dirIterator.filePath();
        QFileInfo fileInfo(dirIterator.fileInfo());

        if (fileInfo.isDir())
        {
            directoriesList << fileInfo.absoluteFilePath();
        }

        if (fileInfo.isFile() && supportedExtensions.contains(fileInfo.suffix().toLower()))
        {
            filesList.insert(fileInfo);
        }
    }

    return std::make_tuple(directoriesList, filesList);
}

bool ProjectStructure::Impl::ForceRemovePaths(const QStringList& directories)
{
    bool result = true;
    for (const QString& directory : directories)
    {
        result &= watcher->removePath(directory);
    }

    return result;
    //     QStringList watchingDirectories = watcher->directories();
    //     watchingDirectories.remo
    //     watcher->removePaths(directories);
}
