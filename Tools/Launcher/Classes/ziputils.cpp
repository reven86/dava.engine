/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "ziputils.h"
#include <QString>
#include <QProcess>
#include <QFile>
#include <QRegularExpression>
#include <QEventLoop>
#include <QDir>
#include <QApplication>
#include <numeric>

const QString &ZipUtils::GetArchiverPath()
{
    static QString processAddr = qApp->applicationDirPath() +
#if defined(Q_OS_WIN)
    "/7z.exe";
#elif defined Q_OS_MAC
    "/../Frameworks/7za";
#endif //Q_OS_MAC Q_OS_WIN
    return processAddr;
}

QString ZipError::GetErrorString() const
{
    switch (error)
    {
    case NO_ERRORS:
        return tr("No errors occurred");
    case FILE_NOT_EXISTS:
        return tr("Required archive not exists");
    case NOT_AN_ARCHIVE:
        return tr("Required file are not an zip archive");
    case ARHIVE_DAMAGED:
        return tr("Archive corrupted");
    case ARCHIVER_NOT_FOUND:\
        return tr("Archiver tool was not found. Reinstall application, please");
    case PROCESS_FAILED_TO_START:
        return tr("Failed to launch archiver");
    case PROCESS_FAILED_TO_FINISH:
        return tr("Archiver failed to get list of files from archive");
    case PARSE_ERROR:
        return tr("Unknown format of archiver output");
    case ARCHIVE_IS_EMPTY:
        return tr("Archive is empty!");
    case OUT_DIRECTORY_NOT_EXISTS:
        return tr("Output directory does not exist!");
    default:
        Q_ASSERT(false && "invalid condition passed to GetErrorString");
        return QString();
    }
}

namespace ZIP_UTILS_LOCAL
{
ZipError *GetDefaultZipError()
{
    static ZipError localError;
    localError.error = ZipError::NO_ERRORS; //prevouis requester can break state of this varaiable
    return &localError;
}
}

bool ZipUtils::IsArchiveValid(const QString &archivePath, ZipError *err)
{
    if(err == nullptr)
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

bool ZipUtils::LaunchArchiver(const QStringList &arguments, ReadyReadCallback callback, ZipError *err)
{
    if(err == nullptr)
    {
        err = ZIP_UTILS_LOCAL::GetDefaultZipError();
    }
    Q_ASSERT(err->error == ZipError::NO_ERRORS);

    QString processAddr = ZipUtils::GetArchiverPath();
    QProcess zipProcess;
    QObject::connect(&zipProcess, &QProcess::readyReadStandardOutput, [&zipProcess, callback, err]() {
        while(zipProcess.canReadLine())
        {
            callback(zipProcess.readLine());
            if(err->error != ZipError::NO_ERRORS) //callback can produce errors
            {
                zipProcess.kill();
                return;
            }
        }
    });
    QEventLoop loop;
    QObject::connect(&zipProcess, static_cast<void(QProcess::*)(int)>(&QProcess::finished), &loop, &QEventLoop::quit, Qt::QueuedConnection);
    zipProcess.start(processAddr, arguments);
    if(!zipProcess.waitForStarted(5000))
    {
        err->error = ZipError::PROCESS_FAILED_TO_START;
        return false;
    }
    loop.exec();
    if(zipProcess.exitStatus() == QProcess::CrashExit)
    {
        err->error = ZipError::PROCESS_FAILED_TO_FINISH;
        return false;
    }
    return err->error == ZipError::NO_ERRORS;
}


bool ZipUtils::GetFileList(const QString &archivePath, CompressedFilesAndSizes &fileList, ZipError *err)
{
    if (err == nullptr)
    {
        err = ZIP_UTILS_LOCAL::GetDefaultZipError();
    }
    if (!IsArchiveValid(archivePath, err))
    {
        return false;
    }
    QRegularExpression regExp("\\s+");
    bool foundOutputData = false;
    ReadyReadCallback callback = [&regExp, &foundOutputData, err, &fileList](const QByteArray &line){
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
        QStringList infoStringList = str.split(regExp, QString::SkipEmptyParts);
        const int SIZE_INDEX = 3;
        const int NAME_INDEX = 5;
        if (infoStringList.size() < NAME_INDEX + 1)
        {
            err->error = ZipError::PARSE_ERROR;
            return;
        }
        bool ok = true;
        qint64 size = infoStringList.at(SIZE_INDEX).toLongLong(&ok);
        if (!ok)
        {
            err->error = ZipError::PARSE_ERROR;
            return;
        }
        const QString &file = infoStringList.at(NAME_INDEX);
        Q_ASSERT(!fileList.contains(file));
        fileList[file] = size;
    };
    if (!LaunchArchiver(QStringList() << "l" << archivePath, callback, err))
    {
        return false;
    }
    if (fileList.empty())
    {
        err->error = ZipError::ARCHIVE_IS_EMPTY;
        return false;
    }
    return true;
}

bool ZipUtils::TestZipArchive(const QString &archivePath, const CompressedFilesAndSizes &files, ProgressFuntor onProgress, ZipError *err)
{
    if (err == nullptr)
    {
        err = ZIP_UTILS_LOCAL::GetDefaultZipError();
    }
    if (!IsArchiveValid(archivePath, err))
    {
        return false;
    }

    bool success = false;
    qint64 matchedSize = 0;
    const auto values = files.values();
    qint64 totalSize = std::accumulate(values.begin(), values.end(), 0);
    ReadyReadCallback callback = [&success, &onProgress, &files, &matchedSize, totalSize, err](const QByteArray &line) {
        QString str(line);
        QRegularExpression stringRegEx("\\s+");
        QStringList infoStringList = str.split(stringRegEx, QString::SkipEmptyParts);
        if (infoStringList.size() > 1)
        {
            const auto &file = infoStringList.at(1);
            if (files.contains(file))
            {
                matchedSize += files[file];
            }
        }

        onProgress((matchedSize * 100.0f) / totalSize);
        if (str.contains("Everything is Ok"))
        {
            success = true;
        }
    };
    QStringList arguments;
    arguments << "t" << "-bb1" << archivePath;
    if (!LaunchArchiver(arguments, callback, err))
    {
        return false;
    }
    if (success != true)
    {
        err->error = ZipError::ARHIVE_DAMAGED;
        return false;
    }
    return true;
}


bool ZipUtils::UnpackZipArchive(const QString &archivePath, const QString &outDirPath, const CompressedFilesAndSizes &files, ProgressFuntor onProgress, ZipError *err)
{
    if (err == nullptr)
    {
        err = ZIP_UTILS_LOCAL::GetDefaultZipError();
    }
    if (!IsArchiveValid(archivePath, err))
    {
        return false;
    }
    QDir outDir(outDirPath);
    if (!outDir.mkpath("."))
    {
        err->error = ZipError::OUT_DIRECTORY_NOT_EXISTS;
        return false;
    }

    bool success = false;
    qint64 matchedSize = 0;
    const auto values = files.values();
    qint64 totalSize = std::accumulate(values.begin(), values.end(), 0);
    ReadyReadCallback callback = [&success, &onProgress, &files, &matchedSize, totalSize, err](const QByteArray &line) {
        QString str(line);
        QRegularExpression stringRegEx("\\s+");
        QStringList infoStringList = str.split(stringRegEx, QString::SkipEmptyParts);
        if (infoStringList.size() > 1)
        {
            const auto &file = infoStringList.at(1);
            if (files.contains(file))
            {
                matchedSize += files[file];
            }
        }

        onProgress((matchedSize * 100.0f) / totalSize);
        if (str.contains("Everything is Ok"))
        {
            success = true;
        }
    };
    QStringList arguments;
    arguments << "x" << "-y" << "-bb1" << archivePath << "-o" + outDirPath;
    if (!LaunchArchiver(arguments, callback, err))
    {
        return false;
    }
    if (success != true)
    {
        err->error = ZipError::ARHIVE_DAMAGED;
        return false;
    }
    return true;
}
