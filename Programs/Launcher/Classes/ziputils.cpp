#include "ziputils.h"
#include "filemanager.h"
#include <QString>
#include <QProcess>
#include <QFile>
#include <QRegularExpression>
#include <QEventLoop>
#include <QDir>
#include <QApplication>
#include <numeric>
#include <QDebug>

QString ZipUtils::GetArchiverPath()
{
    static QString processAddr = qApp->applicationDirPath() +
#if defined(Q_OS_WIN)
    "/7z.exe";
#elif defined Q_OS_MAC
    "/../Resources/7za";
#endif //Q_OS_MAC Q_OS_WIN
    return processAddr;
}

QString ZipError::GetErrorString() const
{
    switch (error)
    {
    case NO_ERRORS:
        return QObject::tr("No errors occurred");
    case FILE_NOT_EXISTS:
        return QObject::tr("Required archive is not existing");
    case NOT_AN_ARCHIVE:
        return QObject::tr("Required file is not a zip archive");
    case ARHIVE_DAMAGED:
        return QObject::tr("Archive is corrupted");
    case ARCHIVER_NOT_FOUND:
        return QObject::tr("Archiver tool was not found. Please, reinstall application");
    case PROCESS_FAILED_TO_START:
        return QObject::tr("Failed to launch archiver");
    case PROCESS_CRASHED:
        return QObject::tr("Archiver crashed");
    case PROCESS_TIMEDOUT:
        return QObject::tr("Archiver time out error");
    case PROCESS_READ_ERROR:
        return QObject::tr("Failed to read output from archiver");
    case PROCESS_WRITE_ERROR:
        return QObject::tr("Failed to write to archiver");
    case PROCESS_UNKNOWN_ERROR:
        return QObject::tr("Archiver crashed with an unknown error");
    case PROCESS_FAILED:
        return QObject::tr("Archiver reports about error in current archive");
    case PARSE_ERROR:
        return QObject::tr("Unknown format of archiver output");
    case ARCHIVE_IS_EMPTY:
        return QObject::tr("Archive is empty!");
    case OUT_DIRECTORY_NOT_EXISTS:
        return QObject::tr("Output directory is not existing!");
    default:
        Q_ASSERT(false && "invalid condition passed to GetErrorString");
        return QString();
    }
}

namespace ZipUtilsDetails
{
ZipError* GetDefaultZipError()
{
    static ZipError localError;
    localError.error = ZipError::NO_ERRORS; //prevouis requester can break state of this varaiable
    return &localError;
}

void ApplyProcessErrorToZipError(ZipError* zipError, QProcess::ProcessError processError)
{
    switch (processError)
    {
    case QProcess::FailedToStart:
        zipError->error = ZipError::PROCESS_FAILED_TO_START;
        break;
    case QProcess::Crashed:
        zipError->error = ZipError::PROCESS_CRASHED;
        break;
    case QProcess::Timedout:
        zipError->error = ZipError::PROCESS_TIMEDOUT;
        break;
    case QProcess::ReadError:
        zipError->error = ZipError::PROCESS_READ_ERROR;
        break;
    case QProcess::WriteError:
        zipError->error = ZipError::PROCESS_WRITE_ERROR;
        break;
    default:
        zipError->error = ZipError::PROCESS_UNKNOWN_ERROR;
        break;
    }
}
} //namespace ZipUtilsDetails

bool ZipUtils::IsArchiveValid(const QString& archivePath, ZipError* err)
{
    if (err == nullptr)
    {
        err = ZipUtilsDetails::GetDefaultZipError();
    }
    QString processAddr = GetArchiverPath();
    if (!QFile::exists(processAddr))
    {
        err->error = ZipError::ARCHIVER_NOT_FOUND;
        return false;
    }
    if (!archivePath.endsWith(".zip"))
    {
        err->error = ZipError::NOT_AN_ARCHIVE;
        return false;
    }
    if (!QFile::exists(archivePath))
    {
        err->error = ZipError::FILE_NOT_EXISTS;
        return false;
    }
    return true;
}

bool ZipUtils::LaunchArchiver(const QStringList& arguments, ReadyReadCallback callback, ZipError* err)
{
    if (err == nullptr)
    {
        err = ZipUtilsDetails::GetDefaultZipError();
    }
    Q_ASSERT(err->error == ZipError::NO_ERRORS);

    QString processAddr = ZipUtils::GetArchiverPath();
    QProcess zipProcess;
    QObject::connect(&zipProcess, &QProcess::readyReadStandardOutput, [&zipProcess, callback, err]() {
        while (zipProcess.canReadLine())
        {
            QByteArray line = zipProcess.readLine();
            callback(line);
            if (err->error != ZipError::NO_ERRORS) //callback can produce errors
            {
                zipProcess.kill();
                return;
            }
        }
    });
    QEventLoop eventLoop;
    QObject::connect(&zipProcess, static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), [&eventLoop, err](int exitCode, QProcess::ExitStatus exitStatus) {
        if (err->error == ZipError::NO_ERRORS && (exitCode != 0 || exitStatus != QProcess::NormalExit))
        {
            err->error = err->PROCESS_FAILED;
        }
        eventLoop.quit();
    });
    QObject::connect(&zipProcess, &QProcess::errorOccurred, [&eventLoop, err](QProcess::ProcessError error) {
        ZipUtilsDetails::ApplyProcessErrorToZipError(err, error);
        eventLoop.quit();
    });

    zipProcess.start(processAddr, arguments);
    eventLoop.exec();

    return err->error == ZipError::NO_ERRORS;
}

bool ZipUtils::GetFileList(const QString& archivePath, CompressedFilesAndSizes& fileList, ZipOperationFunctor& functor)
{
    ZipError err;
    if (!IsArchiveValid(archivePath, &err))
    {
        functor.OnError(err);
        return false;
    }
    fileList.clear();
    bool foundOutputData = false;
    ReadyReadCallback callback = [&foundOutputData, &fileList, &functor](const QByteArray& line) {
        if (line.startsWith("----------")) //this string occurrs two times: before file list and at the and of file list
        {
            foundOutputData = !foundOutputData;
            return;
        }
        if (!foundOutputData)
        {
            return;
        }
        QString str(line);

        const int SIZE_INDEX = 26; //fixed index for size
        const int SIZE_STR_LEN = 12; //fixed index for len
        bool ok = true;
        QString sizeStr = str.mid(SIZE_INDEX, SIZE_STR_LEN);
        sizeStr.remove(QRegularExpression("\\s*"));
        qint64 size = sizeStr.toULongLong(&ok);
        if (!ok)
        {
            functor.OnError(ZipError::PARSE_ERROR);
            return;
        }

        const int NAME_INDEX = 53; //fixed index for name
        if (str.length() <= NAME_INDEX)
        {
            functor.OnError(ZipError::PARSE_ERROR);
            return;
        }
        QString file = str.right(str.size() - NAME_INDEX);
        Q_ASSERT(!fileList.contains(file));
        fileList[file] = size;
    };
    if (!LaunchArchiver(QStringList() << "l" << archivePath, callback, &err))
    {
        functor.OnError(err);
        return false;
    }
    if (fileList.empty())
    {
        functor.OnError(ZipError::ARCHIVE_IS_EMPTY);
        return false;
    }
    functor.OnSuccess();
    return true;
}

bool ZipUtils::UnpackZipArchive(const QString& archivePath, const QString& outDirPath, const CompressedFilesAndSizes& files, ZipOperationFunctor& functor)
{
    ZipError err;
    if (!IsArchiveValid(archivePath, &err))
    {
        functor.OnError(err);
        return false;
    }
    QDir outDir(outDirPath);
    if (!outDir.mkpath("."))
    {
        functor.OnError(ZipError::OUT_DIRECTORY_NOT_EXISTS);
        return false;
    }

    bool success = false;
    qint64 matchedSize = 0;
    const auto values = files.values();
    qint64 totalSize = std::accumulate(values.begin(), values.end(), 0);
    ReadyReadCallback callback = [&success, &functor, &files, &matchedSize, totalSize](const QByteArray& line) {
        QString str(line);
        QString okStr = "Everything is Ok";
        if (str.contains(okStr))
        {
            success = true;
        }
        QString startStr("- ");
        if (!str.startsWith(startStr))
        {
            return;
        }
        str.remove(0, startStr.length());
        if (files.contains(str))
        {
            matchedSize += files[str];
        }
        float progress = (matchedSize * 100.0f) / totalSize;
        int progressInt = static_cast<int>(qRound(progress));
        functor.OnProgress(progressInt);
    };
    QStringList arguments;
    //this is needed for correct work with pathes contains whitespace
    QString nativeOutPath = QDir::toNativeSeparators(outDirPath);
    arguments << "x"
              << "-y"
              << "-bb1"
              << archivePath
              << "-o" + nativeOutPath;

    if (!LaunchArchiver(arguments, callback, &err))
    {
        functor.OnError(err);
        return false;
    }
    if (success != true)
    {
        functor.OnError(ZipError::ARHIVE_DAMAGED);
        return false;
    }
    functor.OnSuccess();
    return true;
}
