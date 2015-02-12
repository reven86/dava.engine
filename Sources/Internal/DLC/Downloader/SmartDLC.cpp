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
//    , downloadUrl("https://dl.google.com/android/ndk/android-ndk32-r10-darwin-x86.tar.bz2")
    , downloadUrl("http://10.128.108.182/DLC_Blitz/r469302/r0-469302.adreno.patch")
//    , downloadUrl("http://dl.wargaming.net/wotblitz/dlc/r469302/r0-469302.adreno.patch")
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
    
void SmartDLC::Enable()
{
    isEnabled = true;
    DownloadManager::Instance()->SetDownloadSpeedLimit(0);
    UpdateState(START);
}
void SmartDLC::Disable()
{
    isEnabled = false;
    UpdateState(PAUSE);
    DownloadManager::Instance()->SetDownloadSpeedLimit(0);
}

void SmartDLC::UpdateState(State nextState)
{
    switch (nextState)
    {
        case START:
            if (END == currentState && 0 == currentDownloadID)
            {
                Logger::Error("DL START %d, %s", currentDownloadID,  savePath.GetAbsolutePathname().c_str());
                currentDownloadID = DownloadManager::Instance()->Download(downloadUrl, savePath, FULL, 1);
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