#pragma once

#include <Tools/TextureCompression/TextureConverter.h>
#include <Tools/AssetCache/AssetCache.h>

#include <Utils/StringFormat.h>
#include <Utils/SceneUtils/SceneUtils.h>

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

    using ExportedObjectCollection = DAVA::Vector<ExportedObject>;

    struct Params
    {
        struct Output
        {
            Output(const DAVA::FilePath& path, const DAVA::Vector<DAVA::eGPUFamily>& gpus, DAVA::TextureConverter::eConvertQuality quality_, bool useHD)
                : exportForGPUs(gpus)
                , dataFolder(path)
                , quality(quality_)
                , useHDTextures(useHD)
            {
            }
            DAVA::Vector<DAVA::eGPUFamily> exportForGPUs;
            DAVA::FilePath dataFolder;
            DAVA::TextureConverter::eConvertQuality quality = DAVA::TextureConverter::eConvertQuality::ECQ_DEFAULT;
            bool useHDTextures = false;
        };

        DAVA::Vector<Output> outputs;
        DAVA::FilePath dataSourceFolder;

        bool optimizeOnExport = false;
    };

    SceneExporter() = default;
    ~SceneExporter();

    void SetExportingParams(const SceneExporter::Params& exportingParams);
    void SetCacheClient(DAVA::AssetCacheClient* cacheClient, DAVA::String machineName, DAVA::String runDate, DAVA::String comment);

    bool ExportScene(DAVA::Scene* scene, const DAVA::FilePath& scenePathname, const DAVA::FilePath& outScenePathname, DAVA::Vector<ExportedObjectCollection>& exportedObjects);
    bool ExportObjects(const ExportedObjectCollection& exportedObjects);

private:
    bool PrepareData(const ExportedObjectCollection& exportedObjects);
    bool ExportSceneObject(const ExportedObject& object);
    bool ExportTextureObject(const ExportedObject& object);
    bool CopyObject(const ExportedObject& object);

    bool ExportSceneFileInternal(const DAVA::FilePath& scenePathname, const DAVA::FilePath& outScenePathname, DAVA::Vector<ExportedObjectCollection>& exportedObjects); //without cache
    bool ExportDescriptor(DAVA::TextureDescriptor& descriptor, const Params::Output& output);
    bool SplitCompressedFile(const DAVA::TextureDescriptor& descriptor, DAVA::eGPUFamily gpu, const Params::Output& output) const;

    void CreateFoldersStructure(const ExportedObjectCollection& exportedObjects);
    bool CopyFile(const DAVA::FilePath& fromPath, const DAVA::FilePath& toPath) const;
    bool CopyFileToOutput(const DAVA::FilePath& fromPath, const Params::Output& output) const;

    DAVA::AssetCacheClient* cacheClient = nullptr;
    DAVA::AssetCache::CachedItemValue::Description cacheItemDescription;

    SceneExporter::Params exportingParams;
    DAVA::Vector<ExportedObjectCollection> objectsToExport;
};

bool operator==(const SceneExporter::ExportedObject& left, const SceneExporter::ExportedObject& right);
bool operator<(const SceneExporter::ExportedObject& left, const SceneExporter::ExportedObject& right);
