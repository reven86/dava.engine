#pragma once

#include "Utils/StringFormat.h"

#include "Utils/SceneUtils/SceneUtils.h"
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
        OBJECT_EMITTER_CONFIG,

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

    struct Params
    {
        DAVA::FilePath dataFolder;
        DAVA::FilePath dataSourceFolder;

        DAVA::Vector<DAVA::eGPUFamily> exportForGPUs;
        DAVA::TextureConverter::eConvertQuality quality = DAVA::TextureConverter::eConvertQuality::ECQ_DEFAULT;

        bool optimizeOnExport = false;
        bool useHDTextures = false;
        bool forceCompressTextures = false;
    };

    SceneExporter() = default;
    ~SceneExporter();

    using ExportedObjectCollection = DAVA::Vector<ExportedObject>;

    void SetExportingParams(const SceneExporter::Params& exportingParams);

    bool ExportScene(DAVA::Scene* scene, const DAVA::FilePath& scenePathname, ExportedObjectCollection& exportedObjects);
    bool ExportObjects(const ExportedObjectCollection& exportedObjects);

    void SetCacheClient(DAVA::AssetCacheClient* cacheClient, DAVA::String machineName, DAVA::String runDate, DAVA::String comment);

private:
    bool ExportSceneFile(const DAVA::FilePath& scenePathname, const DAVA::String& sceneLink); //with cache
    bool ExportTextureFile(const DAVA::FilePath& descriptorPathname, const DAVA::String& descriptorLink);
    bool ExportHeightmapFile(const DAVA::FilePath& heightmapPathname, const DAVA::String& heightmapLink);
    bool ExportEmitterConfigFile(const DAVA::FilePath& configPathname, const DAVA::String& configLink);

    bool ExportSceneFileInternal(const DAVA::FilePath& scenePathname, ExportedObjectCollection& exportedObjects); //without cache

    bool ExportTextures(DAVA::TextureDescriptor& descriptor);

    bool CopyFile(const DAVA::FilePath& filePath) const;
    bool CopyFile(const DAVA::FilePath& filePath, const DAVA::String& fileLink) const;

    bool SplitCompressedFile(const DAVA::TextureDescriptor& descriptor, DAVA::eGPUFamily gpu) const;

    DAVA::AssetCacheClient* cacheClient = nullptr;
    DAVA::AssetCache::CachedItemValue::Description cacheItemDescription;

    SceneExporter::Params exportingParams;
};
