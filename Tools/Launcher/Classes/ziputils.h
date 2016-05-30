#ifndef __LAUNCHER_ZIP_UTILS_H__
#define __LAUNCHER_ZIP_UTILS_H__

//this is private header
#include <QObject>
#include <functional>

class ZipError
{
public:
    enum eZipListError
    {
        NO_ERRORS, //NO_ERROR is taken by winerror.h
        FILE_NOT_EXISTS,
        NOT_AN_ARCHIVE,
        ARHIVE_DAMAGED,
        ARCHIVER_NOT_FOUND,
        PROCESS_FAILED_TO_START,
        PROCESS_FAILED_TO_FINISH,
        PARSE_ERROR,
        ARCHIVE_IS_EMPTY,
        OUT_DIRECTORY_NOT_EXISTS
    };
    ZipError(eZipListError errCode = NO_ERRORS)
        : error(errCode)
    {
    }
    eZipListError error = NO_ERRORS;
    QString GetErrorString() const;
};

namespace ZipUtils
{
class ZipOperationFunctor
{
public:
    virtual ~ZipOperationFunctor() = default;

    virtual void OnStart(){};
    virtual void OnProgress(int){};
    virtual void OnSuccess(){};
    virtual void OnError(const ZipError&){};
};

using ReadyReadCallback = std::function<void(const QByteArray&)>;
using CompressedFilesAndSizes = QMap<QString, qint64>;

QString GetArchiverPath();
bool IsArchiveValid(const QString& archivePath, ZipError* err = nullptr);
bool LaunchArchiver(const QStringList& arguments, ReadyReadCallback callback = ReadyReadCallback(), ZipError* err = nullptr);
bool GetFileList(const QString& archivePath, CompressedFilesAndSizes& files, ZipOperationFunctor& functor);
bool TestZipArchive(const QString& archivePath, const CompressedFilesAndSizes& files, ZipOperationFunctor& functor);
bool UnpackZipArchive(const QString& archivePath, const QString& outDir, const CompressedFilesAndSizes& files, ZipOperationFunctor& onProgress);
}

#endif // __LAUNCHER_ZIP_LIST_H__
