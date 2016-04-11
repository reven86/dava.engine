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

#ifndef __ARCHIVE_PACK_TOOL_H__
#define __ARCHIVE_PACK_TOOL_H__

#include "Base/BaseTypes.h"
#include "FileSystem/ResourceArchive.h"
#include "CommandLineTool.h"
#include "AssetCache/AssetCache.h"

class ArchivePackTool : public CommandLineTool
{
public:
    ArchivePackTool();

private:
    enum class Source
    {
        UseDir,
        UseListFiles,
        UseSrcFiles,
        Unknown
    };

    bool ConvertOptionsToParamsInternal() override;
    void ProcessInternal() override;

    void CollectAllFilesInDirectory(const DAVA::String& pathDirName, DAVA::Vector<DAVA::String>& output);
    //void OnOneFilePacked(const DAVA::ResourceArchive::FileInfo& info);

    void ConstructCacheKey(DAVA::AssetCache::CacheItemKey& key, const DAVA::Vector<DAVA::String>& files, const DAVA::String& compression) const;
    bool RetrieveFromCache(const DAVA::AssetCache::CacheItemKey& key, const DAVA::FilePath& pathToPack, const DAVA::FilePath& pathToLog) const;
    bool AddToCache(const DAVA::AssetCache::CacheItemKey& key, const DAVA::FilePath& pathToPack, const DAVA::FilePath& pathToLog) const;

    DAVA::String compressionStr;
    DAVA::Compressor::Type compressionType;
    bool addHidden = false;
    bool useCache = false;
    DAVA::String ip;
    DAVA::uint32 port;
    DAVA::uint32 timeout;
    DAVA::String logFileName;
    DAVA::String srcDir;
    DAVA::List<DAVA::String> listFiles;
    DAVA::Vector<DAVA::String> srcFiles;
    DAVA::String packFileName;
    Source source = Source::Unknown;
};


#endif // __ARCHIVE_PACK_TOOL_H__
