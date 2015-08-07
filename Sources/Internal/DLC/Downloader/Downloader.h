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
#include "Concurrency/Thread.h"
#include "Concurrency/Mutex.h"
#include "FileSystem/File.h"
#include "FileSystem/FileSystem.h"
#include "DownloaderCommon.h"
#include "Functional/Function.h"
#include "Concurrency/Spinlock.h"

namespace DAVA
{

/*
    Base class for eny downloaders. Used as interface inside DownloadManager.
*/
class Downloader
{
/* only Download manager could use Downloader and it's childs*/
friend class DownloadManager;
    
public:
    Downloader();
    virtual ~Downloader(){};

/* all methods putted into protected section because they should be used only from DownloadManager. */
protected: 
    /**
        \brief Get content size in bytes for remote Url.
        \param[in] url - destination fie Url
        \param[out] retSize - place result to
        \param[in] timeout - operation timeout
     */
    virtual DownloadError GetSize(const String &url, uint64 &retSize, int32 timeout) = 0;
    /**
        \brief Main downloading operation. Should call SaveData to store data.
        \param[in] url - destination file Url
        \param[in] savePath - path to save location of remote file
        \param[in] partsCount - quantity of download threads
        \param[in] timeout - operation timeout
    */
    virtual DownloadError Download(const String &url, const FilePath &savePath, uint8 partsCount, int32 timeout) = 0;
    /**
        \brief Interrupt download process. We expects that you will save last data chunk came before 
     */
    virtual void Interrupt() = 0;
    /**
        \brief Main save method. Should be preferred way to store any downloaded data. If not - you can reimplement it, but it is not recommended.
        Take a look on CurlDownloader::CurlDataRecvHandler(...) for example.
        \param[in] ptr - pointer to data
        \param[in] storePath - path to save location of remote file
        \param[in] size - amout of data
        \param[in] seek - position in file where data should be stored
    */
    virtual bool SaveData(const void *ptr, const FilePath& storePath, uint64 size);
    /**
        \brief Used to report about saved data size to download manager. Used to calculate total download progress.
     */
    virtual void SetProgressNotificator(Function<void (uint64)> progressNotifier);
    /**
        \brief Reset download statistics
        \param[in] sizeToDownload - data size we suppose to download
     */
    void ResetStatistics(uint64 sizeToDownload);
    /**
        \brief Calculate download statistics. Should be called at data came or saved.
        \param[in] dataCame - amout of data came or saved.
     */
    void CalcStatistics(uint32 dataCame);
    /**
        \brief Returns download statistics structure
     */
    DownloadStatistics GetStatistics();
    /**
         \brief Sets maximum allowed download speed. 0 means unlimited.
         \param[in] limit - speed limit in bytes per second.
     */
    virtual void SetDownloadSpeedLimit(const uint64 limit) = 0;

    /**
        \brief return errno occurred during work with destination file
    */
    int32 GetFileErrno() const;

protected:
    int32 fileErrno;
    Function<void(uint64)> notifyProgress;
    
private:
    uint64 dataToDownloadLeft;
    
    Spinlock statisticsMutex;
    DownloadStatistics statistics;

};


}

#endif