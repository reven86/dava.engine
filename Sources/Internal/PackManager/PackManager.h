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
#pragma once

#include "Functional/Signal.h"
#include "DLC/Downloader/DownloaderCommon.h"

namespace DAVA
{
class PackManagerImpl;

class PackManager final
{
public:
    struct Pack
    {
        enum class Status : uint32
        {
            NotRequested = 0,
            Requested = 1,
            Downloading = 2,
            Mounted = 3,
            ErrorLoading = 4, // downloadError - value returned from DLC DownloadManager
            OtherError = 5 // mount failed, check hash failed, file IO failed see otherErrorMsg
        };

        enum class Change : uint32
        {
            State = 1,
            DownloadProgress = 2,
            Priority = 4,
        };

        Vector<String> dependency; // names of dependency packs or empty

        String name; // unique pack name
        String remoteUrl; // url used for download archive or empty
        String otherErrorMsg;

        float32 downloadProgress = 0.f; // 0.0f to 1.0f
        float32 priority = 0.f; // 0.0f to 1.0f

        uint32 hashFromMeta = 0; // example: tanks.pak -> tanks.pak.hash
        uint32 hashFromDB = 0;

        DownloadError downloadError = DLE_NO_ERROR;
        Status state = Status::NotRequested;

        bool isGPU = false;
    };

    PackManager();
    ~PackManager();

    // 1. open local database and read all packs info
    // 2. list all packs on filesystem
    // 3. mount all packs which found on filesystem and in database
    // throw exception if can't initialize with deteils
    void Initialize(const FilePath& dbFile,
                    const FilePath& localPacksDir,
                    const String& commonPacksUrl,
                    const String& gpuPacksUrl);

    bool IsProcessingEnabled() const;
    // enable user request processing
    void EnableProcessing();
    // disalbe user request processing
    void DisableProcessing();

    // internal method called per frame in framework (can thow exception)
    void Update();

    // return unique pack name or empty string
    const String& FindPack(const FilePath& relativePathInArchive) const;

    // thow exception if can't find pack
    const Pack& RequestPack(const String& packName, float priority = 0.0f);

    // all packs state, valid till next call Update()
    const Vector<Pack>& GetPacks() const;

    void Delete(const String& packName);

    // отслеживание статуса запросов
    Signal<const Pack&, Pack::Change> onPackStateChanged;

    const FilePath& GetLocalPacksDirectory() const;
    const String& GetRemotePacksUrl() const;

private:
    std::unique_ptr<PackManagerImpl> impl;
};

} // end namespace DAVA
