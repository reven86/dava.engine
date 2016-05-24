#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <QString>

class FileManager
{
public:
    static FileManager* Instance();

    const QString& GetDocumentsDirectory();
    const QString& GetBaseAppsDirectory();
    const QString& GetTempDirectory();
    const QString& GetLauncherDirectory();
    const QString& GetSelfUpdateTempDirectory();

    const QString& GetTempDownloadFilepath();

    bool DeleteDirectory(const QString& path);
    void ClearTempDirectory();

    void MakeDirectory(const QString& path);

    void MoveFilesOnlyToDirectory(const QString& dirFrom, const QString& dirTo);
    //directories path must be with '/' at the end

    QString GetApplicationFolder(const QString& branchID, const QString& appID);
    QString GetBranchFolder(const QString& branchID);

private:
    FileManager();

    static FileManager* instance;

    QString docDir;
    QString launcherDir;
    QString baseAppDir;
    QString tempDir;
    QString tempSelfUpdateDir;
    QString tempFile;
};

#endif // FILEMANAGER_H
