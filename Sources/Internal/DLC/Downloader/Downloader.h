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

#ifndef __DATA_DOWNLOADER_H__
#define __DATA_DOWNLOADER_H__

#include "Base/BaseTypes.h"
#include "Platform/Thread.h"
#include "Platform/Mutex.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "DownloaderCommon.h"

namespace DAVA
{

/*
    Base class for eny downloaders. Used as interface inside DownloadManager.
*/
class Downloader
{

friend class DownloadManager;

public:
    Downloader(uint32 operationTimeout = 2);
    virtual ~Downloader(){};

/* all methods putted into protected section because they should be used only from DownloadManager. */
protected: 
    /* Get content size in bytes for remote Url. Place result to retSize, timeout - for operation cancelling */
    virtual DownloadError GetSize(const String &url, int64 &retSize, int32 _timeout = -1) = 0;
    /* Main downloading operation. Should call SaveData to store data. */
    virtual DownloadError Download(const String &url, const uint64 &loadFrom, int32 _timeout = -1) = 0;
    /* Interrupt download process. We expects that you will save last data chunk came before */
    virtual void Interrupt() = 0;
    /* 
        Main save method. Should be preferred way to store any downloaded data. If not - you can reimplement it, but it is not recommended. 
        Take a look on CurlDownloader::CurlDataRecvHandler(...) for example.
    */
    virtual size_t SaveData(void *ptr, size_t size, size_t nmemb);

protected:
    uint32 timeout;
};

}

#endif