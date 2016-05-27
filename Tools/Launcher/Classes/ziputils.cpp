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
    case PROCESS_FAILED_TO_FINISH:
        return QObject::tr("Archiver failed to get list of files from archive");
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

namespace ZIP_UTILS_LOCAL
{
ZipError* GetDefaultZipError()
{
    static ZipError localError;
    localError.error = ZipError::NO_ERRORS; //prevouis requester can break state of this varaiable
    return &localError;
}
}

bool ZipUtils::IsArchiveValid(const QString& archivePath, ZipError* err)
{
    if (err == nullptr)
    {
        err = ZIP_UTILS_LOCAL::GetDefaultZipError();
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
        err = ZIP_UTILS_LOCAL::GetDefaultZipError();
    }
    Q_ASSERT(err->error == ZipError::NO_ERRORS);

    QString processAddr = ZipUtils::GetArchiverPath();
    QProcess zipProcess;
    QObject::connect(&zipProcess, &QProcess::readyReadStandardOutput, [&zipProcess, callback, err]() {
        while (zipProcess.canReadLine())
        {
            callback(zipProcess.readLine());
            if (err->error != ZipError::NO_ERRORS) //callback can produce errors
            {
                zipProcess.kill();
                return;
            }
        }
    });
    QEventLoop loop;
    QObject::connect(&zipProcess, static_cast<void (QProcess::*)(int)>(&QProcess::finished), &loop, &QEventLoop::quit, Qt::QueuedConnection);

    zipProcess.start(processAddr, arguments);
    if (!zipProcess.waitForStarted(5000))
    {
        err->error = ZipError::PROCESS_FAILED_TO_START;
        return false;
    }
    loop.exec();
    if (zipProcess.exitStatus() == QProcess::CrashExit)
    {
        err->error = ZipError::PROCESS_FAILED_TO_FINISH;
        return false;
    }
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
    QFile file("test2.txt");
    file.open(QFile::WriteOnly | QFile::Truncate);
    qint64 totalSize = std::accumulate(values.begin(), values.end(), 0);
    ReadyReadCallback callback = [&success, &functor, &file, &files, &matchedSize, totalSize](const QByteArray& line) {
        QString str(line);
        file.write(line + "\n");
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

        functor.OnProgress((matchedSize * 100.0f) / totalSize);
    };
    QStringList arguments;
    QString nativeOutPath = QDir::toNativeSeparators(outDirPath);
    arguments << "x"
              << "-y"
              << "-bb1"
              << archivePath
              << "-o" + nativeOutPath;

    QString testStr = arguments.join(' ');
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
