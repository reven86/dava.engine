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
CURL *CurlDownloader::currentCurlHandle = NULL;

CurlDownloader::CurlDownloader(uint32 operationTimeout)
    : Downloader(operationTimeout)
    , isDownloadInterrupting(false)
{
    if (!isCURLInit && CURLE_OK == curl_global_init(CURL_GLOBAL_ALL))
        isCURLInit = true;
}

CurlDownloader::~CurlDownloader()
{
    curl_global_cleanup();
}

size_t CurlDownloader::CurlDataRecvHandler(void *ptr, size_t size, size_t nmemb, void *fileDownloader)
{
    CurlDownloader *thisDownloader = static_cast<CurlDownloader *>(fileDownloader);
    size_t bytesWritten = thisDownloader->SaveData(ptr, size, nmemb);   

    if (thisDownloader->isDownloadInterrupting)
        return 0; 

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

DownloadError CurlDownloader::Download(const String &url, const uint64 &loadFrom, int32 _timeout)
{
    /* init the curl session */
    currentCurlHandle = CurlSimpleInit();

   if (!currentCurlHandle)
        return DLE_INIT_ERROR;

    curl_easy_setopt(currentCurlHandle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(currentCurlHandle, CURLOPT_WRITEFUNCTION, CurlDownloader::CurlDataRecvHandler);
    curl_easy_setopt(currentCurlHandle, CURLOPT_RESUME_FROM, loadFrom);
    curl_easy_setopt(currentCurlHandle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(currentCurlHandle, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(currentCurlHandle, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(currentCurlHandle, CURLOPT_WRITEDATA, static_cast<void *>(this));
    curl_easy_setopt(currentCurlHandle, CURLOPT_CONNECTTIMEOUT, (-1 >= _timeout) ? this->timeout : _timeout);
    
    /* get it! */ 
    CURLcode curlStatus = curl_easy_perform(currentCurlHandle);

    /* cleanup curl stuff */ 
    curl_easy_cleanup(currentCurlHandle);
    currentCurlHandle = NULL;

    if (isDownloadInterrupting)
    {
        isDownloadInterrupting = false;
        // that is an exception from rule because of CURL interrupting mechanism.
        return DLE_CANCELLED;
    }

    return CurlStatusToDownloadStatus(curlStatus);
}

DownloadError CurlDownloader::GetSize(const String &url, int64 &retSize, int32 _timeout)
{
    float64 sizeToDownload = 0.0;
    currentCurlHandle = CurlSimpleInit();

    if (!currentCurlHandle)
        return DLE_INIT_ERROR;

    curl_easy_setopt(currentCurlHandle, CURLOPT_URL, url.c_str());

    // Set a valid user agent
    curl_easy_setopt(currentCurlHandle, CURLOPT_USERAGENT, "Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.8.1.11) Gecko/20071127 Firefox/2.0.0.11");

    // Don't return the header (we'll use curl_getinfo();
    curl_easy_setopt(currentCurlHandle, CURLOPT_HEADER, 0);

    curl_easy_setopt(currentCurlHandle, CURLOPT_NOBODY, 1);
    curl_easy_setopt(currentCurlHandle, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(currentCurlHandle, CURLOPT_CONNECTTIMEOUT, (-1 >= _timeout) ? this->timeout : _timeout);
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
    
    DownloadError retError;

    // to discuss. It is ideal to place it to DownloadManager because in that case we need to use same code inside each downloader.
    DownloadError httpError = HttpCodeToError(httpCode);
    if (DLE_NO_ERROR != httpError)
    {
        retError = httpError;
        sizeToDownload = -1;
    }
    else
        retError = CurlStatusToDownloadStatus(curlStatus);

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

}