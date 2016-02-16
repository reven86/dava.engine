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


#ifndef __LAUNCHER_ZIP_UTILS_H__
#define __LAUNCHER_ZIP_UTILS_H__

//this is private header
#include <QObject>
#include <functional>

class ZipError : public QObject
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
        : error(errCode) {}
    eZipListError error = NO_ERRORS;
    QString GetErrorString() const;
};

namespace ZipUtils
{
    class ZipOperationFunctor
    {
    public:
        virtual ~ZipOperationFunctor() = default;
        
        virtual void OnStart() {};
        virtual void OnProgress(int) {};
        virtual void OnSuccess() {};
        virtual void OnError(const ZipError &) {};
    };

    using ReadyReadCallback = std::function<void(const QByteArray &)>;
    using CompressedFilesAndSizes = QMap < QString, qint64 >;

    const QString &GetArchiverPath();
    bool IsArchiveValid(const QString &archivePath, ZipError *err = nullptr);
    bool LaunchArchiver(const QStringList &arguments, ReadyReadCallback callback = ReadyReadCallback(), ZipError *err = nullptr);
    bool GetFileList(const QString &archivePath, CompressedFilesAndSizes &files, ZipOperationFunctor &functor);
    bool TestZipArchive(const QString &archivePath, const CompressedFilesAndSizes &files, ZipOperationFunctor &functor);
    bool UnpackZipArchive(const QString &archivePath, const QString &outDir, const CompressedFilesAndSizes &files, ZipOperationFunctor &onProgress);


}

#endif // __LAUNCHER_ZIP_LIST_H__
