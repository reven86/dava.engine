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

    void InitFolders(const FilePath & inputPath,const FilePath & outputPath);
    bool RecalculateDirMD5(const FilePath& pathname, const FilePath& md5file, bool isRecursive) const;
    void RecalculateMD5ForOutputDir();

    void SetConvertQuality(const TextureConverter::eConvertQuality quality);

    void SetRunning(bool arg);
    bool IsRunning() const;

    void SetCacheClientTool(const FilePath& path, const String& ip, const String& port, const String& timeout);
    void ClearCacheClientTool();

    void PackResources(eGPUFamily forGPU);

    const Set<String>& GetErrors() const;

private:
    bool RecalculateParamsMD5(const String& params, const FilePath& md5file) const;
    bool RecalculateFileMD5(const FilePath& pathname, const FilePath& md5file) const;

    bool ReadMD5FromFile(const FilePath& md5file, MD5::MD5Digest& digest) const;
    bool WriteMD5ToFile(const FilePath& md5file, const MD5::MD5Digest& digest) const;

    bool IsUsingCache() const;

    Vector<String> FetchFlags(const FilePath& flagsPathname);
    DefinitionFile* ProcessPSD(const FilePath& processDirectoryPath, const FilePath& psdPathname, const String& psdName);
    static String GetProcessFolderName();

    void AddError(const String& errorMsg);

    void RecursiveTreeWalk(const FilePath& inputPath, const FilePath& outputPath, const Vector<String>& flags = Vector<String>());

    bool GetFilesFromCache(const AssetCache::CacheItemKey& key, const FilePath& inputPath, const FilePath& outputPath);
    bool AddFilesToCache(const AssetCache::CacheItemKey& key, const FilePath& inputPath, const FilePath& outputPath);

public:
    FilePath inputGfxDirectory;
    FilePath outputGfxDirectory;
    FilePath rootDirectory;
    String gfxDirName;

    bool outputDirModified = true;

    bool isLightmapsPacking = false;
    bool forceRepack = false;
    bool clearOutputDirectory = true;
    eGPUFamily requestedGPUFamily = GPU_INVALID;
    TextureConverter::eConvertQuality quality = TextureConverter::ECQ_VERY_HIGH;

private:
    FilePath cacheClientTool;
    String cacheClientIp;
    String cacheClientPort;
    String cacheClientTimeout;
    bool isUsingCache = false;

    Set<String> errors;

    std::atomic<bool> running;
};

inline bool ResourcePacker2D::IsUsingCache() const
{
#ifdef __DAVAENGINE_WIN_UAP__
    //no cache in win uap
    return false;
#else
    return isUsingCache;
#endif
}

inline bool ResourcePacker2D::IsRunning() const
{
    return running;
}

};

#endif // __DAVAENGINE_RESOURCEPACKER2D_H__
