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
    , fileErrno(0)
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

DownloadPart::DownloadPart(Downloader *currentDownloader)
    : downloader(currentDownloader)
    , dataBuffer(0)
    , downloadSize(0)
    , seekPos(0)
    , progress(0)
    
{
}
    
bool DownloadPart::SaveToBuffer(char8 *srcBuf, uint32 size)
{
    DVASSERT(downloadSize - progress >= size);
    if (downloadSize - progress > 0)
    {
        Memcpy(dataBuffer + progress, srcBuf, size);
        progress += size;
        return true;
    }
    else
    {
        Logger::Error("[DownloadPart::SaveToBuffer] Cannot save data.");
    }
    return false;
}

DataChunkInfo::DataChunkInfo(uint32 size)
    : buffer(NULL)
    , bufferSize(size)
    , progress(0)
{
    buffer = new char8[bufferSize];
}

DataChunkInfo::~DataChunkInfo()
{
    SafeDeleteArray(buffer);
}

}