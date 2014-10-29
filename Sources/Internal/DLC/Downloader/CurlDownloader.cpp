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

#include "CurlDownloader.h"

namespace DAVA
{

bool CurlDownloader::isCURLInit = false;

CurlDownloader::ErrorWithPriority CurlDownloader::errorsByPriority[] = {
    { DLE_INIT_ERROR, 0 },
    { DLE_FILE_ERROR, 1 },
    { DLE_COULDNT_RESOLVE_HOST, 2 },
    { DLE_COULDNT_CONNECT, 3 },
    { DLE_CONTENT_NOT_FOUND, 4 },
    { DLE_COMMON_ERROR, 5 },
    { DLE_UNKNOWN, 6 },
    { DLE_CANCELLED, 7 },
    { DLE_NO_ERROR, 8 },
};

CurlDownloader::CurlDownloader()
    : isDownloadInterrupting(false)
    , multiHandle(NULL)
    , storePath("")
    , dlInfoStorePath("")
    , dlinfoExt(".dlinfo")
{
    if (!isCURLInit && CURLE_OK == curl_global_init(CURL_GLOBAL_ALL))
    {
        isCURLInit = true;
    }
}

CurlDownloader::~CurlDownloader()
{
    curl_global_cleanup();
}

size_t CurlDownloader::CurlDataRecvHandler(void *ptr, size_t size, size_t nmemb, void *part)
{
    DownloadPart *thisPart = static_cast<DownloadPart *>(part);
    CurlDownloader *thisDownloader = static_cast<CurlDownloader *>(thisPart->downloader);
    
    uint64 seekPos = thisPart->info.seekPos + thisPart->info.progress;
    uint64 dataLeft = thisPart->info.size - thisPart->info.progress;
    size_t dataSizeCame = size*nmemb;
    uint64 dataSizeToWrite = 0;
    
    if (dataLeft < dataSizeCame)
    {
        Logger::Error("[CurlDownloader::CurlDataRecvHandler] dataLeft < dataSizeCame");
        dataSizeToWrite = dataLeft; // don't write more than part.size
    }
    else
    {
        dataSizeToWrite = dataSizeCame;
    }
    
    size_t bytesWritten = thisDownloader->SaveData(ptr, thisDownloader->storePath, dataSizeToWrite, seekPos);
    if (bytesWritten != dataSizeToWrite)
    {
        Logger::Error("[CurlDownloader::CurlDataRecvHandler] Couldn't save downloaded data chunk");
        return bytesWritten; // this case means that not all data which we wants to save is saved. So we produce file system error.
    }
    
    thisPart->info.progress += bytesWritten; // if SaveData not performes - then we have not stored chunk of data and it is not a finished download.

    if (!thisPart->SaveDownload(thisDownloader->dlInfoStorePath))
    {
        Logger::Error("[CurlDownloader::CurlDataRecvHandler] Couldn't save download part info");
        return 0;
    }

    if (thisDownloader->isDownloadInterrupting)
    {
        return 0; // download is interrupted
    }
    
    // no errors was found
    return dataSizeCame;
}

void CurlDownloader::Interrupt()
{
    isDownloadInterrupting = true;
}

CURL *CurlDownloader::CurlSimpleInit()
{
    /* init the curl session */
    CURL *curl_handle = curl_easy_init();

    curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.11) Gecko/20071127 Firefox/2.0.0.11");
    curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl_handle, CURLOPT_FOLLOWLOCATION, 1);
    return curl_handle;
}

CURL *CurlDownloader::CreateEasyHandle(const String &url, DownloadPart *part, int32 timeout)
{
    /* init the curl session */
    CURL *handle = CurlSimpleInit();
    
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, CurlDownloader::CurlDataRecvHandler);
    if (0 < part->info.size)
    {
        char8 rangeStr[80];
        sprintf(rangeStr, "%lld-%lld", part->info.seekPos + part->info.progress, part->info.size + part->info.seekPos - 1);
        curl_easy_setopt(handle, CURLOPT_RANGE, rangeStr);
        curl_easy_setopt(handle, CURLOPT_NOBODY, 0);
    }
    else
    {
        // we don't need to receive any data when it is unexpected
        curl_easy_setopt(handle, CURLOPT_NOBODY, 1);
    }
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, static_cast<void *>(part));

    
    // set all timeouts
    SetTimeout(handle, timeout);
    
    return handle;
}
    
DownloadError CurlDownloader::CreateDownload(CURLM **multiHandle, const String &url, const FilePath &savePath, uint8 partsCount, int32 timeout)
{
    DVASSERT(0 < partsCount);

    uint64 remoteFileSize;
    DownloadError err = GetSize(url, remoteFileSize, timeout);

    if (DLE_NO_ERROR != err)
        return err;

    // save it for static SaveData function.
    storePath = savePath;
    dlInfoStorePath = storePath + dlinfoExt;

    uint64 partSize;
    uint8 downloadPartsCount;
    
    
    if (!savePath.Exists())
    {
        if (dlInfoStorePath.Exists())
        {
            if (!FileSystem::Instance()->DeleteFile(dlInfoStorePath))
            {
                return DLE_FILE_ERROR;
            }
        }

        // create new file if there is no file and allocate space for it
        if (!FileSystem::CreateZeroFilledFile(savePath, remoteFileSize))
        {
            return DLE_FILE_ERROR;
        }
    }

    // try to restore interrupted download
    const bool isRestoredDownlaod = DownloadPart::RestoreDownload(dlInfoStorePath, downloadParts);
    if (!isRestoredDownlaod)
    {
        downloadPartsCount = partsCount;
        partSize = remoteFileSize / downloadPartsCount;
        if (!DownloadPart::CreateDownload(dlInfoStorePath, partsCount))
        {
            return DLE_FILE_ERROR;
        }
    }
    else
    {
        // restored parts count could be not equal with
        downloadPartsCount = downloadParts.size();
    }

    *multiHandle = curl_multi_init();

    if (NULL == *multiHandle)
    {
        return DLE_INIT_ERROR;
    }

    CURLMcode ret;
    for (uint8 i = 0; i < downloadPartsCount; i++)
    {
        DownloadPart *part;
        
        if (!isRestoredDownlaod)
        {
            part = new DownloadPart();
            part->info.seekPos = partSize * i;
            part->info.size = partSize;
            part->info.number = i;
            part->info.progress = 0;

            if (!part->SaveDownload(dlInfoStorePath))
            {
                CleanupDownload();
                return DLE_FILE_ERROR;
            }

            if (i == partsCount - 1)
            {
                // we cannot divide without errors, so we will compensate that
                part->info.size += remoteFileSize - partSize*downloadPartsCount;
            }

            downloadParts.push_back(part);
        }
        else
        {
            part = downloadParts.at(i);
            notifyProgress(part->info.progress);
        }

        part->downloader = this;

        CURL *easyHandle = CreateEasyHandle(url, part, timeout);

        if (NULL == easyHandle)
        {
            Logger::Error("[CurlDownloader::CreateDownload] Curl easy handle init error");
            CleanupDownload();
            return DLE_INIT_ERROR;
        }

        easyHandles.push_back(easyHandle);

        ret = curl_multi_add_handle(*multiHandle, easyHandle);
        if (CURLM_OK != ret)
        {
            Logger::Error("[CurlDownloader::CreateDownload] Curl multi add handle error %d: ", ret);
            CleanupDownload();
            return DLE_INIT_ERROR;
        }
    }

    return DLE_NO_ERROR;
}

CURLMcode CurlDownloader::Perform(CURLM *multiHandle)
{
    CURLMcode ret;
    int handlesRunning = 0;
    ret = curl_multi_perform(multiHandle, &handlesRunning);
    if (CURLM_OK != ret)
    {
        return ret;
    }

    do
    {
        struct timeval timeout;
        int rc = -1; /* select() return code */

        fd_set fdread;
        fd_set fdwrite;
        fd_set fdexcep;
        int32 maxfd = -1;
        long curlTimeout = -1;

        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);

        /* set a suitable timeout to play around with */
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;

        ret = curl_multi_timeout(multiHandle, &curlTimeout);
        if (CURLM_OK != ret)
        {
            break;
        }

        if (curlTimeout >= 0)
        {
            timeout.tv_sec = curlTimeout / 1000;
            if (timeout.tv_sec > 1)
            {
                timeout.tv_sec = 1;
            }
            else
            {
                timeout.tv_usec = (curlTimeout % 1000) * 1000;
            }
        }
        /* get file descriptors from the transfers */
        ret = curl_multi_fdset(multiHandle, &fdread, &fdwrite, &fdexcep, &maxfd);
        if (CURLM_OK != ret)
            break;

        /* In a real-world program you OF COURSE check the return code of the
        function calls.  On success, the value of maxfd is guaranteed to be
        greater or equal than -1.  We call select(maxfd + 1, ...), specially in
        case of (maxfd == -1), we call select(0, ...), which is basically equal
        to sleep. */
        if (maxfd >= 0)
        {
            rc = select(maxfd + 1, &fdread, &fdwrite, &fdexcep, &timeout);
        }
        else
        {
            // Curl documentation recommends to sleep not less than 200ms in this case
            Thread::Sleep(200);
            rc = 0;
        }

        switch (rc)
        {
        case -1:
            /* select error */
            break;
        case 0: /* timeout */
        default: /* action */
            ret = curl_multi_perform(multiHandle, &handlesRunning);
            if (CURLM_OK != ret)
            {
                return ret;
            }
            break;
        }
    } while (handlesRunning);

    return ret;
}

void CurlDownloader::CleanupDownload()
{
    Vector<DownloadPart *>::iterator endP = downloadParts.end();
    for (Vector<DownloadPart *>::iterator it = downloadParts.begin(); it != endP; ++it)
    {
        SafeDelete(*it);
    }
    downloadParts.clear();
    
    Vector<CURL *>::iterator endH = easyHandles.end();
    for (Vector<CURL *>::iterator it = easyHandles.begin(); it != endH; ++it)
    {
        curl_easy_cleanup(*it);
    }
    easyHandles.clear();
    
    curl_multi_cleanup(multiHandle);
    multiHandle = NULL;
}

DownloadError CurlDownloader::Download(const String &url, const FilePath &savePath, uint8 partsCount, int32 timeout)
{
    Logger::FrameworkDebug("[CurlDownloader::Download]");
    DownloadError retCode;

    CURLM *multiHandle = NULL;

    DownloadError retCreate = CreateDownload(&multiHandle, url, savePath, partsCount, timeout);
    if (DLE_NO_ERROR != retCreate)
    {
        return retCreate;
    }
    
    CURLMcode retPerform = Perform(multiHandle);
    
    DVASSERT(CURLM_CALL_MULTI_PERFORM != retPerform); // should not be used in curl 7.20.0 and later.    

    // handle easy handles states
    Vector<DownloadError> results;
    if (CURLM_OK == retPerform)
    {
        int32 messagesRest;
        do
        {
            CURLMsg *message = curl_multi_info_read(multiHandle, &messagesRest);
            if (NULL == message)
            {
                break;
            }

            results.push_back(ErrorForEasyHandle(message->easy_handle, message->data.result));
        } while (0 != messagesRest);
    }
    else
    {
        retCode = CurlmCodeToDownloadError(retPerform);
    }

    // cleanup curl stuff
    CleanupDownload();

    if (isDownloadInterrupting)
    {
        isDownloadInterrupting = false;
        // that is an exception from rule because of CURL interrupting mechanism.
        return DLE_CANCELLED;
    }

    retCode = TakeMostImportantReturnValue(results);

    if (DLE_NO_ERROR == retCode)
    {
        if (dlInfoStorePath.Exists())
        {
            if (!FileSystem::Instance()->DeleteFile(dlInfoStorePath))
            {
                Logger::Error("[CurlDownloader::Download] Error at .dlinfo file deletion");
                // there is no return because it is not matter for download.
            }
        }
    }
    
    return retCode;
}

DownloadError CurlDownloader::GetSize(const String &url, uint64 &retSize, int32 timeout)
{
    float64 sizeToDownload = 0.0;
    CURL *currentCurlHandle = CurlSimpleInit();

    if (!currentCurlHandle)
    {
        return DLE_INIT_ERROR;
    }

    curl_easy_setopt(currentCurlHandle, CURLOPT_HEADER, 0);
    curl_easy_setopt(currentCurlHandle, CURLOPT_URL, url.c_str());
    
    // Don't return the header (we'll use curl_getinfo();
    curl_easy_setopt(currentCurlHandle, CURLOPT_NOBODY, 1);

    // set all timeouts
    SetTimeout(currentCurlHandle, timeout);

    CURLcode curlStatus = curl_easy_perform(currentCurlHandle);
    curl_easy_getinfo(currentCurlHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &sizeToDownload);
    if (0 > sizeToDownload)
    {
        sizeToDownload = 0.;
    }

    DownloadError retError = ErrorForEasyHandle(currentCurlHandle, curlStatus);
    retSize = static_cast<int64>(sizeToDownload);


    /* cleanup curl stuff */ 
    curl_easy_cleanup(currentCurlHandle);

    return retError;
}
    
DownloadError CurlDownloader::CurlStatusToDownloadStatus(CURLcode status) const
{
    switch (status)
    {
        case CURLE_OK:
            return DLE_NO_ERROR;

        case CURLE_RANGE_ERROR:
            return DLE_COULDNT_RESUME;

        case CURLE_WRITE_ERROR: // happens if callback function for data receive returns wrong number of written data
            return DLE_FILE_ERROR;

        case CURLE_COULDNT_RESOLVE_HOST:
            return DLE_COULDNT_RESOLVE_HOST;

        case CURLE_RECV_ERROR:
        case CURLE_COULDNT_CONNECT:
        case CURLE_OPERATION_TIMEDOUT:
            return DLE_COULDNT_CONNECT;

        default:
            return DLE_COMMON_ERROR; // need to log status
    }
}

DownloadError CurlDownloader::CurlmCodeToDownloadError(CURLMcode curlMultiCode) const
{
    switch(curlMultiCode)
    {
        case CURLM_OK:
            return DLE_NO_ERROR;
        case CURLM_CALL_MULTI_PERFORM:
        case CURLM_ADDED_ALREADY:
        case CURLM_BAD_HANDLE:
        case CURLM_BAD_EASY_HANDLE:
        case CURLM_OUT_OF_MEMORY:
            return DLE_INIT_ERROR;
        case CURLM_INTERNAL_ERROR:
        case CURLM_BAD_SOCKET:
        case CURLM_UNKNOWN_OPTION:
        default:
            return DLE_COMMON_ERROR;
        }
}

DownloadError CurlDownloader::HttpCodeToDownloadError(uint32 code) const
{
    HttpCodeClass code_class = static_cast<HttpCodeClass>(code/100);
    switch (code_class)
    {
        case HTTP_CLIENT_ERROR:
        case HTTP_SERVER_ERROR:
            return DLE_CONTENT_NOT_FOUND;
        default:
            return DLE_NO_ERROR;
    }
}

void CurlDownloader::SetTimeout(CURL *easyHandle, int32 timeout)
{
    curl_easy_setopt(easyHandle, CURLOPT_CONNECTTIMEOUT, timeout);
    // we could set operation time limit which produce timeout if operation takes setted time.
    curl_easy_setopt(easyHandle, CURLOPT_TIMEOUT, 0);
    curl_easy_setopt(easyHandle, CURLOPT_DNS_CACHE_TIMEOUT, timeout);
    curl_easy_setopt(easyHandle, CURLOPT_SERVER_RESPONSE_TIMEOUT, timeout);
}

DownloadError CurlDownloader::ErrorForEasyHandle(CURL *easyHandle, CURLcode status) const
{
    DownloadError retError;

    uint32 httpCode;
    curl_easy_getinfo(easyHandle, CURLINFO_HTTP_CODE, &httpCode);

    // to discuss. It is ideal to place it to DownloadManager because in that case we need to use same code inside each downloader.
    
    DownloadError httpError = HttpCodeToDownloadError(httpCode);
    if (DLE_NO_ERROR != httpError)
    {
        retError = httpError;
    }
    else
    {
        retError = CurlStatusToDownloadStatus(status);
    }

    return retError;
}

DownloadError CurlDownloader::TakeMostImportantReturnValue(const Vector<DownloadError> &errorList) const
{
    char8 errorCount = sizeof(errorsByPriority)/sizeof(ErrorWithPriority);
    char8 retIndex = errorCount - 1; // last error in the list is the less important.
    char8 priority = errorsByPriority[retIndex].priority; //priority of less important error
    
    // iterate over download results
    Vector<DownloadError>::const_iterator end = errorList.end();
    for (Vector<DownloadError>::const_iterator it = errorList.begin(); it != end; ++it)
    {
        // find error in the priority map
        for (char8 i = 0; i < errorCount; ++i)
        {
            // yes, that is the error
            if (errorsByPriority[i].error == (*it))
            {
                // is found error priority is less that current more important error?
                // less priority is more important
                if (priority > errorsByPriority[i].priority)
                {
                    //yes, so save more important priority and it's index
                    priority = errorsByPriority[i].priority;
                    retIndex = i;
                }
            }
        }
    }
    
    //return more important error by index
    return errorsByPriority[retIndex].error;
}

}
