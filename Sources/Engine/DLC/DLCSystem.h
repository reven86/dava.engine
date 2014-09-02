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



#ifndef __DAVAENGINE_DLC_SYSTEM_H__
#define __DAVAENGINE_DLC_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "DLC/FileDownloader.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

class DLCSystemDelegate
{
public:
    enum DLCStatusCode
    {
        DOWNLOAD_SUCCESS ,
        DOWNLOAD_ERROR,
        DOWNLOAD_SERVER_ERROR,
        DOWNLOAD_FILESYSTEM_ERROR
    };
    

public:
    virtual ~DLCSystemDelegate() = 0;

    // File with contents of all DLCs has getted 
    virtual void InitCompleted(DLCStatusCode withStatus);
    // Single DLC file has downloaded or end with error
    virtual void DLCCompleted(DLCStatusCode withStatus, uint16 index);
    // All DLC files has downloaded
    virtual void AllDLCCompleted();
};

class DLCSource;
    
class DLCSystem: public Singleton<DLCSystem>, public FileDownloaderDelegate
{
public:
    enum DLCState
    {
        DLC_NOT_DOWNLOADED = 0,
        DLC_DOWNLOADED,
        DLC_DOWNLOAD_NOT_FINISHED,
        DLC_FILES_TREE_CORRUPT,
        DLC_OLD_VERSION
    };
    
public:
    DLCSystem();
    virtual ~DLCSystem();

    // Get Index file of all DLCs
    virtual void InitSystem(const String& serverURL, const FilePath & _contentPath = "~doc:/downloads/");

    virtual uint16 GetDLCCount() const;
    // Return -1 if not found
    virtual uint16 GetDLCIndexByName(const String& name) const;
    // Return NULL if out of bounds
    virtual const DLCSource * GetDLCSource(const uint16 index);

    // Add DLC to queue to download
    virtual void DownloadDLC( const uint16 index, const uint16 _reconnectCount = 1);
    virtual void DownloadAllDLC();
    
    // Add & remove delegates
    virtual void AddDelegate(DLCSystemDelegate* delegate);
    virtual void RemoveDelegate(DLCSystemDelegate* delegate);

    // Pause & resume download
    virtual void Pause();
    virtual void Resume();
    // Stop & delete DLCSystem
    virtual bool Stop();

    // FileDownloaderDelegate methods
    virtual void DownloadGetPacket(uint64 size);
    virtual void DownloadComplete(FileDownloaderDelegate::DownloadStatusCode status);

protected:
    enum CollbackType
    {
        INIT_COMPLETE = 0,
        DOWNLOAD_COMPLETE = 1,
        ALL_DOWNLOAD_COMPLETE = 2
    };
    
protected:
    virtual void SendStatusToDelegates(const CollbackType type, const DLCSystemDelegate::DLCStatusCode code = DLCSystemDelegate::DOWNLOAD_SUCCESS, const uint16 index = 0) const;
    
    virtual void DownloadNextDLC();
    
    void LoadOldDlcs();
    void SaveOldDlcs() const;
    
    void CheckFileStructureDLC(const FilePath & path, DLCSource * _dlc);
    
private:
    String indexURL;
    FilePath indexFilePath;
    FilePath contentPath;
    Vector<DLCSystemDelegate*> delegates;
    Vector<DLCSource*> dlcs;
    Vector<DLCSource*> oldDlcs;
    Vector<DLCSource*> queueDLC;
    
    FileDownloader * currDownloader;
    bool isInited;
    bool isStoped;
};

// 
class DLCSource: public BaseObject
{
protected:
    ~DLCSource(){}
public:
    DLCSource(const YamlNode* node);
    DLCSource(File * file);
    void Save(File * file) const;
    
    static void Deleter(BaseObject * obj);
    
public:
    // Yaml fields
    String name;
    String pathOnServer;
    //
    float32 curVersion;
    float32 lastVersion;
    //
    uint64 size;
    uint64 curSize;
    //
    String description;

    // Not Yaml fields
    uint16 index;
    DLCSystem::DLCState state;
    uint16 reconnectCount;
    FilePath fullPath;
};
    
    
}

#endif//__DAVAENGINE_DLC_SYSTEM_H__












