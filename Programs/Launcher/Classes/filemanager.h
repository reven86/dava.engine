#pragma once

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
    QString GetLauncherDirectory() const;
    QString GetSelfUpdateTempDirectory() const;
    QString GetTempDownloadFilePath() const;
    QString GetApplicationDirectory(const QString& branchID, const QString& appID) const;
    QString GetBranchDirectory(const QString& branchID) const;
    //this function move all files and folder except folders, which created by Launcher
    bool MoveLauncherRecursively(const QString& pathOut, const QString& pathIn) const;

    QString GetFilesDirectory() const;

    static QString GetDocumentsDirectory();
    static bool CreateFileAndWriteData(const QString& filePath, const QByteArray& data);
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
    EntireList CreateEntireList(const QString& pathOut, const QString& pathIn) const;

    QString filesDirectory;
};
