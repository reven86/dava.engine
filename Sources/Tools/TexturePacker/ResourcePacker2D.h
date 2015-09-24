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


#ifndef __DAVAENGINE_RESOURCEPACKER2D_H__
#define __DAVAENGINE_RESOURCEPACKER2D_H__

#include "Base/BaseTypes.h"
#include "Render/RenderBase.h"
#include "FileSystem/FilePath.h"
#include "TextureCompression/TextureConverter.h"
#include <atomic>

#include "AssetCache/CacheItemKey.h"

namespace DAVA
{

class DefinitionFile;
class YamlNode;

class ResourcePacker2D
{
    static const String VERSION;

public:
    using FILESMAP = std::map<String, String>;
    ResourcePacker2D();

    void InitFolders(const FilePath & inputPath,const FilePath & outputPath);
    void PackResources(eGPUFamily forGPU);
    void RecalculateMD5ForOutputDir();
    bool RecalculateDirMD5(const FilePath& pathname, const FilePath& md5file, bool isRecursive) const;
    bool RecalculateFileMD5(const FilePath& pathname, const FilePath& md5file) const;

    DefinitionFile * ProcessPSD(const FilePath & processDirectoryPath, const FilePath & psdPathname, const String & psdName, bool twoSideMargin, uint32 texturesMargin);
    Vector<String> FetchFlags(const FilePath & flagsPathname);

    static String GetProcessFolderName();

    void SetConvertQuality(const TextureConverter::eConvertQuality quality);
    void SetRunning(bool arg);
    bool IsRunning() const;

    void SetCacheClientTool(const FilePath& path, const String& ip, const String& port, const String& timeout);
    void ClearCacheClientTool();
    bool IsUsingCache() const;

    const Set<String>& GetErrors() const;

protected:
    void AddError(const String& errorMsg);

    void RecursiveTreeWalk(const FilePath& inputPath, const FilePath& outputPath, const Vector<String>& flags = Vector<String>());
    bool isRecursiveFlagSet(const Vector<String>& flags);

    bool GetFilesFromCache(const AssetCache::CacheItemKey& key, const FilePath& inputPath, const FilePath& outputPath);
    bool AddFilesToCache(const AssetCache::CacheItemKey& key, const FilePath& inputPath, const FilePath& outputPath);

public:
    FilePath inputGfxDirectory;
    FilePath outputGfxDirectory;
    FilePath excludeDirectory;
    String gfxDirName;
    
    bool isGfxModified;
    
    bool isLightmapsPacking;
    bool forceRepack;
    bool clearOutputDirectory;
    eGPUFamily requestedGPUFamily;
    TextureConverter::eConvertQuality quality;
    FILESMAP spriteFiles;

protected:
    FilePath cacheClientTool;
    String cacheClientIp;
    String cacheClientPort;
    String cacheClientTimeout;

    Set<String> errors;

private:
    std::atomic<bool> running;
};

inline bool ResourcePacker2D::IsUsingCache() const
{
#ifdef __DAVAENGINE_WIN_UAP__
    //no cache in win uap
    return false;
#else
    return (!cacheClientTool.IsEmpty());
#endif
}

inline bool ResourcePacker2D::IsRunning() const
{
    return running;
}

};

#endif // __DAVAENGINE_RESOURCEPACKER2D_H__
