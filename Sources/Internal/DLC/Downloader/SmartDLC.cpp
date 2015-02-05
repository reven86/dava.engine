//
//  SmartDLC.cpp
//  Framework
//
//  Created by Aleksei Kanash on 2/4/15.
//
//

#include "DLC/Downloader/SmartDLC.h"
#include "DLC/Downloader/DownloadManager.h"
#include "FileSystem/FileSystem.h"

namespace DAVA
{

SmartDLC::SmartDLC()
    : isEnabled(false)
    , downloadUrl("http://by2-badava-mac-11.corp.wargaming.local/DLC_Blitz/r469302/r0-469302.adreno.patch")
    , savePath("~doc:/test.patch")
    , currentState(END)
    , currentDownloadID(0)
{
    Logger::Error("ctor %d, %s", currentState,  savePath.GetAbsolutePathname().c_str());
}
    
SmartDLC::~SmartDLC()
{
    UpdateState(PAUSE);
}
    
void SmartDLC::Update()
{
    static uint32 i = 0;
    if (i++%50 == 0)
    {
        const DownloadStatistics * const stats = DownloadManager::Instance()->GetStatistics();
        Logger::Error("Download Speed is %3.2f Mb/s", stats->downloadSpeedBytesPerSec/1024/1024);
    }
}
    
void SmartDLC::Enable()
{
    isEnabled = true;
    UpdateState(START);
}
void SmartDLC::Disable()
{
    isEnabled = false;
    UpdateState(PAUSE);
}

void SmartDLC::UpdateState(State nextState)
{
    switch (nextState)
    {
        case START:
            if (END == currentState && 0 == currentDownloadID)
            {
                Logger::Error("DL START %d, %s", currentDownloadID,  savePath.GetAbsolutePathname().c_str());
                currentDownloadID = DownloadManager::Instance()->Download(downloadUrl, savePath, FULL);
                break;
            }
        case RESUME:
            if ((PAUSE == currentState || END == currentState) && 0 != currentDownloadID)
            {
                Logger::Error("DL RESUME %d, %s", currentDownloadID,  savePath.GetAbsolutePathname().c_str());
                FileSystem::Instance()->DeleteFile(savePath);
                DownloadManager::Instance()->Retry(currentDownloadID);
            }
            break;
        case PAUSE:
            if(0 != currentDownloadID)
            {
                Logger::Error("DL PAUSE %d, %s", currentDownloadID,  savePath.GetAbsolutePathname().c_str());
                DownloadManager::Instance()->Cancel(currentDownloadID);
            }
            break;
        case END:
            DownloadManager::Instance()->Cancel(currentDownloadID);
            currentDownloadID = 0;
            break;
        default: DVASSERT_MSG(false, "Some SmartDLC state is not handled!");
    };

    currentState = nextState;
}

}