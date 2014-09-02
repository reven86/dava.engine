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

#ifndef __DATA_CURL_DOWNLOADER_H__
#define __DATA_CURL_DOWNLOADER_H__

#include "Downloader.h"
#include "curl/curl.h"

namespace DAVA
{

class CurlDownloader : public Downloader
{
    enum HttpCodeClass
    {
        HTTP_INFO = 1,
        HTTP_SUCCESS,
        HTTP_REDIRECTION,
        HTTP_CLIENT_ERROR,
        HTTP_SERVER_ERROR,
    };

public:
    CurlDownloader(uint32 operationTimeout = 2);
    virtual ~CurlDownloader();

protected:
    virtual void Interrupt();

    CURL *CurlSimpleInit();

    virtual DownloadError GetSize(const String &url, int64 &retSize, int32 timeout = -1);
    virtual DownloadError Download(const String &url, const uint64 &loadFrom, int32 timeout = -1);
    
    static size_t CurlDataRecvHandler(void *ptr, size_t size, size_t nmemb, void *fileDownloader);
    
    DownloadError CurlStatusToDownloadStatus(const CURLcode &status);
    DownloadError HttpCodeToError(uint32 code);
    
private:
    static bool isCURLInit;
    static CURL *currentCurlHandle;
    bool isDownloadInterrupting;
};

}

#endif