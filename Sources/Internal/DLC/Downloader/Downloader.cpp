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

#include "Downloader.h"
#include "DownloadManager.h"

namespace DAVA
{

Downloader::Downloader(uint32 operationTimeout)
    : timeout(operationTimeout)
{

}

size_t Downloader::SaveData(void *ptr, size_t size, size_t nmemb)
{
    DownloadManager *mgr = DownloadManager::Instance();
    
    // SaveData performs when task is in IN_PROCESS state, so currentTask should be NOT NULL.
    DVASSERT(mgr->currentTask);

    FilePath storePath = mgr->currentTask->storePath;

    size_t written = 0;
    File *destFile = File::Create(storePath, File::APPEND | File::WRITE);
    if (destFile)
    {
        written = destFile->Write(ptr, size * nmemb);
        mgr->currentTask->downloadProgress += written;
        SafeRelease(destFile);
    }

    // maybee not ideal, but only Manager can use Downloader, so maybee callback is not required.
    if (written != 0)
        DownloadManager::Instance()->ResetRetriesCount();

    return written;
}

}