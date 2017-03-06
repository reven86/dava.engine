#ifndef __DAVAENGINE_RESOURCEPACKER2D_H__
#define __DAVAENGINE_RESOURCEPACKER2D_H__

#include "Tools/TextureCompression/TextureConverter.h"
#include "Tools/TexturePacker/Spritesheet.h"
#include "Tools/AssetCache/AssetCacheClient.h"

#include <Base/BaseTypes.h>
#include <Render/RenderBase.h>
#include <FileSystem/FilePath.h>

#include <atomic>

namespace DAVA
{
class DefinitionFile;
class YamlNode;
class AssetCacheClient;

class ResourcePacker2D
{
    static const String VERSION;
    static const String INTERNAL_LIBPSD_VERSION;

public:
    void InitFolders(const FilePath& inputPath, const FilePath& outputPath);
    bool RecalculateDirMD5(const FilePath& pathname, const FilePath& md5file, bool isRecursive) const;
    void RecalculateMD5ForOutputDir();

    void SetConvertQuality(const TextureConverter::eConvertQuality quality);

    void SetRunning(bool arg);
    bool IsRunning() const;

    void SetCacheClient(AssetCacheClient* cacheClient, const String& comment);
    bool IsUsingCache() const;

    void SetTexturePostfix(const String& postfix);

    void PackResources(const Vector<eGPUFamily>& forGPUs);

    const Set<String>& GetErrors() const;

private:
    bool RecalculateParamsMD5(const String& params, const FilePath& md5file) const;
    bool RecalculateFileMD5(const FilePath& pathname, const FilePath& md5file) const;

    bool ReadMD5FromFile(const FilePath& md5file, MD5::MD5Digest& digest) const;
    void WriteMD5ToFile(const FilePath& md5file, const MD5::MD5Digest& digest) const;

    uint32 GetMaxTextureSize() const;
    Vector<String> FetchFlags(const FilePath& flagsPathname);
    static String GetProcessFolderName();

    void AddError(const String& errorMsg);

    void RecursiveTreeWalk(const FilePath& inputPath, const FilePath& outputPath, const Vector<PackingAlgorithm>& packAlgorithms, const Vector<String>& flags = Vector<String>());

    bool GetFilesFromCache(const AssetCache::CacheItemKey& key, const FilePath& inputPath, const FilePath& outputPath);
    bool AddFilesToCache(const AssetCache::CacheItemKey& key, const FilePath& inputPath, const FilePath& outputPath);

public:
    FilePath inputGfxDirectory;
    FilePath outputGfxDirectory;
    FilePath rootDirectory;
    FilePath dataSourceDirectory;
    String gfxDirName;
    String texturePostfix;

    bool outputDirModified = true;

    bool isLightmapsPacking = false;
    bool forceRepack = false;
    bool clearOutputDirectory = true;
    Vector<eGPUFamily> requestedGPUs;
    TextureConverter::eConvertQuality quality = TextureConverter::ECQ_VERY_HIGH;

private:
    AssetCacheClient* cacheClient = nullptr;
    AssetCache::CachedItemValue::Description cacheItemDescription;

    Set<String> errors;

    std::atomic<bool> running;
};

inline bool ResourcePacker2D::IsRunning() const
{
    return running;
}
};

#endif // __DAVAENGINE_RESOURCEPACKER2D_H__
