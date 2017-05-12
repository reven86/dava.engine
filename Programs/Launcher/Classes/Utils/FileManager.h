#pragma once

#include "Core/Tasks/BaseTask.h"

#include <QString>
#include <QObject>
#include <QFileInfo>
#include <QPair>

class FileManager : public QObject
{
    Q_OBJECT
public:
    FileManager(QObject* parent = nullptr);

    QString GetBaseAppsDirectory() const;
    QString GetTempDirectory() const;
    QString GetSelfUpdateTempDirectory() const;

    QString GetTempDownloadFilePath(const QString& url) const;

    //name CreateFile is already taken by windows
    bool CreateFileFromRawData(const QByteArray& dataToWrite, const QString& filePath) const;

    QString GetApplicationDirectory(const QString& branchID, const QString& appID) const;
    QString GetBranchDirectory(const QString& branchID) const;

    //name MoveFile is already taken by windows
    bool MoveFileWithMakePath(const QString& currentPath, const QString& newPath);

    //this function move all files and folder except folders, which created by Launcher
    bool MoveLauncherRecursively(const QString& pathOut, const QString& pathIn, ErrorHolder* result) const;

    QString GetFilesDirectory() const;

    QString GetLauncherDirectory() const;

    static QString GetFileNameFromURL(const QString& url);
    static QString GetDocumentsDirectory();
    static bool DeleteDirectory(const QString& path);
    static void MakeDirectory(const QString& path);

public slots:
    void SetFilesDirectory(const QString& newDirPath);

signals:
    void FilesDirPathChanged(const QString& oldPath, const QString& newPath);

private:
    using EntireList = QList<QPair<QFileInfo, QString>>;

    QString GetDefaultFilesDirectory() const;

    QStringList OwnDirectories() const;
    EntireList CreateEntireList(const QString& pathOut, const QString& pathIn, ErrorHolder* result) const;

    QString filesDirectory;
};
