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



#ifndef __DAVAENGINE_FILE_DOWNLOADER_H__
#define __DAVAENGINE_FILE_DOWNLOADER_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

class File;
typedef void DCURL;


class FileDownloaderDelegate
{
public:
    enum DownloadStatusCode
    {
        DL_SUCCESS = 0, //
        DL_ERROR_FILE_SYSTEM,
        DL_ERROR_DOWNLOAD
    };
    
public:
    virtual ~FileDownloaderDelegate() = 0;
    
    virtual void DownloadGetPacket(uint64 size);
    virtual void DownloadReconnect();
    virtual void DownloadComplete(DownloadStatusCode status);
};

class FileDownloader: public BaseObject
{
protected:
    virtual ~FileDownloader();
public:
    //
    FileDownloader();
    FileDownloader(const String & _sourceUrl, const FilePath & _savePath, bool reload = false);
    
    // Start download file & save with same name
    // return type CURLcode
    virtual uint32 SynchDownload();
    virtual void AsynchDownload();
    
    virtual void Pause();
    virtual void Resume();
    virtual void Stop();
    
    // Getters & Setters
    virtual const String & GetSourceUrl() const;
    virtual void SetSourceUrl(const String & _sourceUrl);

    virtual const FilePath & GetSavePath() const;
    virtual void SetSavePath(const FilePath & _savePath);

    virtual uint16 GetMaxReconnect() const;
    virtual void  SetMaxReconnect(const uint16 _cnt);
    
    virtual FileDownloaderDelegate * GetDelegate() const;
    virtual void SetDelegate(FileDownloaderDelegate * _delegate);

    virtual bool GetReloadFile() const;
    virtual void SetReloadFile(const bool _reload);
    
protected:
    // Callback for cUrl write data
    static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream);

    // Method for DAVA::Message to create Thread
    void DownloadFile(BaseObject*, void*, void*);

    // Main download func 
    // return type CURLcode
    uint32 DownloadFile();
    
    // CURL download to tile
    // return type CURLcode
    uint32 CurlDownload();
    
    static bool isCURLInit;

    virtual DCURL * GetCurlHandler() const;

    enum FdState
    {
        FD_RUN = 0,
        FD_PAUSE,
        FD_STOP
    };
    
private:
    String sourceUrl;
    FilePath savePath;
    uint16 reconnectCnt;
    int16 reconnectMax;
    bool reloadFile;
    
    FileDownloaderDelegate * delegate;
    File * fileForWrite;
    
    FdState state;
    bool isPauseChangeApplied;
    DCURL *curl_handle;
};

}


#endif //__DAVAENGINE_FILE_DOWNLOADER_H__
