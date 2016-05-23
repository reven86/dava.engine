#ifndef __SCENE_EXPORTER_H__
#define __SCENE_EXPORTER_H__

#include "Utils/StringFormat.h"

#include "CommandLine/SceneUtils/SceneUtils.h"
#include "TextureCompression/TextureConverter.h"
#include "AssetCache/AssetCache.h"

namespace DAVA
{
class TextureDescriptor;
class Scene;
class AssetCacheClient;
}

class SceneExporter final
{
public:
    enum eExportedObjectType : DAVA::int32
    {
        OBJECT_NONE = -1,

        OBJECT_SCENE = 0,
        OBJECT_TEXTURE,
        OBJECT_HEIGHTMAP,

        OBJECT_COUNT
    };

    struct ExportedObject
    {
        ExportedObject(eExportedObjectType type_, DAVA::String path)
            : type(type_)
            , relativePathname(std::move(path))
        {
        }

        eExportedObjectType type = OBJECT_NONE;
        DAVA::String relativePathname;
    };

    SceneExporter() = default;
    ~SceneExporter();

    using ExportedObjectCollection = DAVA::Vector<ExportedObject>;

    void SetFolders(const DAVA::FilePath& dataFolder, const DAVA::FilePath& dataSourceFolder);
    void SetCompressionParams(const DAVA::eGPUFamily gpu, DAVA::TextureConverter::eConvertQuality quality);
    void EnableOptimizations(bool enable);

    bool ExportScene(DAVA::Scene* scene, const DAVA::FilePath& scenePathname, ExportedObjectCollection& exportedObjects);
    void ExportObjects(const ExportedObjectCollection& exportedObjects);

    void SetCacheClient(DAVA::AssetCacheClient* cacheClient, DAVA::String machineName, DAVA::String runDate, DAVA::String comment);

private:
    void ExportSceneFile(const DAVA::FilePath& scenePathname, const DAVA::String& sceneLink); //with cache
    void ExportTextureFile(const DAVA::FilePath& descriptorPathname, const DAVA::String& descriptorLink);
    void ExportHeightmapFile(const DAVA::FilePath& heightmapPathname, const DAVA::String& heightmapLink);

    void ExportSceneFileInternal(const DAVA::FilePath& scenePathname, ExportedObjectCollection& exportedObjects); //without cache

    DAVA::FilePath CompressTexture(DAVA::TextureDescriptor& descriptor) const;
    void CopySourceTexture(DAVA::TextureDescriptor& descriptor) const;

    bool CopyFile(const DAVA::FilePath& filePath) const;
    bool CopyFile(const DAVA::FilePath& filePath, const DAVA::String& fileLink) const;

    DAVA::eGPUFamily exportForGPU = DAVA::eGPUFamily::GPU_ORIGIN;
    DAVA::TextureConverter::eConvertQuality quality = DAVA::TextureConverter::eConvertQuality::ECQ_DEFAULT;

    bool optimizeOnExport = false;

    DAVA::FilePath dataFolder;
    DAVA::FilePath dataSourceFolder;

    DAVA::AssetCacheClient* cacheClient = nullptr;
    DAVA::AssetCache::CachedItemValue::Description cacheItemDescription;
};


#endif // __SCENE_EXPORTER_H__
