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
    CurlDownloader();
    virtual ~CurlDownloader();

protected:
    virtual void Interrupt();

    CURL *CurlSimpleInit();

    virtual DownloadError GetSize(const String &url, uint64 &retSize, const int32 timeout);
    virtual DownloadError Download(const String &url, const FilePath &savePath, const uint8 partsCount, const int32 timeout);
    
    static size_t CurlDataRecvHandler(void *ptr, size_t size, size_t nmemb, void *part);
    
    DownloadError CurlStatusToDownloadStatus(const CURLcode &status);
    DownloadError HttpCodeToError(uint32 code);

private:
    CURL *CreateEasyHandle(const String &url, DownloadPart *part, int32 _timeout);
    DownloadError CreateDownload(CURLM **multiHandle, const String &url, const FilePath &savePath, const uint8 partsCount, int32 timeout);
    CURLMcode Perform(CURLM *multiHandle);
    void CleanupDownload();
    void SetTimeout(CURL *handle, int32 _timeout);
    DownloadError ErrorForEasyHandle(CURL *easyHandle, const CURLcode status);
    DownloadError TakeMostImportantReturnValue(const Vector<DownloadError> &errorList);

private:
    static bool isCURLInit;
    bool isDownloadInterrupting;
    Vector<DownloadPart *> downloadParts;
    Vector<CURL *> easyHandles;
    CURLM *multiHandle;
    FilePath storePath;
    FilePath dlInfoStorePath;
    const String dlinfoExt;
    
    struct ErrorWithPriority
    {
        DownloadError error;
        char8 priority;
    };
    static ErrorWithPriority errorsByPriority[];
};

}

#endif