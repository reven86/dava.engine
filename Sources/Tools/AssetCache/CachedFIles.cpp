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



#include "AssetCache/CachedFiles.h"
#include "FileSystem/KeyedArchive.h"
#include "FileSystem/FileList.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
    
namespace AssetCache
{
    
CachedFiles::CachedFiles()
{
}
    
CachedFiles::CachedFiles(const FilePath & path)
{
    if(path.IsDirectoryPathname())
    {
        ScopedPtr<FileList> flist(new FileList(path));
        
        auto count = flist->GetCount();
        for(auto i = 0; i < count; ++i)
        {
            if(!flist->IsDirectory(i))
            {
                files.insert(flist->GetPathname(i));
            }
        }
    }
    else
    {
        files.insert(path);
    }
}

    
void CachedFiles::Serialize(KeyedArchive * archieve) const
{
    DVASSERT(nullptr != archieve);
    
    auto count = files.size();
    archieve->SetUInt32("count", count);
    
    int32 index = 0;
    for(auto & file : files)
    {
        archieve->SetString(Format("file_%d", index++), file.GetAbsolutePathname());
    }
}

void CachedFiles::Deserialize(KeyedArchive * archieve)
{
    DVASSERT(nullptr != archieve);
    
    files.clear();
    
    auto count = archieve->GetUInt32("count");
    for(uint32 i = 0; i < count; ++i)
    {
        files.insert(archieve->GetString(Format("file_%d", i)));
    }
}


bool CachedFiles::operator == (const CachedFiles &cf) const
{
    return (files == cf.files);
}

    
}; // end of namespace AssetCache
}; // end of namespace DAVA

