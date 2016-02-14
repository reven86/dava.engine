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

#include "ziplist.h"
#include <QString>
#include <QProcess>
#include <QFile>
#include <QRegularExpression>
#include <QEventLoop>

void ZipList::GetFileList(const QString &archivePath, CompressedFilesAndSizes &fileList, ZipError *err)
{
    if(err == nullptr)
    {
        static ZipError localError;
        err = &localError;
    }
    if(!ZipUtils::IsArchiveValid(archivePath, err))
    {
        return;
    }
    QRegularExpression regExp("\\s+");
    bool foundOutputData = false;
    ZipUtils::ReadyReadCallback callback = [&regExp, &foundOutputData, err, &fileList](const QByteArray &line){
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
    if(!ZipUtils::LaunchArchiver(QStringList() << "l" << archivePath, callback, err))
    {
        return;
    }
    if(fileList.empty())
    {
        err->error = ZipError::ARCHIVE_IS_EMPTY;
        return;
    }
}
