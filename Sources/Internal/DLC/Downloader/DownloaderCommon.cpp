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

#include "DownloaderCommon.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{
DownloadTaskDescription::DownloadTaskDescription(const String &srcUrl, const FilePath &storeToFilePath, DownloadType downloadMode, int32 _timeout, int32 _retriesCount, uint8 _partsCount)
    : id(0)
    , url(srcUrl)
    , storePath(storeToFilePath)
    , timeout(_timeout)
    , retriesCount(_retriesCount)
    , retriesLeft(retriesCount)
    , type(downloadMode)
    , status(DL_UNKNOWN)
    , error(DLE_NO_ERROR)
    , downloadTotal(0)
    , downloadProgress(0)
    , partsCount(_partsCount)
{

}

bool DownloadPart::RestoreDownload(const FilePath &infoFilePath, Vector<DownloadPart*> &downloadParts)
{
    ScopedPtr<File> infoFile(File::Create(infoFilePath, File::OPEN | File::READ));
    if (NULL == static_cast<File *>(infoFile))
    {
        return false;
    }
    
    DownloadInfoHeader downloadHeader;
    uint8 partsRead = 0;
    const uint64 readPartSize = sizeof(DownloadPart::StoreData);
    
    bool readOk = sizeof(DownloadInfoHeader) == infoFile->Read(&downloadHeader);

    while (readOk && partsRead < downloadHeader.partsCount)
    {
        DownloadPart *part = new DownloadPart();

        readOk = sizeof(part->info.number) == infoFile->Read(&part->info.number);
     
        if (readOk)
        {
            infoFile->Seek(sizeof(DownloadInfoHeader) + part->info.number * readPartSize, File::SEEK_FROM_START);
            readOk = sizeof(part->info) == infoFile->Read(&part->info);
        }
        
        if (!readOk)
        {
            Logger::Error("[DownloadPart::RestoreDownload] Unexpected end of file");
            SafeDelete(part);
            return false;
        }

        // progress should not be bigger than expected download size
        DVASSERT(part->info.progress <= part->info.size);

        if (part->info.size == part->info.progress)
        {
            SafeDelete(part);
        }
        else
        {
            downloadParts.push_back(part);
        }

        ++partsRead;
    }

    return readOk;
}

bool DownloadPart::SaveDownload(const FilePath &infoFilePath)
{
    ScopedPtr<File> file(File::Create(infoFilePath, File::OPEN | File::READ | File::WRITE));
    if (NULL != static_cast<File*>(file))
    {
        // we should to write pert data in it's place inside info file
        uint64 seekPos = sizeof(DownloadInfoHeader) + info.number*sizeof(DownloadPart::StoreData);
        file->Seek(seekPos, File::SEEK_FROM_START);

        return sizeof(info) == file->Write(&info, sizeof(info));
    }

    return false;
}

bool DownloadPart::CreateDownload(const FilePath &infoFilePath, uint8 partsCount)
{
    // Create info file and allocate space for it
    uint64 infoFileSize = sizeof(DownloadInfoHeader) + partsCount*sizeof(DownloadPart::StoreData);
    if (!FileSystem::CreateZeroFilledFile(infoFilePath, infoFileSize))
    {
        return false;
    }

    // write download header into info file
    ScopedPtr<File> file(File::Create(infoFilePath, File::OPEN | File::READ | File::WRITE));
    if (NULL != static_cast<File*>(file))
    {
        DownloadInfoHeader header;
        header.partsCount = partsCount;
        return (sizeof(DownloadInfoHeader) == file->Write(&header));
    }

    return false;
}

}