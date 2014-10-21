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

#ifndef __DOWNLOADER_COMMON_H__
#define __DOWNLOADER_COMMON_H__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

/*
    Task type
 */
enum DownloadType
{
    RESUMED = 0, // try to resume downllad
    FULL,        // download data even if there was a downloaded part
    GET_SIZE,    // just get size of remote file
};

/*
    Download task states
 */
enum DownloadStatus
{
    DL_PENDING = 0, // task is in pending queue
    DL_IN_PROGRESS, // task is performs now (means DownloadManager::currentTask is only one the task on that state)
    DL_FINISHED,    // task is in finished queue
    DL_UNKNOWN,     // unknow download status (means that task is just created)
};
    
/*
    All download errors which we handles
*/
enum DownloadError
{
    DLE_NO_ERROR = 0,           // there is no errors
    DLE_CANCELLED,              // download was cancelled by our side
    DLE_COULDNT_RESUME,         // seems server doesn't supports download resuming
    DLE_COULDNT_RESOLVE_HOST,   // DNS request failed and we cannot to take IP from full qualified domain name
    DLE_COULDNT_CONNECT,        // we cannot connect to given adress at given port
    DLE_CONTENT_NOT_FOUND,      // server replies that there is no requested content
    DLE_COMMON_ERROR,           // some common error which is rare and requires to debug the reason
    DLE_INIT_ERROR,             // any handles initialisation was unsuccessful
    DLE_FILE_ERROR,             // file read and write errors
    DLE_UNKNOWN,                // we cannot determine the error
};

/*
    HTTP code classes which should be handles by any application which works with HTTP
    You can take more information here https://en.wikipedia.org/wiki/List_of_HTTP_status_codes
 */
enum HttpCodeClass
{
    HTTP_INFO = 1,
    HTTP_SUCCESS,
    HTTP_REDIRECTION,
    HTTP_CLIENT_ERROR,
    HTTP_SERVER_ERROR,
};

/*
    Download task information which contains all necessery data to perform download and handle any download states
 */
struct DownloadTaskDescription
{
    DownloadTaskDescription(const String &srcUrl, const FilePath &storeToFilePath, DownloadType downloadMode, int32 _timeout, int32 _retriesCount, uint8 _partsCount);

    uint32 id;
    String url;
    FilePath storePath;
    int32 timeout;
    int32 retriesCount;
    int32 retriesLeft;
    DownloadType type;
    DownloadStatus status;
    DownloadError error;
    uint64 downloadTotal;
    uint64 downloadProgress;
    uint8 partsCount;
};
    
/*
    Contains all info which we need to know at download restore
*/
struct DownloadInfoHeader
{
    uint8 partsCount;
};

class Downloader;
struct DownloadPart
{
    /**
        \brief Fills a given downloadParts list by readed download parts state from .dlinfo file
        \param[in] infoFilePath - file with stored download info (not necessery to be created)
        \param[out] downloadParts - restored download parts
        \param[out] true if all is fine and false if there is any file read error
     */
    static bool RestoreDownload(const FilePath &infoFilePath, Vector<DownloadPart*> &downloadParts);
    /**
        \brief Save current part info into given file
        \param[in] infoFilePath - file which already should be created and have Header
        \param[out] true if all is fine and false if there is any file write or read error
     */
    bool SaveDownload(const FilePath &infoFilePath);
    /**
        \brief Creaites info file for current download and puts a header in that file.
        \param[in] infoFilePath - file path to download info file to store the header
        \param[in] partsCount - planed quantity of download parts
        \param[out] true if all is fine and false if there is any file write or read error
     */
    static bool CreateDownload(const FilePath &infoFilePath, const uint8 partsCount);

    /*
        Used to pass a pointer to current Downloader into DataReceive handler
     */
    Downloader *downloader;
    
    /*
        All the data which we store to save and thed resume download part
     */
    struct StoreData
    {
        uint8 number;
        uint64 seekPos;
        uint64 size;
        uint64 progress;
    } info;
};

}

#endif