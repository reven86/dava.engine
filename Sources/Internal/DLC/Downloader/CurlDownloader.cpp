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

CurlDownloader::CurlDownloader()
    : isDownloadInterrupting(false)
    , multiHandle(NULL)
{
    if (!isCURLInit && CURLE_OK == curl_global_init(CURL_GLOBAL_ALL))
        isCURLInit = true;
}

CurlDownloader::~CurlDownloader()
{
    curl_global_cleanup();
}

size_t CurlDownloader::CurlDataRecvHandler(void *ptr, size_t size, size_t nmemb, void *part)
{
    bool isFinished;
    
    PartInfo *thisPart = static_cast<PartInfo *>(part);
    CurlDownloader *thisDownloader = static_cast<CurlDownloader *>(thisPart->downloader);
    
    uint64 seekPos = thisPart->seekPos + thisPart->progress;
    uint64 dataLeft = thisPart->size - thisPart->progress;
    uint64 dataSize = size*nmemb;

    if (dataLeft <= dataSize)
    {
        dataSize = dataLeft; // don't write more than part.size
        isFinished = true; // download is finished
    }
    
    static Mutex writeLock;
    writeLock.Lock();
    size_t bytesWritten = thisDownloader->SaveData(ptr, dataSize, seekPos);
    writeLock.Unlock();
    
    thisPart->progress += bytesWritten; // if SaveData not performes - then we have not stored chunk of data and it is not a finished download.
    
    if (thisDownloader->isDownloadInterrupting)
        return -1; // download is interrupted

    if (isFinished && thisPart->progress == thisPart->size)
        return 0; // download is finished
        
    return bytesWritten;
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
    return curl_handle;
}
CURL *CurlDownloader::CreateEasyHandle(const String &url, PartInfo *part, int32 _timeout)
{
    /* init the curl session */
    CURL *handle = CurlSimpleInit();
    
    curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(handle, CURLOPT_WRITEFUNCTION, CurlDownloader::CurlDataRecvHandler);
    curl_easy_setopt(handle, CURLOPT_RESUME_FROM_LARGE, part->seekPos);
    //if (0 != part->size)
    //    curl_easy_setopt(handle, CURLOPT_MAXFILESIZE_LARGE, part->size);
    curl_easy_setopt(handle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(handle, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(handle, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(handle, CURLOPT_WRITEDATA, static_cast<void *>(part));
    
    // set all timeouts
    SetTimeout(handle, _timeout);
    
    return handle;
}
    
void CurlDownloader::CleanupDownload()
{
    
    while (false == downloadParts.empty())
    {
        Vector<PartInfo *>::iterator it = downloadParts.begin();
        SafeDelete((*it));
        downloadParts.erase(it);
    }

    while (false == easyHandles.empty())
    {
        Vector<CURL *>::iterator it = easyHandles.begin();
        curl_easy_cleanup((*it));
        easyHandles.erase(it);
    }
    
    curl_multi_cleanup(multiHandle);
    multiHandle = NULL;
}
    
void CurlDownloader::WaitForDone(CURLM *multiHandle)
{
    int still_running;
    do
    {
        struct timeval timeout;
        int rc; /* select() return code */
        
        fd_set fdread;
        fd_set fdwrite;
        fd_set fdexcep;
        int maxfd = -1;
        
        long curl_timeo = -1;
        
        FD_ZERO(&fdread);
        FD_ZERO(&fdwrite);
        FD_ZERO(&fdexcep);
        
        /* set a suitable timeout to play around with */
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        curl_multi_timeout(multiHandle, &curl_timeo);
        if(curl_timeo >= 0) {
            timeout.tv_sec = curl_timeo / 1000;
            if(timeout.tv_sec > 1)
                timeout.tv_sec = 1;
            else
                timeout.tv_usec = (curl_timeo % 1000) * 1000;
        }
        
        /* get file descriptors from the transfers */
        curl_multi_fdset(multiHandle, &fdread, &fdwrite, &fdexcep, &maxfd);
        
        /* In a real-world program you OF COURSE check the return code of the
         function calls.  On success, the value of maxfd is guaranteed to be
         greater or equal than -1.  We call select(maxfd + 1, ...), specially in
         case of (maxfd == -1), we call select(0, ...), which is basically equal
         to sleep. */
        
        rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
        
        switch(rc) {
            case -1:
                /* select error */ 
                break;
            case 0: /* timeout */ 
            default: /* action */ 
                curl_multi_perform(multiHandle, &still_running);
                break;
        }
    } while(still_running);
}


DownloadError CurlDownloader::Download(const String &url, const char8 partsCount, int32 _timeout)
{
    uint64 remoteFileSize;
    DownloadError err = GetSize(url, remoteFileSize, _timeout);
    
    if (DLE_NO_ERROR != err)
        return err;
    
    uint64 partSize = remoteFileSize/partsCount;
    
    multiHandle = curl_multi_init();
    if (NULL == multiHandle)
    {
        return DLE_INIT_ERROR;
    }
    
    for (int i = 0; i < partsCount; i++)
    {
        PartInfo *part;

        part = new PartInfo();
        part->seekPos = partSize * i;
        part->size = partSize;
        part->downloader = this;
        downloadParts.push_back(part);
        
        CURL *easyHandle = CreateEasyHandle(url, part,  _timeout);
        if (NULL == easyHandle)
        {
            // cleanup curl stuff
            CleanupDownload();
            return DLE_INIT_ERROR;
        }
        
        easyHandles.push_back(easyHandle);
        
        curl_multi_add_handle(multiHandle, easyHandle);
    }

    // we cannot divide without errors, so we will compensate that
    uint64 lastPartIncrement = remoteFileSize - partSize*partsCount;
    downloadParts.at(partsCount-1)->size += lastPartIncrement;
    
    int handlesToExecute;
    CURLMcode ret = curl_multi_perform(multiHandle, &handlesToExecute);

    WaitForDone(multiHandle);

    // cleanup curl stuff
    CleanupDownload();

    
    CURLcode curlStatus;
    
    Logger::FrameworkDebug("Download");
    Logger::FrameworkDebug("CURL: download status = %d", ret);
    Logger::FrameworkDebug("Downloader: download status %d", CurlStatusToDownloadStatus(curlStatus));
    if (isDownloadInterrupting)
    {
        isDownloadInterrupting = false;
        // that is an exception from rule because of CURL interrupting mechanism.
        return DLE_CANCELLED;
    }

    return CurlStatusToDownloadStatus(curlStatus);
}

DownloadError CurlDownloader::GetSize(const String &url, uint64 &retSize, int32 _timeout)
{
    float64 sizeToDownload = 0.0;
    CURL *currentCurlHandle = CurlSimpleInit();

    if (!currentCurlHandle)
        return DLE_INIT_ERROR;

    curl_easy_setopt(currentCurlHandle, CURLOPT_URL, url.c_str());

    // Set a valid user agent
    curl_easy_setopt(currentCurlHandle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.11) Gecko/20071127 Firefox/2.0.0.11");

    // Don't return the header (we'll use curl_getinfo();
    curl_easy_setopt(currentCurlHandle, CURLOPT_HEADER, 1);
    curl_easy_setopt(currentCurlHandle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(currentCurlHandle, CURLOPT_NOBODY, 1);
    curl_easy_setopt(currentCurlHandle, CURLOPT_SSL_VERIFYPEER, 0);
    
    // set all timeouts
    SetTimeout(currentCurlHandle, _timeout);

    curl_easy_setopt(currentCurlHandle, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(currentCurlHandle, CURLOPT_NOPROGRESS, 1);
    CURLcode curlStatus = curl_easy_perform(currentCurlHandle);
    curl_easy_getinfo(currentCurlHandle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &sizeToDownload);

    char8 *contentType = new char8[80];
    curl_easy_getinfo(currentCurlHandle, CURLINFO_CONTENT_TYPE, &contentType);
    uint32 httpCode;
    curl_easy_getinfo(currentCurlHandle, CURLINFO_HTTP_CODE, &httpCode);

    /* cleanup curl stuff */ 
    curl_easy_cleanup(currentCurlHandle);

    currentCurlHandle = NULL;
    Logger::FrameworkDebug("GetSize");
    Logger::FrameworkDebug("CURL: download status = %d", curlStatus);
    Logger::FrameworkDebug("Downloader: download status %d", CurlStatusToDownloadStatus(curlStatus));
    DownloadError retError;

    // to discuss. It is ideal to place it to DownloadManager because in that case we need to use same code inside each downloader.
    DownloadError httpError = HttpCodeToError(httpCode);
    Logger::FrameworkDebug("Downloader: download HTTP Code %d", CurlStatusToDownloadStatus(curlStatus));
    if (DLE_NO_ERROR != httpError)
    {
        retError = httpError;
    }
    else
    {
        retError = CurlStatusToDownloadStatus(curlStatus);
    }

    retSize = static_cast<int64>(sizeToDownload);
  
    return retError;
}
    
DownloadError CurlDownloader::CurlStatusToDownloadStatus(const CURLcode &status)
{
    switch (status)
    {
        case CURLE_OK:
            return DLE_NO_ERROR;

        case CURLE_RANGE_ERROR:
            return DLE_CANNOT_RESUME;

        case CURLE_WRITE_ERROR: // happens if callback function for data receive returns wrong number of written data
            return DLE_FILE_ERROR;

        case CURLE_COULDNT_RESOLVE_HOST:
            return DLE_COULDNT_RESOLVE_HOST;

        case CURLE_COULDNT_CONNECT:
        case CURLE_OPERATION_TIMEDOUT:
            return DLE_CANNOT_CONNECT;

        default:
            return DLE_COMMON_ERROR; // need to log status
    }
}


DownloadError CurlDownloader::HttpCodeToError(uint32 code)
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

void CurlDownloader::SetTimeout(CURL *handle, int _timeout)
{
    curl_easy_setopt(handle, CURLOPT_CONNECTTIMEOUT, _timeout);
    // we could set operation time limit which produce timeout if operation takes setted time.
    curl_easy_setopt(handle, CURLOPT_TIMEOUT, 0);
    curl_easy_setopt(handle, CURLOPT_DNS_CACHE_TIMEOUT, _timeout);
    curl_easy_setopt(handle, CURLOPT_SERVER_RESPONSE_TIMEOUT, _timeout);
}
    
}
