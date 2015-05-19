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


#ifndef __DAVAENGINE_ASSET_CACHE_CACHED_FILES_H__
#define __DAVAENGINE_ASSET_CACHE_CACHED_FILES_H__

#include "Base/BaseTypes.h"
#include "Base/Data.h"
#include "FileSystem/FilePath.h"
#include "Utils/MD5.h"

namespace DAVA
{

class KeyedArchive;
class File;
    
namespace AssetCache
{

class CachedFiles
{
    
public:
    
    CachedFiles();
    CachedFiles(const CachedFiles & right);
    
    virtual ~CachedFiles();
    
    void AddFile(const FilePath &path);
    const Map<FilePath, Data *> & GetFiles() const;
    
    bool IsEmtpy() const;
    
    void Serialize(KeyedArchive * archieve) const;
    void Deserialize(KeyedArchive * archieve);
    
    bool operator == (const CachedFiles &right) const;
    CachedFiles & operator=(const CachedFiles &right);

    void LoadFiles();
    void UnloadFiles();
    
    void Save(const FilePath & folder) const;
    
    uint64 GetFilesSize() const;
    
private:

    Map<FilePath, Data *> files;
    bool filesAreLoaded = false;
};


inline bool CachedFiles::IsEmtpy() const
{
    return (files.size() == 0);
}

inline const Map<FilePath, Data *> & CachedFiles::GetFiles() const
{
    return files;
}

    
}; // end of namespace AssetCache
}; // end of namespace DAVA

#endif // __DAVAENGINE_ASSET_CACHE_CACHED_FILES_H__

