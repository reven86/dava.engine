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
#include "Timer/RawTimer.h"

namespace DAVA
{

class CurlDownloader : public Downloader
{
public:
    CurlDownloader();
    virtual ~CurlDownloader();

protected:
    /**
        \brief Interrupts current download.
     */
    void Interrupt() override;
    /**
        \brief Init an easy handle for use it later for any Curl operation. Setups all common paramaters.
        Returns a pointer to CURL easy handle. NULL if there was an init error.
     */
    CURL *CurlSimpleInit();
    /**
     \brief Get content size in bytes for remote Url.
     \param[in] url - destination fie Url
     \param[out] retSize - place result to
     \param[in] timeout - operation timeout
     */
    DownloadError GetSize(const String &url, uint64 &retSize, int32 timeout) override;
    /**
     \brief Main downloading operation. Should call SaveData to store data.
     \param[in] url - destination file Url
     \param[in] savePath - path to save location of remote file
     \param[in] partsCount - quantity of download threads
     \param[in] timeout - operation timeout
     */
    DownloadError Download(const String &url, const FilePath &savePath, uint8 partsCount, int32 timeout) override;
    /**
     \brief Sets maximum allowed download speed. -1 means unlimited.
     \param[in] limit - speed limit in bytes per second.
     */
    void SetDownloadSpeedLimit(const uint64 limit) override;

private:
    /**
     \brief Method for save downloaded data in a separate thread
     */
    void SaveChunkHandler(BaseObject *caller, void *callerData, void *userData);
    /**
     \brief Downloads a part of file using a number of download threads
     \param[in] seek - position inside remote file to download from
     \param[in] size - size of data do download
     */
    DownloadError DownloadRangeOfFile(uint64 seek, uint32 size);
    /**
        \brief Data receive handler for all easy handles which downloads a data
        \param[in] ptr - pointer to incoming data chunk
        \param[in] size - one data buffer size
        \param[in] nmemb - quantity of came data buffers
        \param[in] part - pointer to download part which contains data for current download thread
     */
    static size_t CurlDataRecvHandler(void *ptr, size_t size, size_t nmemb, void *part);
    /**
        \brief Convert Curl easy interface error to Download error
        \param[in] status - Curl easy interface operation status status
     */
    DownloadError CurlStatusToDownloadStatus(CURLcode status) const;
    /**
       \brief Convert Curl multi interface error to Download error
       \param[in] curMultiCode - Curl multi interface operation status
     */
    DownloadError CurlmCodeToDownloadError(CURLMcode curlMultiCode) const;
    /**
        \brief Convert HTTP code to Download error
        \param[in] code HTTP code
     */
    DownloadError HttpCodeToDownloadError(uint32 code) const;
    /**
        \brief Create one of easy handles to download content. Returns a pointer to new created curl easy handle
        \param[in] part - pointer to download part which contains data for current download thread
     */
    void SetupEasyHandle(CURL *handle, DownloadPart *part);
    /**
     \brief Init curl download handles and DownloadParts
     */
    DownloadError CreateDownload();
    /**
     \brief Prepare all we need to start or resume download
     \param[in] seek - position inside remote file to download from
     \param[in] size - size of data do download
     */
    DownloadError SetupDownload(uint64 seek, uint32 size);
    /**
        \brief Do a prepared download. Do nothing and returnes DLE_NO_ERROR if there is no easy handles.
        \param[in] multiHandle - pointer to Curl multi interface handle
     */
    CURLMcode Perform();
    /**
        \brief Cleanup all used Curl resurces when they are not needed anymore
     */
    void CleanupDownload();
    /**
        \brief Set up Curl timeouts
        \param[in] handle - Curl easy handle to set options
     */
    void SetTimeout(CURL *easyHandle);
    /**
        \brief Handle download results and return generalized result
     */
    DownloadError HandleDownloadResults(CURLM *multiHandle);
    /**
        \brief Returns actual error state for given easy handle with it's ststus
        \param[in] easyHandle - Curl easy handle to set options
        \param[in] status - status of current easyHandle
     */
    DownloadError ErrorForEasyHandle(CURL *easyHandle, CURLcode status) const;
    /**
        \brief Take more importand error from all download results. Returns DLE_NO_ERROR if there is no errors or all is fine.
        \param[in] errorList - a lis of DownloadErrors to take the more important
     */
    DownloadError TakeMostImportantReturnValue(const Vector<DownloadError> &errorList) const;

private:
    struct ErrorWithPriority
    {
        DownloadError error;
        char8 priority;
    };
    
private:
    static bool isCURLInit;
    bool isDownloadInterrupting;
    uint8 currentDownloadPartsCount;
    Vector<DownloadPart *> downloadParts;
    Vector<CURL *> easyHandles;
    CURLM *multiHandle;
    FilePath storePath;
    String downloadUrl;
    int32 operationTimeout;
    RawTimer inactivityConnectionTimer;
    uint64 remoteFileSize;
    uint64 sizeToDownload;
    uint64 downloadSpeedLimit;

    static ErrorWithPriority errorsByPriority[];
    
    DownloadError saveResult;
    DataChunkInfo *chunkInfo;
    Mutex chunksMutex;
    List<DataChunkInfo *> chunksToSave;
    Thread *saveThread;
    const uint8 allowedBuffersInMemory;
    
    const uint32 maxChunkSize;
    const uint32 minChunkSize;
    
    bool isRangeRequestSent = false;
};

}

#endif
