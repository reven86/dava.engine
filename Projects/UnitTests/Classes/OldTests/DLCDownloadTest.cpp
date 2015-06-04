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


#include "DLCDownloadTest.h"
#include "DLC/Downloader/DownloadManager.h"
#include "DLC/Downloader/Downloader.h"

bool CurlTestDownloader::SaveData(const void *ptr, const FilePath& storePath, uint64 size)
{
    static int8 i = 0;
    if (0 < i++)
    {
        Interrupt();
        i = 0;
        return 0;
    }
    
    return CurlDownloader::SaveData(ptr, storePath, size);
}

DLCDownloadTest::DLCDownloadTest()
    : TestTemplate<DLCDownloadTest>("DLCDownloadTest")
    , serverUrl("by2-badava-mac-11.corp.wargaming.local")
    , testFileEmpty("/UnitTests/Downloader/empty.file")
    , testFileOne("/UnitTests/testFile.patch")
{
    RegisterFunction(this, &DLCDownloadTest::TestFunction, String("DLCDownloadTest TestFunction"), NULL);
}

void DLCDownloadTest::LoadResources()
{
    GetBackground()->SetColor(Color(1.f, 0, 0, 1));
}

void DLCDownloadTest::UnloadResources()
{
    RemoveAllControls();
}

void DLCDownloadTest::DownloadCallback(const uint32 &taskId, const DownloadStatus &status)
{
       Logger::FrameworkDebug("task %d status %d", taskId, status);
       statusToWait = status;
       taskIdToWait = taskId;
}

void DLCDownloadTest::WaitForTaskState(const uint32 &taskId, const DownloadStatus &status, const uint32 timeout)
{
    uint64 startTime = SystemTimer::Instance()->AbsoluteMS();
    while (taskIdToWait != taskId && statusToWait != status)
    {
        DownloadManager::Instance()->Update();
        Thread::Sleep(1);
        uint64 delta = SystemTimer::Instance()->AbsoluteMS() - startTime;
        if (delta >= timeout)
            return;
    }
}

String DLCDownloadTest::StorePathForUrl(const String &url)
{
    String prefix = "";

    if (url.length() >= 5)
    {
        if ("ftp" == url.substr(0, 3))
        {
            prefix = "ftp";
        }

        else if ("ftps" == url.substr(0, 4))
        {
            prefix = "ftps";
        }
        else if ("http" == url.substr(0, 4))
        {
            prefix = "http";
        }
        else if ("https" == url.substr(0, 5))
        {
            prefix = "https";
        }
    }


    return "~doc:/downloads/res/" + prefix + "_" + FilePath(url).GetFilename();
}

void DLCDownloadTest::TestFunction(PerfFuncData * data)
{
    String srcUrlMissingFile = "http://" + serverUrl + "/missingFile.txt";
    String srcUrlFolder = "http://" + serverUrl + "/";
    String srcUrlEmptyFile = "http://" + serverUrl + testFileEmpty;
    String srcUrlMissingServerName = "http://wriongServername.not.a.domain/missingFile.txt";
    String srcUrlMissingServerAddress = "http://1.1.1.1:8011/missingFile.txt";
    String srcUrl = "http://" + serverUrl + testFileOne;
    String srcUrlSSL = "https://" + serverUrl + testFileOne;
    String srcUrlFTP = "ftp://" + serverUrl + testFileOne;
    String srcUrlFTPUser = "ftp://" + serverUrl + testFileOne;

    String dstMissingFile = StorePathForUrl(srcUrlMissingFile);
    String dstHttpFolder = StorePathForUrl(srcUrlFolder);
    String dstHttpEmptyFile = StorePathForUrl(srcUrlEmptyFile);
    String dstMissingServer = StorePathForUrl(srcUrlMissingServerName);
    String dstHttp = StorePathForUrl(srcUrl);
    String dstHttps = StorePathForUrl(srcUrlSSL);
    String dstFtp = StorePathForUrl(srcUrlFTP);

    // set custom downloader - it interrupts download after one chunk of data comes
    DownloadManager::Instance()->SetDownloader(new CurlTestDownloader());

    // example "how to" usage of callback
    DownloadManager::Instance()->SetNotificationCallback(DownloadManager::NotifyFunctor(this, &DLCDownloadTest::DownloadCallback));

    DownloadStatus status;
    DownloadError error;
    uint64 progress = 0;
    uint64 total = 0;
    uint64 filesize = 0;
    File *file = NULL;

  
    // change downloader to full featured Curl downloader.
    DownloadManager::Instance()->SetDownloader(new CurlDownloader());

    // POSITIVE CHECK 
    {

        uint64 startTest1Time = SystemTimer::Instance()->AbsoluteMS();
        
        uint32 multiHttpID = DownloadManager::Instance()->Download(srcUrl, dstHttp, FULL, 4, 2);
        DownloadManager::Instance()->Wait(multiHttpID);

        DownloadManager::Instance()->GetError(multiHttpID, error);
        DownloadManager::Instance()->GetTotal(multiHttpID, total);
        
        uint64 test1Time = SystemTimer::Instance()->AbsoluteMS() - startTest1Time;
        
        uint64 startTest2Time = SystemTimer::Instance()->AbsoluteMS();
        multiHttpID = DownloadManager::Instance()->Download(srcUrl, dstHttp, FULL, 1, 2);
        DownloadManager::Instance()->Wait(multiHttpID);
        
        DownloadManager::Instance()->GetError(multiHttpID, error);
        DownloadManager::Instance()->GetTotal(multiHttpID, total);

        uint64 test2Time = SystemTimer::Instance()->AbsoluteMS() - startTest2Time;
        
        Logger::FrameworkDebug("time for 4 threads = %d", test1Time);
        Logger::FrameworkDebug("time for 1 thread1 = %d", test2Time);
        
        // FULL DOWNLOAD
        // Make sure that file is missing
        FileSystem::Instance()->DeleteFile(dstHttp); 
        uint32 httpID = DownloadManager::Instance()->Download(srcUrl, dstHttp, FULL, 1, 1);
        DownloadManager::Instance()->Wait(httpID);
        TEST_VERIFY(DownloadManager::Instance()->GetProgress(httpID, progress));
        TEST_VERIFY(DownloadManager::Instance()->GetTotal(httpID, total));
        file = File::Create(dstHttp, File::OPEN | File::READ);
        TEST_VERIFY(NULL != file);
        if (NULL != file)
        {
            filesize = file->GetSize();
            TEST_VERIFY(filesize == progress);
            TEST_VERIFY(total == progress);
            SafeRelease(file);
        }

        // retry finished download
        DownloadManager::Instance()->Retry(httpID);
        DownloadManager::Instance()->GetStatus(httpID, status);
        TEST_VERIFY(DL_PENDING == status);

        WaitForTaskState(httpID, DL_IN_PROGRESS, 1000);
        DownloadManager::Instance()->Cancel(httpID);
        WaitForTaskState(httpID, DL_FINISHED, 1000);
        DownloadManager::Instance()->GetStatus(httpID, status);
        TEST_VERIFY(DL_FINISHED == status);
        DownloadManager::Instance()->GetError(httpID, error);
        TEST_VERIFY(DLE_CANCELLED == error);
        
        // retry task with error
        DownloadManager::Instance()->Retry(httpID);
        DownloadManager::Instance()->GetStatus(httpID, status);
        TEST_VERIFY(DL_PENDING == status);
        DownloadManager::Instance()->GetError(httpID, error);
        TEST_VERIFY(DLE_NO_ERROR == error);

        // retry pending task
        DownloadManager::Instance()->Retry(httpID);
        DownloadManager::Instance()->GetStatus(httpID, status);
        TEST_VERIFY(DL_PENDING == status);
        DownloadManager::Instance()->GetError(httpID, error);
        TEST_VERIFY(DLE_NO_ERROR == error);

        FileSystem::Instance()->DeleteFile(dstHttp);
    }
    
    {
        // RESUMED DOWNLOAD
        // Make sure that file is missing
        FileSystem::Instance()->DeleteFile(dstHttp);
        uint32 httpID = DownloadManager::Instance()->Download(srcUrl, dstHttp, RESUMED, 1, 1);
        DownloadManager::Instance()->Wait(httpID);
        TEST_VERIFY(DownloadManager::Instance()->GetProgress(httpID, progress));
        TEST_VERIFY(DownloadManager::Instance()->GetTotal(httpID, total));
        file = File::Create(dstHttp, File::OPEN | File::READ);
        TEST_VERIFY(NULL != file);
        if (NULL != file)
        {
            filesize = file->GetSize();
            TEST_VERIFY(filesize == progress);
            TEST_VERIFY(total == progress);
            SafeRelease(file);
        }
        FileSystem::Instance()->DeleteFile(dstHttp);
    }
// END POSITIVE CHECK

// MISSING FILE ON EXISTENT SERVER

    uint32 missingFileId = DownloadManager::Instance()->Download(srcUrlMissingFile, dstMissingFile, RESUMED, 1, 1, 0);
    DownloadManager::Instance()->Wait(missingFileId);

    DownloadManager::Instance()->GetError(missingFileId, error);
    TEST_VERIFY(DLE_CONTENT_NOT_FOUND == error);
    DownloadManager::Instance()->GetStatus(missingFileId, status);
    TEST_VERIFY(DL_FINISHED == status);

    // file should not be created
    file = File::Create(dstMissingFile, File::OPEN | File::READ);
    TEST_VERIFY(NULL == file);
    if (NULL != file)
    {
        FileSystem::Instance()->DeleteFile(dstMissingFile);
        SafeRelease(file);
    }

    // cancel finished download should not work
    DownloadManager::Instance()->Cancel(missingFileId);
    DownloadManager::Instance()->GetStatus(missingFileId, status);
    TEST_VERIFY(DL_FINISHED == status);
    DownloadManager::Instance()->GetError(missingFileId, error);
    TEST_VERIFY(DLE_CONTENT_NOT_FOUND == error);

// END MISSING FILE ON EXISTENT SERVER
    
// FOLDER INSTEAD OF FILE 

    uint32 folderId = DownloadManager::Instance()->Download(srcUrlFolder, dstHttpFolder, RESUMED, 1, 1, 0);
    DownloadManager::Instance()->Wait(folderId);

    DownloadManager::Instance()->GetError(folderId, error);
    TEST_VERIFY(DLE_NO_ERROR == error);
    DownloadManager::Instance()->GetStatus(folderId, status);
    TEST_VERIFY(DL_FINISHED == status);

    FileSystem::Instance()->DeleteFile(dstHttpFolder);

// END FOLDER INSTEAD OF FILE

// EMPTY FILE

    uint32 emptyFileId = DownloadManager::Instance()->Download(srcUrlEmptyFile, dstHttpEmptyFile, RESUMED, 1, 1, 1);
    DownloadManager::Instance()->Wait(emptyFileId);
    TEST_VERIFY(DownloadManager::Instance()->GetProgress(emptyFileId, progress));
    TEST_VERIFY(DownloadManager::Instance()->GetTotal(emptyFileId, total));
    file = File::Create(dstHttpEmptyFile, File::OPEN | File::READ);
    TEST_VERIFY(NULL != file);
    if (NULL != file)
    {
        filesize = file->GetSize();
        TEST_VERIFY(filesize == progress);
        TEST_VERIFY(total == progress);
        SafeRelease(file);
    }
    FileSystem::Instance()->DeleteFile(dstHttpEmptyFile);

// END EMPTY FILE 

// DIFFERENT FILE SIZES

    uint64 remoteFileSize;

    // RESUMING 
    {
        // remote size < local size 

        uint32 getSmallerFileSizeId = DownloadManager::Instance()->Download(srcUrl, "", GET_SIZE, 1, 1, 1);
        DownloadManager::Instance()->Wait(getSmallerFileSizeId);

        if (DownloadManager::Instance()->GetTotal(getSmallerFileSizeId, remoteFileSize))
        {
            // create new file with size > remoteFileSize
            File *f = File::Create(dstHttp, File::CREATE | File::WRITE);
            if (f)
            {
                // generate file with lagrer size than remote
                uint8 *buf = new uint8[static_cast<uint32>(remoteFileSize) + 1]; // values is not important
                f->Write(buf, static_cast<uint32>(remoteFileSize) + 1);
                SafeRelease(f);
                SafeDeleteArray(buf);

                uint32 smallerFileId = DownloadManager::Instance()->Download(srcUrl, dstHttp, RESUMED, 1, 1, 1);
                DownloadManager::Instance()->Wait(smallerFileId);

                DownloadManager::Instance()->GetTotal(smallerFileId, total);
                DownloadManager::Instance()->GetProgress(smallerFileId, progress);

                file = File::Create(dstHttp, File::OPEN | File::READ);
                TEST_VERIFY(NULL != file);
                if (NULL != file)
                {
                    filesize = file->GetSize();
                    TEST_VERIFY(filesize == progress);
                    TEST_VERIFY(total == progress);
                    SafeRelease(file);
                }
                FileSystem::Instance()->DeleteFile(dstHttp);
            }
        }

        // remote size > local size 

        uint32 getLargerFileSizeId = DownloadManager::Instance()->Download(srcUrl, "", GET_SIZE, 1, 1, 1);
        DownloadManager::Instance()->Wait(getLargerFileSizeId);

        if (DownloadManager::Instance()->GetTotal(getLargerFileSizeId, remoteFileSize))
        {
            // create new file with size > remoteFileSize
            File *f = File::Create(dstHttp, File::CREATE | File::WRITE);
            if (f)
            {
                // generate file with smaller size than remote
                uint8 *buf = new uint8[static_cast<uint32>(remoteFileSize) + 1]; // values is not important
                f->Write(buf, static_cast<uint32>(remoteFileSize) - 1);
                SafeRelease(f);
                SafeDeleteArray(buf);

                uint32 largerFileId = DownloadManager::Instance()->Download(srcUrl, dstHttp, RESUMED, 1, 1, 1);
                DownloadManager::Instance()->Wait(largerFileId);

                DownloadManager::Instance()->GetTotal(largerFileId, total);
                DownloadManager::Instance()->GetProgress(largerFileId, progress);

                file = File::Create(dstHttp, File::OPEN | File::READ);
                TEST_VERIFY(NULL != file);
                if (NULL != file)
                {
                    filesize = file->GetSize();
                    TEST_VERIFY(filesize == progress);
                    TEST_VERIFY(total == progress);
                    SafeRelease(file);
                }
                FileSystem::Instance()->DeleteFile(dstHttp);
            }
        }

        // remote size == local size

        uint32 getEqualFileSizeId = DownloadManager::Instance()->Download(srcUrl, "", GET_SIZE, 1, 1, 1);
        DownloadManager::Instance()->Wait(getEqualFileSizeId);

        if (DownloadManager::Instance()->GetTotal(getEqualFileSizeId, remoteFileSize))
        {
            // create new file with size > remoteFileSize
            File *f = File::Create(dstHttp, File::CREATE | File::WRITE);
            if (f)
            {
                // generate file with smaller size than remote
                uint8 *buf = new uint8[static_cast<uint32>(remoteFileSize)]; // values is not important
                f->Write(buf, static_cast<uint32>(remoteFileSize));
                SafeRelease(f);
                SafeDeleteArray(buf);

                uint32 equalFileId = DownloadManager::Instance()->Download(srcUrl, dstHttp, RESUMED, 1, 1, 1);
                DownloadManager::Instance()->Wait(equalFileId);

                DownloadManager::Instance()->GetTotal(equalFileId, total);
                DownloadManager::Instance()->GetProgress(equalFileId, progress);

                file = File::Create(dstHttp, File::OPEN | File::READ);
                TEST_VERIFY(NULL != file);
                if (NULL != file)
                {
                    filesize = file->GetSize();
                    TEST_VERIFY(filesize == progress);
                    TEST_VERIFY(total == progress);
                    SafeRelease(file);
                }
                FileSystem::Instance()->DeleteFile(dstHttp);
            }
        }
    }

    // FULL RELOAD
    {
        uint32 getSmallerFileSizeId = DownloadManager::Instance()->Download(srcUrl, "", GET_SIZE, 1, 1, 1);
        DownloadManager::Instance()->Wait(getSmallerFileSizeId);

        if (DownloadManager::Instance()->GetTotal(getSmallerFileSizeId, remoteFileSize))
        {
            // create new file with size > remoteFileSize
            File *f = File::Create(dstHttp, File::CREATE | File::WRITE);
            if (f)
            {
                // generate file with lagrer size than remote
                uint8 *buf = new uint8[static_cast<uint32>(remoteFileSize) + 1]; // values is not important
                f->Write(buf, static_cast<uint32>(remoteFileSize) + 1);
                SafeRelease(f);
                SafeDeleteArray(buf);

                uint32 smallerFileId = DownloadManager::Instance()->Download(srcUrl, dstHttp, FULL, 1, 1, 1);
                DownloadManager::Instance()->Wait(smallerFileId);

                DownloadManager::Instance()->GetTotal(smallerFileId, total);
                DownloadManager::Instance()->GetProgress(smallerFileId, progress);

                file = File::Create(dstHttp, File::OPEN | File::READ);
                TEST_VERIFY(NULL != file);
                if (NULL != file)
                {
                    filesize = file->GetSize();
                    TEST_VERIFY(filesize == progress);
                    TEST_VERIFY(total == progress);
                    SafeRelease(file);
                }
                FileSystem::Instance()->DeleteFile(dstHttp);
            }
        }

        // remote size > local size

        uint32 getLargerFileSizeId = DownloadManager::Instance()->Download(srcUrl, "", GET_SIZE, 1, 1, 1);
        DownloadManager::Instance()->Wait(getLargerFileSizeId);

        if (DownloadManager::Instance()->GetTotal(getLargerFileSizeId, remoteFileSize))
        {
            // create new file with size > remoteFileSize
            File *f = File::Create(dstHttp, File::CREATE | File::WRITE);
            if (f)
            {
                // generate file with smaller size than remote
                uint8 *buf = new uint8[static_cast<uint32>(remoteFileSize) + 1]; // values is not important
                f->Write(buf, static_cast<uint32>(remoteFileSize) - 1);
                SafeRelease(f);
                SafeDeleteArray(buf);

                uint32 largerFileId = DownloadManager::Instance()->Download(srcUrl, dstHttp, FULL, 1, 1, 1);
                DownloadManager::Instance()->Wait(largerFileId);

                DownloadManager::Instance()->GetTotal(largerFileId, total);
                DownloadManager::Instance()->GetProgress(largerFileId, progress);

                file = File::Create(dstHttp, File::OPEN | File::READ);
                TEST_VERIFY(NULL != file);
                if (NULL != file)
                {
                    filesize = file->GetSize();
                    TEST_VERIFY(filesize == progress);
                    TEST_VERIFY(total == progress);
                    SafeRelease(file);
                }
                FileSystem::Instance()->DeleteFile(dstHttp);
            }
        }

        // remote size == local size

        uint32 getEqualFileSizeId = DownloadManager::Instance()->Download(srcUrl, "", GET_SIZE, 1, 1, 1);
        DownloadManager::Instance()->Wait(getEqualFileSizeId);

        if (DownloadManager::Instance()->GetTotal(getEqualFileSizeId, remoteFileSize))
        {
            // create new file with size > remoteFileSize
            File *f = File::Create(dstHttp, File::CREATE | File::WRITE);
            if (f)
            {
                // generate file with smaller size than remote
                uint8 *buf = new uint8[static_cast<uint32>(remoteFileSize)]; // values is not important
                f->Write(buf, static_cast<uint32>(remoteFileSize));
                SafeRelease(f);
                SafeDeleteArray(buf);

                uint32 equalFileId = DownloadManager::Instance()->Download(srcUrl, dstHttp, FULL, 1, 1, 1);
                DownloadManager::Instance()->Wait(equalFileId);

                DownloadManager::Instance()->GetTotal(equalFileId, total);
                DownloadManager::Instance()->GetProgress(equalFileId, progress);

                file = File::Create(dstHttp, File::OPEN | File::READ);
                TEST_VERIFY(NULL != file);
                if (NULL != file)
                {
                    filesize = file->GetSize();
                    TEST_VERIFY(filesize == progress);
                    TEST_VERIFY(total == progress);
                    SafeRelease(file);
                }
                FileSystem::Instance()->DeleteFile(dstHttp);
            }
        }
    }


// END DIFFERENT FILE SIZES

// TIMEOUTS

    int32 timeout;
    int32 retries;
    uint64 startTime;
    uint64 delta;

    // MISSING SERVER NAME
    timeout = 2;
    retries = 1;

    uint32 missingServerNameID = DownloadManager::Instance()->Download(srcUrlMissingServerName, dstMissingServer, RESUMED, 1, timeout, retries);
    DownloadManager::Instance()->Wait(missingServerNameID);

    DownloadManager::Instance()->GetError(missingServerNameID, error);
    TEST_VERIFY(DLE_COULDNT_RESOLVE_HOST == error);
    DownloadManager::Instance()->GetStatus(missingServerNameID, status);
    TEST_VERIFY(DL_FINISHED == status);

    // MISSING SERVER NAME
    timeout = 1;
    retries = 0;

    uint32 missingServerAddressID = DownloadManager::Instance()->Download(srcUrlMissingServerAddress, dstMissingServer, RESUMED, 1, timeout, retries);

    startTime = SystemTimer::Instance()->AbsoluteMS();
    DownloadManager::Instance()->Wait(missingServerAddressID);
    delta = SystemTimer::Instance()->AbsoluteMS() - startTime;

    TEST_VERIFY(delta >= timeout*(retries+1));
    DownloadManager::Instance()->GetError(missingServerAddressID, error);
    TEST_VERIFY(DLE_COULDNT_CONNECT == error);
    DownloadManager::Instance()->GetStatus(missingServerAddressID, status);
    TEST_VERIFY(DL_FINISHED == status);


    // MISSING FILE ON THE SERVER
    timeout = 1;
    retries = 1;

    uint32 missingID = DownloadManager::Instance()->Download(srcUrlMissingFile, dstMissingFile, FULL, 1, timeout, retries);
    DownloadManager::Instance()->Wait(missingID);
    TEST_VERIFY(DownloadManager::Instance()->GetProgress(missingID, progress));
    TEST_VERIFY(DownloadManager::Instance()->GetTotal(missingID, total));
    DownloadManager::Instance()->GetError(missingID, error);
    TEST_VERIFY(DLE_CONTENT_NOT_FOUND == error);
    DownloadManager::Instance()->GetStatus(missingID, status);
    TEST_VERIFY(DL_FINISHED == status);

    // File exists in file system but not exists on remote server
    File *f = File::Create(dstMissingFile, File::CREATE | File::WRITE);
    if (f)
    {
        uint8 t = 1;
        f->Write(&t, sizeof(t));
        SafeRelease(f);

        uint32 missingID = DownloadManager::Instance()->Download(srcUrlMissingFile, dstMissingFile, RESUMED, 1, timeout, retries);
        DownloadManager::Instance()->Wait(missingID);
        TEST_VERIFY(DownloadManager::Instance()->GetProgress(missingID, progress));
        TEST_VERIFY(DownloadManager::Instance()->GetTotal(missingID, total));
        DownloadManager::Instance()->GetError(missingID, error);
        TEST_VERIFY(DLE_CONTENT_NOT_FOUND == error);
        DownloadManager::Instance()->GetStatus(missingID, status);
        TEST_VERIFY(DL_FINISHED == status);

        FileSystem::Instance()->DeleteFile(dstMissingFile);
    }



// END TIMEOUTS





// DOWNLOAD QUEUES

{
    uint32 httpID = DownloadManager::Instance()->Download(srcUrl, dstHttp, RESUMED, 1, 2, 2);
    uint32 missingFileId = DownloadManager::Instance()->Download(srcUrlMissingFile, dstMissingFile, RESUMED, 1, 2, 2);
    uint32 folderId = DownloadManager::Instance()->Download(srcUrlFolder, dstHttpFolder, RESUMED, 1, 2, 2);
    uint32 emptyFileId = DownloadManager::Instance()->Download(srcUrlEmptyFile, dstHttpEmptyFile, RESUMED, 1, 2, 2);



    WaitForTaskState(httpID, DL_IN_PROGRESS, 1000);
    // Check if only one task is in the progress
    DownloadManager::Instance()->GetStatus(httpID, status);
    TEST_VERIFY(DL_IN_PROGRESS == status);
    DownloadManager::Instance()->GetStatus(missingFileId, status);
    TEST_VERIFY(DL_PENDING == status);
    DownloadManager::Instance()->GetStatus(folderId, status);
    TEST_VERIFY(DL_PENDING == status);
    DownloadManager::Instance()->GetStatus(emptyFileId, status);
    TEST_VERIFY(DL_PENDING == status);

    // retry current task should not work
    DownloadManager::Instance()->Retry(httpID);
    DownloadManager::Instance()->GetStatus(httpID, status);
    TEST_VERIFY(DL_IN_PROGRESS == status);

    // check that pending task could be cancelled
    DownloadManager::Instance()->Cancel(missingFileId);

    DownloadManager::Instance()->GetStatus(httpID, status);
    TEST_VERIFY(DL_IN_PROGRESS == status);
    DownloadManager::Instance()->GetStatus(missingFileId, status);
    TEST_VERIFY(DL_FINISHED == status);
    DownloadManager::Instance()->GetError(missingFileId, error);
    TEST_VERIFY(DLE_CANCELLED == error);
    DownloadManager::Instance()->GetStatus(folderId, status);
    TEST_VERIFY(DL_PENDING == status);
    DownloadManager::Instance()->GetStatus(emptyFileId, status);
    TEST_VERIFY(DL_PENDING == status);

    // check retry pending task
    DownloadManager::Instance()->Retry(folderId);
    DownloadManager::Instance()->GetStatus(folderId, status);
    TEST_VERIFY(DL_PENDING == status);

    // cancel task in progress
    DownloadManager::Instance()->Cancel(httpID);
    WaitForTaskState(httpID, DL_FINISHED, 1000);
    DownloadManager::Instance()->GetStatus(httpID, status);
    TEST_VERIFY(DL_FINISHED == status);
    DownloadManager::Instance()->GetError(httpID, error);
    TEST_VERIFY(DLE_CANCELLED == error);

    // cancel cancelled task
    DownloadManager::Instance()->Cancel(httpID);
    DownloadManager::Instance()->GetStatus(httpID, status);
    TEST_VERIFY(DL_FINISHED == status);
    DownloadManager::Instance()->GetError(httpID, error);
    TEST_VERIFY(DLE_CANCELLED == error);

    // retry cancelled task
    DownloadManager::Instance()->Retry(httpID);
    DownloadManager::Instance()->GetStatus(httpID, status);
    TEST_VERIFY(DL_PENDING == status);

    DownloadManager::Instance()->Cancel(httpID);
    DownloadManager::Instance()->Wait(httpID);

    DownloadManager::Instance()->WaitAll();

    FileSystem::Instance()->DeleteFile(dstHttp);
    FileSystem::Instance()->DeleteFile(dstHttpFolder);
    FileSystem::Instance()->DeleteFile(dstHttpEmptyFile);
}

{
    //Check Cancel All

    // finished task
    uint32 httpFinishedID = DownloadManager::Instance()->Download(srcUrl, dstHttp, RESUMED, 1, 2, 2);
    DownloadManager::Instance()->Wait(httpFinishedID);
    FileSystem::Instance()->DeleteFile(dstHttp);

    DownloadManager::Instance()->GetStatus(httpFinishedID, status);
    TEST_VERIFY(DL_FINISHED == status);
    DownloadManager::Instance()->GetError(httpFinishedID, error);
    TEST_VERIFY(DLE_NO_ERROR == error);

    // error result task
    uint32 missingFileId = DownloadManager::Instance()->Download(srcUrlMissingFile, dstMissingFile, RESUMED, 1, 2, 2);
    DownloadManager::Instance()->Wait(missingFileId);

    uint32 httpCancelledID = DownloadManager::Instance()->Download(srcUrl, dstHttp, RESUMED, 1, 2, 2);
    WaitForTaskState(httpCancelledID, DL_IN_PROGRESS, 1000);
    DownloadManager::Instance()->Cancel(httpCancelledID);
    FileSystem::Instance()->DeleteFile(dstHttp);

    uint32 httpInProcessID = DownloadManager::Instance()->Download(srcUrl, dstHttp, RESUMED, 1, 2, 2);
    WaitForTaskState(httpInProcessID, DL_IN_PROGRESS, 1000);

    DownloadManager::Instance()->CancelAll();

    DownloadManager::Instance()->GetStatus(httpFinishedID, status);
    TEST_VERIFY(DL_FINISHED == status);
    DownloadManager::Instance()->GetError(httpFinishedID, error);
    TEST_VERIFY(DLE_NO_ERROR == error);

    DownloadManager::Instance()->GetStatus(missingFileId, status);
    TEST_VERIFY(DL_FINISHED == status);
    DownloadManager::Instance()->GetError(missingFileId, error);
    TEST_VERIFY(DLE_CONTENT_NOT_FOUND == error);

    DownloadManager::Instance()->GetStatus(httpCancelledID, status);
    TEST_VERIFY(DL_FINISHED == status);
    DownloadManager::Instance()->GetError(httpCancelledID, error);
    TEST_VERIFY(DLE_CANCELLED == error);

    DownloadManager::Instance()->GetStatus(httpInProcessID, status);
    TEST_VERIFY(DL_FINISHED == status);
    DownloadManager::Instance()->GetError(httpInProcessID, error);
    TEST_VERIFY(DLE_CANCELLED == error);

    DownloadManager::Instance()->Wait(httpInProcessID);
    FileSystem::Instance()->DeleteFile(dstHttp);

}

// END DOWNLOAD QUEUES

// GET SIZE
    {
    // finished task
    uint32 httpFinishedID = DownloadManager::Instance()->Download(srcUrl, dstHttp, GET_SIZE, 1, 2, 2);
    DownloadManager::Instance()->Wait(httpFinishedID);

    DownloadManager::Instance()->GetStatus(httpFinishedID, status);
    TEST_VERIFY(DL_FINISHED == status);
    DownloadManager::Instance()->GetError(httpFinishedID, error);
    TEST_VERIFY(DLE_NO_ERROR == error);

    // error result task
    uint32 missingServerId = DownloadManager::Instance()->Download(srcUrlMissingServerAddress, dstMissingServer, GET_SIZE, 1, 4, 2);

    WaitForTaskState(missingServerId, DL_IN_PROGRESS, 1000);
    uint64 startTime = SystemTimer::Instance()->AbsoluteMS();

    // task should be not finished (timeout is larger)
    DownloadManager::Instance()->GetStatus(missingServerId, status);
    TEST_VERIFY(DL_IN_PROGRESS == status);
    DownloadManager::Instance()->GetError(missingServerId, error);
    TEST_VERIFY(DLE_NO_ERROR == error);

    DownloadManager::Instance()->Wait(missingServerId);

    // task is finished, timeout is ok, status is predictable

    uint64 delta = SystemTimer::Instance()->AbsoluteMS() - startTime;
    TEST_VERIFY(delta >= 2 * 2);
    DownloadManager::Instance()->GetStatus(missingServerId, status);
    TEST_VERIFY(DL_FINISHED == status);
    DownloadManager::Instance()->GetError(missingServerId, error);
    TEST_VERIFY(DLE_COULDNT_CONNECT == error);

    uint32 emptyFileId = DownloadManager::Instance()->Download(srcUrlEmptyFile, dstHttpEmptyFile, GET_SIZE, 1, 2, 2);
    DownloadManager::Instance()->Wait(emptyFileId);

    DownloadManager::Instance()->GetTotal(emptyFileId, total);
    TEST_VERIFY(0 == total);
    DownloadManager::Instance()->GetStatus(emptyFileId, status);
    TEST_VERIFY(DL_FINISHED == status);
    DownloadManager::Instance()->GetError(emptyFileId, error);
    TEST_VERIFY(DLE_NO_ERROR == error);

    }

// END GET SIZE


// SET DOWNLOADER TEST

    // we don't need to full featured downloader here
    DownloadManager::Instance()->SetDownloader(new CurlTestDownloader);
    uint32 httpId = DownloadManager::Instance()->Download(srcUrl, dstHttp, FULL);
    DownloadManager::Instance()->Retry(httpId);
    DownloadManager::Instance()->Wait(httpId);

    DownloadManager::Instance()->GetProgress(httpId, progress);
    DownloadManager::Instance()->GetTotal(httpId, total);
    file = File::Create(dstHttp, File::OPEN | File::READ);
    TEST_VERIFY(NULL != file);
    if (NULL != file)
    {
        filesize = file->GetSize();
    }
    TEST_VERIFY(filesize == progress);
    TEST_VERIFY(total == progress);
    SafeRelease(file);
    FileSystem::Instance()->DeleteFile(dstHttp);

// END SET DOWNLOADER TEST
}

