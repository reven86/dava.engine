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


#include "zipunpacker.h"
#include "quazip/quazip.h"
#include "quazip/quazipfile.h"
#include <QDir>
#include <QFile>
#include <QPair>

#define PERM_OTHER_EXEC 1 << 0
#define PERM_OWNER_EXEC 1 << 3
#define PERM_GROUP_EXEC 1 << 6
#define PERM_DIRECTORY  1 << 8
#define ATTRIBUTE_SYMLINK 0xA0000000 // on macos this value equal S_IFLNK from stat.st_mode

ZipUnpacker::ZipUnpacker(QObject *parent) :
    QObject(parent)
{
    errorMap[0] = "UNZ_OK";
    errorMap[-1] = "UNZ_ERRNO";
    errorMap[-100] = "UNZ_END_OF_LIST_OF_FILE";
    errorMap[-102] = "UNZ_PARAMERROR";
    errorMap[-103] = "UNZ_BADZIPFILE";
    errorMap[-104] = "UNZ_INTERNALERROR";
    errorMap[-105] = "UNZ_CRCERROR";
}

ZipUnpacker::~ZipUnpacker()
{

}

const QString & ZipUnpacker::GetErrorString(int errorCode)
{
    if(errorMap.contains(errorCode))
        return errorMap[errorCode];
    else
        return errorMap[-1];
}

bool ZipUnpacker::UnZipFile(const QString& archiveFilePath, const QString& extDirPath)
{
    QuaZip zip(archiveFilePath);
    if (!zip.open(QuaZip::mdUnzip))
    {
        emit OnError(zip.getZipError());
        return false;
    }

    QDir().mkpath(extDirPath);

    QuaZipFileInfo info;
    QuaZipFile file(&zip);

    QString name;

    int allcount = zip.getEntriesCount();
    int unzipCount = 0;

    QList<QPair<QString, QString>> symlinks;

    for (bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
    {
        if (!zip.getCurrentFileInfo(&info))
        {
            emit OnError(zip.getZipError());
            return false;
        }

        if (!file.open(QIODevice::ReadOnly))
        {
            emit OnError(zip.getZipError());
            return false;
        }

        name = QDir::toNativeSeparators(QString("%1/%2").arg(extDirPath).arg(file.getActualFileName()));

        if (file.getZipError() != UNZ_OK)
        {
            emit OnError(zip.getZipError());
            return false;
        }

        uint attr = info.externalAttr;
        attr = attr >> 16;  //leave only file permision info

        bool isSymLink = (info.externalAttr & ATTRIBUTE_SYMLINK) == ATTRIBUTE_SYMLINK;

        if (name.at(name.size() - 1) == '/' || name.at(name.size() - 1) == '\\')
        {
            //create directory
            QDir().mkdir(name);
        }
        else if (isSymLink)
        {
            symlinks.push_back(qMakePair(name, QString(file.readAll())));
        }
        else
        {
            QFile out;

            //unpack file
            out.setFileName(name);

            // this will fail if "name" contains subdirectories, but we don't mind that
            if (!out.open(QIODevice::WriteOnly))
            {
                qDebug("[ZipUnpacker::UnZipFile]: Error create file");
            }
            else
            {
                QFile::Permissions perm = out.permissions();
                perm |= QFile::ExeOwner;
                perm |= QFile::ExeGroup;
                perm |= QFile::ExeOther;

                if (!out.setPermissions(perm))
                  qDebug("[ZipUnpacker::UnZipFile] Error set file permision");

                out.write(file.readAll());
                out.close();
            }
        }

        if (file.getZipError() != UNZ_OK)
        {
            emit OnError(zip.getZipError());
            return false;
        }

        if (!file.atEnd()) {
            qWarning("[ZipUnpacker::UnZipFile]: read all but not EOF");
            return false;
        }

        file.close();

        if (file.getZipError() != UNZ_OK)
        {
            emit OnError(zip.getZipError());
            return false;
        }
        unzipCount++;

        emit OnProgress(unzipCount, allcount);
    }

    for (QPair<QString, QString> link : symlinks)
    {
        QFile::link(link.second, link.first);
    }

    emit OnComplete();

    return true;
}
