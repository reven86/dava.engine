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



#include "DLC/DLCSystem.h"
#include "DLC/DLCUnpacker.h"
#include "FileSystem/FileSystem.h"
#include "FileSystem/File.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/YamlNode.h"

#include <sstream>
#include <iostream>
#include <algorithm>

//

namespace DAVA
{

void DLCSystemDelegate::InitCompleted(DLCStatusCode /*withStatus*/)
{
}

void DLCSystemDelegate::DLCCompleted(DLCStatusCode /*withStatus*/, uint16 /*index*/)
{
}
	
void DLCSystemDelegate::AllDLCCompleted()
{
}

DLCSystem::DLCSystem()
    : Singleton<DLCSystem>()
    , indexURL("")
    , indexFilePath("")
    , contentPath("")
    , delegates()
    , dlcs()
    , oldDlcs()
    , queueDLC()
    , currDownloader(NULL)
    , isInited(false)
    , isStoped(false)
{
    
    
}

DLCSystem::~DLCSystem()
{
    std::for_each(dlcs.begin(), dlcs.end(), DLCSource::Deleter);
    dlcs.clear();
    
    std::for_each(oldDlcs.begin(), oldDlcs.end(), DLCSource::Deleter);
    oldDlcs.clear();
    
    queueDLC.clear();
};

void DLCSystem::InitSystem(const String& serverURL, const FilePath & _contentPath)
{
    indexURL = serverURL;
    String indexURLPath = serverURL + "IndexDLC.yaml";
    
    indexFilePath = "~doc:/downloads/";
    contentPath = _contentPath;
    FileSystem::Instance()->CreateDirectory(indexFilePath, true);
    FileSystem::Instance()->CreateDirectory(contentPath, true);
    
    currDownloader = new FileDownloader( indexURLPath, indexFilePath, true );
    currDownloader->SetDelegate(this);
//    currDownloader->SetMaxReconnect(-1);
    currDownloader->AsynchDownload();
};

void DLCSystem::AddDelegate(DLCSystemDelegate* delegate)
{
    delegates.push_back(delegate);
}

void DLCSystem::RemoveDelegate(DLCSystemDelegate* delegate)
{
    Vector<DLCSystemDelegate*>::iterator iter = delegates.begin();
    Vector<DLCSystemDelegate*>::iterator endIter = delegates.end();
    for (; iter != endIter; ++iter )
    {
        if ( delegate == *iter )
        {
            delegates.erase(iter);
            break;
        }
    }
}

// FileDownloaderDelegate implementation
void DLCSystem::DownloadGetPacket(uint64 size)
{
    if ( isInited )
    {
        DLCSource * dlc = *queueDLC.begin();
        dlc->curSize += size;

        if ( dlc->state != DLC_DOWNLOAD_NOT_FINISHED )
        {
            dlc->state = DLC_DOWNLOAD_NOT_FINISHED;
        }
    }
}

void DLCSystem::DownloadComplete(FileDownloaderDelegate::DownloadStatusCode status)
{
    SafeRelease(currDownloader);
    currDownloader = NULL;

    if ( isInited )
    {
        DLCSource * dlc = *queueDLC.begin();

        //
        queueDLC.erase( queueDLC.begin() );
        // 
        if ( status == DL_SUCCESS )
        {
            dlc->state = DLC_DOWNLOADED;
            dlc->curVersion = dlc->lastVersion;
            SendStatusToDelegates( DOWNLOAD_COMPLETE, DLCSystemDelegate::DOWNLOAD_SUCCESS, dlc->index );

            // Unpack DLC & save files paths
            FilePath fileForPaths = indexFilePath + (dlc->name + ".ka");
            DLCUnpacker::Unpack( dlc->fullPath, contentPath, fileForPaths );
            FileSystem::Instance()->DeleteFile(dlc->fullPath);
        }
        else
        {
            if ( dlc->curSize > 0 )
            {
                dlc->state = DLC_DOWNLOAD_NOT_FINISHED;
            }
            SendStatusToDelegates( DOWNLOAD_COMPLETE, DLCSystemDelegate::DOWNLOAD_ERROR, dlc->index );
        }
        
        SaveOldDlcs();

        // 
        if ( queueDLC.size() <= 0 )
        {
            SendStatusToDelegates( ALL_DOWNLOAD_COMPLETE );
            return;
        }
        
        // Call last in function to avoid thread problem
        DownloadNextDLC();
    }
    else
    {
        if ( status == DL_SUCCESS )
        {
            // Create DLC files descriptions
            YamlParser * parser = YamlParser::Create(indexFilePath + "IndexDLC.yaml");
            const YamlNode * rootNode = parser->GetRootNode()->Get( "downloads" );
            for ( uint16 roonInd = 0; roonInd < rootNode->GetCount(); ++roonInd )
            {
                DLCSource * dlcSource = new DLCSource( rootNode->Get(roonInd) );
                dlcSource->index = roonInd;
                dlcs.push_back( dlcSource );
            }
            isInited = true;
            
            // Load prev states
            LoadOldDlcs();

            for ( int32 curInd = 0; curInd  < (int32)dlcs.size(); ++curInd )
            {
                DLCSource * dlc = dlcs[curInd];
                // 
                FilePath fullSavePath(dlc->pathOnServer);
                fullSavePath.ReplaceDirectory(contentPath);
                
                dlc->fullPath = fullSavePath;
                bool isDlcFound = false;
                
                for ( int32 oldInd = 0; oldInd  < (int32)oldDlcs.size(); ++oldInd )
                {
                    DLCSource * oldDlc = oldDlcs[oldInd];
                    
                    if ( dlc->name.compare( oldDlc->name ) == 0 )
                    {
                        isDlcFound = true;
                        // Check download file to resume download
                        File * existFile = FileSystem::Instance()->CreateFileForFrameworkPath(fullSavePath, File::OPEN | File::READ);
                        
                        // Check file exist
                        if ( existFile != NULL )
                        {
                            dlc->state = DLC_DOWNLOAD_NOT_FINISHED;
                            dlc->curSize = existFile->GetSize();
                            existFile->Release();
                        }
                        
                        // Check file system
                        FilePath fileForPaths = indexFilePath + (dlc->name + ".ka");
                        CheckFileStructureDLC(fileForPaths, dlc);

                        // Check version
                        dlc->curVersion = oldDlc->curVersion;
                        if ( dlc->curVersion != dlc->lastVersion && 
                            ( dlc->state == DLC_DOWNLOADED || dlc->state == DLC_DOWNLOAD_NOT_FINISHED ) )
                        {
                            dlc->state = DLC_OLD_VERSION;
                        }
                        
                        break;
                    }
                }
                
                if ( isDlcFound == false )
                {
                    dlc->state = DLC_NOT_DOWNLOADED;
                }
            }
            
            // TODO: Delete unfound old DLCs
            
            //
            SaveOldDlcs();
            SafeRelease(parser);
            
            SendStatusToDelegates( INIT_COMPLETE, DLCSystemDelegate::DOWNLOAD_SUCCESS );
        }
        else
        {
            SendStatusToDelegates( INIT_COMPLETE, DLCSystemDelegate::DOWNLOAD_ERROR );
        }
    }
}

void DLCSystem::Pause()
{
    if ( currDownloader )
    {
        currDownloader->Pause();
    }
}

void DLCSystem::Resume()
{
    if ( currDownloader )
    {
        currDownloader->Resume();
    }
}
    
bool DLCSystem::Stop()
{
    isStoped = true;
    if ( currDownloader )
    {
        currDownloader->Stop();
        return false;
    }
    return true;
}


void DLCSystem::LoadOldDlcs()
{
    File * file = File::Create(indexFilePath + "DLCSystemStates", File::OPEN | File::READ );
    if ( file == NULL )
    {
        return;
    }

    uint16 size[1];

    file->Read( size, sizeof( uint16 ) );
    
    oldDlcs.reserve(size[0]);
    for ( uint16 dlcInd = 0; dlcInd < size[0]; ++dlcInd )
    {
        DLCSource * dlc = new DLCSource( file );
        oldDlcs.push_back( dlc );
    }
    
    SafeRelease(file);
}

void DLCSystem::SaveOldDlcs() const
{
    File * file = File::Create(indexFilePath + "DLCSystemStates", File::CREATE | File::WRITE );
    if ( file == NULL )
    {
        return;
    }
    
    uint16 size[1] = { (uint16)dlcs.size() };
    file->Write( size, sizeof(uint16) );
    
    for ( uint16 dlcInd = 0; dlcInd < dlcs.size(); ++dlcInd )
    {
        dlcs[dlcInd]->Save( file );
    }

    SafeRelease(file);
}

void DLCSystem::CheckFileStructureDLC(const FilePath & path, DLCSource * _dlc)
{
    KeyedArchive * archive = new KeyedArchive();
    archive->Load( path );
    uint32 size = archive->GetUInt32("size");
    bool isFileStructureOk = true;
    
    for ( uint32 i = 0; i < size; ++i )
    {
        String key = "";
        std::stringstream stream;
        stream << i;
        stream >> key;
        FilePath fullPathForCheck = contentPath + archive->GetString(key);
        
        File * file = File::Create(fullPathForCheck, File::OPEN | File::READ );
        if ( file == NULL )
        {
            isFileStructureOk = false;
            break;
        }
        SafeRelease(file);
    }
    
    SafeRelease(archive);
    
    if ( !isFileStructureOk )
    {
        _dlc->state = DLC_FILES_TREE_CORRUPT;
    }
    else if ( size > 0 )
    {
        _dlc->state = DLC_DOWNLOADED;
    }
}

    
void DLCSystem::SendStatusToDelegates(const CollbackType type, const DLCSystemDelegate::DLCStatusCode code, const uint16 index) const
{
    uint16 delegatesCnt = delegates.size();
    for ( uint16 i = 0; i < delegatesCnt; ++i )
    {
        switch ( type ) {
            case INIT_COMPLETE:
                delegates[i]->InitCompleted(code);
                break;
            case DOWNLOAD_COMPLETE:
                delegates[i]->DLCCompleted(code, index);
                break;
            case ALL_DOWNLOAD_COMPLETE:
                Logger::Instance()->FrameworkDebug(" DLC System finish");
                delegates[i]->AllDLCCompleted();
                break;
        }
    }
}

void DLCSystem::DownloadDLC( const uint16 index, const uint16 _reconnectCount)
{
    if ( dlcs[index]->state == DLC_DOWNLOADED )
    {
        SendStatusToDelegates( DOWNLOAD_COMPLETE, DLCSystemDelegate::DOWNLOAD_SUCCESS, index );
        return;
    }
    
    queueDLC.push_back( dlcs[index] );
    dlcs[index]->reconnectCount = _reconnectCount;
    
    if ( !currDownloader && isInited)
    {
        DownloadNextDLC();
    }
}

void DLCSystem::DownloadAllDLC()
{
    uint16 count = dlcs.size();
    for (uint16 i = 0; i < count; ++i)
    {
        DownloadDLC(i);
    }
    
    if ( queueDLC.size() <= 0 )
    {
        SendStatusToDelegates( ALL_DOWNLOAD_COMPLETE );
        return;
    }
}

void DLCSystem::DownloadNextDLC()
{
    if ( isStoped )
    {
        queueDLC.clear();
        return;
    }

    if ( queueDLC.size() <= 0 )
    {
        return;
    }
    
    DLCSource * downloadingDLC = *queueDLC.begin();
    
    String filePath = indexURL + downloadingDLC->pathOnServer;
    
    bool reload = false;
    if ( downloadingDLC->state == DLC_OLD_VERSION )
    {
        reload = true;
    }
    
    currDownloader = new FileDownloader(filePath, contentPath, reload);
    
    currDownloader->SetDelegate( this );
    currDownloader->SetMaxReconnect( downloadingDLC->reconnectCount );
    currDownloader->AsynchDownload();
}

uint16 DLCSystem::GetDLCCount() const
{
    return dlcs.size();
}

uint16 DLCSystem::GetDLCIndexByName(const std::string& name) const
{
    for ( uint16 i = 0; i < dlcs.size(); ++i)
    {
        if ( dlcs[i]->name.compare(name) == 0 )
        {
            return i;
        }
    }
    return -1;
}
    
const DLCSource * DLCSystem::GetDLCSource(const uint16 index)
{
    if ( index >= dlcs.size() )
    {
        return NULL;
    }
        
    return dlcs[index];
};
    

// DLCSource imlementation
DLCSource::DLCSource(const YamlNode* node)
    : BaseObject()
    , name("")
    , pathOnServer("")
    , curVersion(0)
    , lastVersion(0)
    , size(0)
    , curSize(0)
    , description("")
    , index(0)
    , state(DLCSystem::DLC_NOT_DOWNLOADED)
    , reconnectCount(1)
    , fullPath("")
{
    name = node->Get("name")->AsString();
    pathOnServer = node->Get("filename")->AsString();
    lastVersion = node->Get("version")->AsFloat();
    size = node->Get("size")->AsInt();
    curSize = 0;
}

DLCSource::DLCSource(File * file)
    : BaseObject()
    , name("")
    , pathOnServer("")
    , curVersion(0)
    , lastVersion(0)
    , size(0)
    , curSize(0)
    , description("")
    , index(0)
    , state(DLCSystem::DLC_NOT_DOWNLOADED)
    , reconnectCount(1)
    , fullPath("")
{
    KeyedArchive * archive = new KeyedArchive();
    archive->Load(file);
    
    name = archive->GetString("name");
    pathOnServer = archive->GetString("pathOnServer");
    curVersion = archive->GetFloat("curVersion");
    lastVersion = archive->GetFloat("lastVersion");
    size = archive->GetUInt32("size");
    
    SafeRelease(archive);
}

void DLCSource::Save(File * file) const
{
    KeyedArchive * archive = new KeyedArchive();
    
    archive->SetString("name", name);
    archive->SetString("pathOnServer", pathOnServer);
    archive->SetFloat("curVersion", curVersion);
    archive->SetFloat("lastVersion", lastVersion);
    archive->SetUInt32("size", (uint32)size);

    archive->Save(file);
    SafeRelease(archive);
}

void DLCSource::Deleter(BaseObject * obj)
{
    SafeRelease(obj);
}

    
// DLCSystemDelegate
DLCSystemDelegate::~DLCSystemDelegate(){}

    
}//END DAVA









