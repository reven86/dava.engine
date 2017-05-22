#include "Utils/SceneExporter/SceneExporter.h"
#include "CommandLine/Private/SceneConsoleHelper.h"

#include <Tools/AssetCache/AssetCacheClient.h>
#include <Tools/TextureCompression/TextureConverter.h>

#include "FileSystem/FilePath.h"
#include "FileSystem/FileSystem.h"
#include "Functional/Function.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleEmitter.h"
#include "Platform/Process.h"
#include "Render/GPUFamilyDescriptor.h"
#include "Render/TextureDescriptor.h"
#include "Render/Highlevel/Heightmap.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Image/ImageSystem.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/SceneFile/VersionInfo.h"
#include "Utils/StringUtils.h"
#include "Utils/MD5.h"
#include "Logger/Logger.h"


#include "StringConstants.h"
#include "Scene/SceneHelper.h"

using namespace DAVA;

namespace SceneExporterCache
{
const uint32 EXPORTER_VERSION = 1;
const uint32 LINKS_PARSER_VERSION = 1;
const String LINKS_NAME = "links.txt";

void CalculateSceneKey(const FilePath& scenePathname, const String& sceneLink, AssetCache::CacheItemKey& key, uint32 optimize)
{
    { //calculate digest for scene file
        MD5::MD5Digest fileDigest;
        MD5::ForFile(scenePathname, fileDigest);
        key.SetPrimaryKey(fileDigest);
    }

    { //calculate digest for params
        ScopedPtr<File> file(File::Create(scenePathname, File::OPEN | File::READ));

        MD5::MD5Digest sceneParamsDigest;
        String params = "ResourceEditor";
        params += Format("Pathname: %s", sceneLink.c_str());
        params += Format("FileSize: %d", (file) ? file->GetSize() : 0);
        params += Format("SceneFileVersion: %d", SCENE_FILE_CURRENT_VERSION);
        params += Format("ExporterVersion: %u", EXPORTER_VERSION);
        params += Format("LinksParserVersion: %u", LINKS_PARSER_VERSION);
        params += Format("Optimized: %u", optimize);
        for (int32 linkType = 0; linkType < SceneExporter::OBJECT_COUNT; ++linkType)
        {
            params += Format("LinkType: %d", linkType);
        }

        MD5::ForData(reinterpret_cast<const uint8*>(params.data()), static_cast<uint32>(params.size()), sceneParamsDigest);
        key.SetSecondaryKey(sceneParamsDigest);
    }
}

} //namespace SceneExporterCache

namespace SceneExporterInternal
{
bool SaveExportedObjects(const FilePath& linkPathname, const Vector<SceneExporter::ExportedObjectCollection>& exportedObjects)
{
    ScopedPtr<File> linksFile(File::Create(linkPathname, File::CREATE | File::WRITE));
    if (linksFile)
    {
        linksFile->WriteLine(Format("%d", static_cast<int32>(exportedObjects.size())));
        for (const SceneExporter::ExportedObjectCollection& collection : exportedObjects)
        {
            for (const SceneExporter::ExportedObject& object : collection)
            {
                linksFile->WriteLine(Format("%d,%s", object.type, object.relativePathname.c_str()));
            }
        }
        return true;
    }
    else
    {
        Logger::Error("Cannot open file with links: %s", linkPathname.GetAbsolutePathname().c_str());
        return false;
    }
}

bool LoadExportedObjects(const FilePath& linkPathname, Vector<SceneExporter::ExportedObjectCollection>& exportedObjects)
{
    ScopedPtr<File> linksFile(File::Create(linkPathname, File::OPEN | File::READ));
    if (linksFile)
    {
        String sizeStr = linksFile->ReadLine();
        int32 size = 0;
        int32 number = sscanf(sizeStr.c_str(), "%d", &size);
        if (size > 0 && number == 1)
        {
            exportedObjects.reserve(size);
            while (size--)
            {
                if (linksFile->IsEof())
                {
                    Logger::Warning("Reading of file stopped by EOF: %s", linkPathname.GetAbsolutePathname().c_str());
                    break;
                }

                String formatedString = linksFile->ReadLine();
                if (formatedString.empty())
                {
                    Logger::Warning("Reading of file stopped by empty string: %s", linkPathname.GetAbsolutePathname().c_str());
                    break;
                }

                auto dividerPos = formatedString.find(',', 1); //skip first number
                DVASSERT(dividerPos != String::npos);

                SceneExporter::eExportedObjectType type = static_cast<SceneExporter::eExportedObjectType>(atoi(formatedString.substr(0, dividerPos).c_str()));
                exportedObjects[type].emplace_back(type, formatedString.substr(dividerPos + 1));
            }
        }
        else if (number != 1)
        {
            Logger::Error("Cannot read size value from file: %s", linkPathname.GetAbsolutePathname().c_str());
            return false;
        }
    }
    else
    {
        Logger::Error("Cannot open file with links: %s", linkPathname.GetAbsolutePathname().c_str());
        return false;
    }

    return true;
}

inline bool IsEditorEntity(Entity* entity)
{
    const String::size_type pos = entity->GetName().find(ResourceEditor::EDITOR_BASE);
    return (String::npos != pos);
}

void RemoveEditorCustomProperties(Entity* entity)
{
    //    "editor.dynamiclight.enable";
    //    "editor.donotremove";
    //
    //    "editor.referenceToOwner";
    //    "editor.isSolid";
    //    "editor.isLocked";
    //    "editor.designerName"
    //    "editor.modificationData"
    //    "editor.staticlight.enable";
    //    "editor.staticlight.used"
    //    "editor.staticlight.castshadows";
    //    "editor.staticlight.receiveshadows";
    //    "editor.staticlight.falloffcutoff"
    //    "editor.staticlight.falloffexponent"
    //    "editor.staticlight.shadowangle"
    //    "editor.staticlight.shadowsamples"
    //    "editor.staticlight.shadowradius"
    //    "editor.intensity"

    KeyedArchive* props = GetCustomPropertiesArchieve(entity);
    if (props)
    {
        const KeyedArchive::UnderlyingMap propsMap = props->GetArchieveData();

        for (auto& it : propsMap)
        {
            const String& key = it.first;

            if (key.find(ResourceEditor::EDITOR_BASE) == 0)
            {
                if ((key != ResourceEditor::EDITOR_DO_NOT_REMOVE) && (key != ResourceEditor::EDITOR_DYNAMIC_LIGHT_ENABLE))
                {
                    props->DeleteKey(key);
                }
            }
        }

        if (props->Count() == 0)
        {
            entity->RemoveComponent(Component::CUSTOM_PROPERTIES_COMPONENT);
        }
    }
}

void PrepareSceneToExport(Scene* scene, bool removeCustomProperties)
{
    //Remove scene nodes
    Vector<Entity*> entities;
    scene->GetChildNodes(entities);

    for (auto& entity : entities)
    {
        bool needRemove = IsEditorEntity(entity);
        if (needRemove)
        {
            //remove nodes from hierarchy
            DVASSERT(entity->GetParent() != nullptr);
            entity->GetParent()->RemoveNode(entity);
        }
        else if (removeCustomProperties)
        {
            RemoveEditorCustomProperties(entity);
        }
    }
}

void CollectHeightmapPathname(Scene* scene, const FilePath& dataSourceFolder, SceneExporter::ExportedObjectCollection& exportedObjects)
{
    Landscape* landscape = FindLandscape(scene);
    if (landscape != nullptr)
    {
        const FilePath& heightmapPath = landscape->GetHeightmapPathname();
        exportedObjects.emplace_back(SceneExporter::OBJECT_HEIGHTMAP, heightmapPath.GetRelativePathname(dataSourceFolder));
    }
}

void CollectTextureDescriptors(Scene* scene, const FilePath& dataSourceFolder, SceneExporter::ExportedObjectCollection& exportedObjects)
{
    SceneHelper::TextureCollector collector(SceneHelper::TextureCollector::IncludeNullTextures);
    SceneHelper::EnumerateSceneTextures(scene, collector);

    exportedObjects.reserve(exportedObjects.size() + collector.GetTextures().size());
    for (const auto& scTex : collector.GetTextures())
    {
        const FilePath& path = scTex.first;
        if (path.GetType() == FilePath::PATH_IN_MEMORY)
        {
            continue;
        }

        DVASSERT(path.IsEmpty() == false);

        exportedObjects.emplace_back(SceneExporter::OBJECT_TEXTURE, path.GetRelativePathname(dataSourceFolder));
    }
}

void CollectParticleConfigs(Scene* scene, const FilePath& dataSourceFolder, SceneExporter::ExportedObjectCollection& exportedObjects)
{
    Function<void(ParticleEmitter*)> collectSuperEmitters = [&collectSuperEmitters, &exportedObjects, &dataSourceFolder](ParticleEmitter* emitter)
    {
        if (emitter->configPath.IsEmpty() == false)
        {
            exportedObjects.emplace_back(SceneExporter::OBJECT_EMITTER_CONFIG, emitter->configPath.GetRelativePathname(dataSourceFolder));
        }

        for (ParticleLayer* layer : emitter->layers)
        {
            if (layer->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
            {
                collectSuperEmitters(layer->innerEmitter);
            }
        }
    };

    Vector<Entity*> effects;
    scene->GetChildEntitiesWithComponent(effects, Component::PARTICLE_EFFECT_COMPONENT);
    for (Entity* e : effects)
    {
        uint32 count = e->GetComponentCount(Component::PARTICLE_EFFECT_COMPONENT);
        for (uint32 ic = 0; ic < count; ++ic)
        {
            ParticleEffectComponent* effectComponent = static_cast<ParticleEffectComponent*>(e->GetComponent(Component::PARTICLE_EFFECT_COMPONENT, ic));
            uint32 emittersCount = effectComponent->GetEmittersCount();
            for (uint32 id = 0; id < emittersCount; ++id)
            {
                ParticleEmitterInstance* emitterInstance = effectComponent->GetEmitterInstance(id);
                ParticleEmitter* emitter = emitterInstance->GetEmitter();
                collectSuperEmitters(emitter);
            }
        }
    }
}

} //namespace SceneExporterV2Internal

SceneExporter::~SceneExporter() = default;

void SceneExporter::SetExportingParams(const SceneExporter::Params& exportingParams_)
{
    exportingParams = exportingParams_;

    DVASSERT(exportingParams.outputs.empty() == false);
    DVASSERT(exportingParams.dataSourceFolder.IsDirectoryPathname());

    for (const Params::Output& output : exportingParams.outputs)
    {
        DVASSERT(output.dataFolder.IsDirectoryPathname());
        DVASSERT(output.exportForGPUs.empty() == false);
    }
}

void SceneExporter::SetCacheClient(AssetCacheClient* cacheClient_, String machineName, String runDate, String comment)
{
    cacheClient = cacheClient_;
    cacheItemDescription.machineName = machineName;
    cacheItemDescription.creationDate = runDate;
    cacheItemDescription.comment = comment;
}

bool SceneExporter::ExportSceneObject(const ExportedObject& sceneObject)
{
    using namespace DAVA;

    Logger::Info("Exporting of %s", sceneObject.relativePathname.c_str());

    FileSystem* fileSystem = GetEngineContext()->fileSystem;

    FilePath scenePathname = exportingParams.dataSourceFolder + sceneObject.relativePathname;
    FilePath rootFolder = fileSystem->GetTempDirectoryPath() + "/Export/";
    FilePath tempDataFolder = rootFolder + "Data/";

    FilePath outScenePathname = tempDataFolder + sceneObject.relativePathname;
    FilePath outSceneFolder = outScenePathname.GetDirectory();
    fileSystem->CreateDirectory(outSceneFolder, true);

    FilePath linksPathname(outSceneFolder + SceneExporterCache::LINKS_NAME);

    SCOPE_EXIT
    { //delete temporary file
        fileSystem->DeleteDirectoryFiles(tempDataFolder, true);
        fileSystem->DeleteDirectory(tempDataFolder, true);
    };

    auto copyScene = [this, &outScenePathname, &sceneObject]()
    {
        bool filesCopied = true;
        for (const Params::Output& output : exportingParams.outputs)
        {
            filesCopied = CopyFile(outScenePathname, output.dataFolder + sceneObject.relativePathname) && filesCopied;
        }

        return filesCopied;
    };

    AssetCache::CacheItemKey cacheKey;
    if (cacheClient != nullptr && cacheClient->IsConnected())
    { //request Scene from cache
        SceneExporterCache::CalculateSceneKey(scenePathname, sceneObject.relativePathname, cacheKey, static_cast<uint32>(exportingParams.optimizeOnExport));

        AssetCache::CachedItemValue retrievedData;
        AssetCache::Error requested = cacheClient->RequestFromCacheSynchronously(cacheKey, &retrievedData);
        if (requested == AssetCache::Error::NO_ERRORS)
        {
            bool exportedToFolder = retrievedData.ExportToFolder(outSceneFolder);

            bool filesCopied = copyScene();
            bool objectsLoaded = SceneExporterInternal::LoadExportedObjects(linksPathname, objectsToExport);
            return exportedToFolder && objectsLoaded && filesCopied;
        }
        else
        {
            Logger::Info("%s - failed to retrieve from cache(%s)", scenePathname.GetAbsolutePathname().c_str(), AssetCache::ErrorToString(requested).c_str());
        }
    }

    bool sceneExported = false;
    Vector<ExportedObjectCollection> externalLinks;
    externalLinks.resize(eExportedObjectType::OBJECT_COUNT);

    { //has no scene in cache or using of cache is disabled. Export scene directly
        sceneExported = ExportSceneFileInternal(scenePathname, outScenePathname, externalLinks);
        sceneExported = copyScene() && sceneExported;

        //add links to whole list of files
        for (int32 i = 0; i < eExportedObjectType::OBJECT_COUNT; ++i)
        {
            objectsToExport[i].insert(objectsToExport[i].end(), externalLinks[i].begin(), externalLinks[i].end());
        }
    }

    if (cacheClient != nullptr && cacheClient->IsConnected())
    { //place exported scene into cache
        SceneExporterInternal::SaveExportedObjects(linksPathname, externalLinks);

        AssetCache::CachedItemValue value;
        value.Add(outScenePathname);
        value.Add(linksPathname);
        value.UpdateValidationData();
        value.SetDescription(cacheItemDescription);

        AssetCache::Error added = cacheClient->AddToCacheSynchronously(cacheKey, value);
        if (added == AssetCache::Error::NO_ERRORS)
        {
            Logger::Info("%s - added to cache", scenePathname.GetAbsolutePathname().c_str());
        }
        else
        {
            Logger::Info("%s - failed to add to cache (%s)", scenePathname.GetAbsolutePathname().c_str(), AssetCache::ErrorToString(added).c_str());
        }
    }

    return sceneExported;
}

bool SceneExporter::ExportSceneFileInternal(const DAVA::FilePath& scenePathname, const DAVA::FilePath& outScenePathname, DAVA::Vector<SceneExporter::ExportedObjectCollection>& exportedObjects)
{
    bool sceneExported = false;

    //Load scene from *.sc2
    ScopedPtr<Scene> scene(new Scene());
    if (SceneFileV2::ERROR_NO_ERROR == scene->LoadScene(scenePathname))
    {
        sceneExported = ExportScene(scene, scenePathname, outScenePathname, exportedObjects);
    }
    else
    {
        Logger::Error("[SceneExporterV2::%s] Can't open file %s", __FUNCTION__, scenePathname.GetAbsolutePathname().c_str());
    }

    SceneConsoleHelper::FlushRHI();
    return sceneExported;
}

namespace TextureDescriptorValidator
{
bool IsImageValidForFormat(const DAVA::ImageInfo& info, const DAVA::PixelFormat format)
{
    if (info.width != info.height && (format == DAVA::FORMAT_PVR2 || format == DAVA::FORMAT_PVR4))
    {
        return false;
    }

    return true;
}

bool IsImageSizeValidForTextures(const DAVA::ImageInfo& info)
{
    return ((info.width >= DAVA::Texture::MINIMAL_WIDTH) && (info.height >= DAVA::Texture::MINIMAL_HEIGHT));
}
}

namespace SceneExporterLocal
{
void CompressNotActualTexture(const DAVA::eGPUFamily gpu, DAVA::TextureConverter::eConvertQuality quality, DAVA::TextureDescriptor& descriptor)
{
    DVASSERT(GPUFamilyDescriptor::IsGPUForDevice(gpu));

    if (descriptor.IsCompressedTextureActual(gpu) == false)
    {
        DAVA::Logger::Warning("Need recompress texture: %s", descriptor.GetSourceTexturePathname().GetAbsolutePathname().c_str());
        TextureConverter::ConvertTexture(descriptor, gpu, true, quality);
    }
}

void CollectSourceImageInfo(const DAVA::TextureDescriptor& descriptor, DAVA::Vector<DAVA::ImageInfo>& sourceImageInfos)
{
    Vector<FilePath> imagePathnames;
    if (descriptor.IsCubeMap())
    {
        descriptor.GetFacePathnames(imagePathnames);
    }
    else
    {
        imagePathnames.push_back(descriptor.GetSourceTexturePathname());
    }

    sourceImageInfos.reserve(imagePathnames.size());
    for (const FilePath& path : imagePathnames)
    {
        sourceImageInfos.push_back(DAVA::ImageSystem::GetImageInfo(path));
    }
}
}

bool SceneExporter::ExportTextureObject(const ExportedObject& object)
{
    using namespace DAVA;

    FilePath descriptorPathname = exportingParams.dataSourceFolder + object.relativePathname;

    bool texturesExported = true;
    for (const Params::Output& output : exportingParams.outputs)
    {
        std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(descriptorPathname));
        if (!descriptor)
        {
            Logger::Error("Can't create descriptor for pathname %s", descriptorPathname.GetStringValue().c_str());
            return false;
        }

        texturesExported = ExportDescriptor(*descriptor, output);
        if (texturesExported)
        {
            if (output.exportForGPUs.size() == 1)
            {
                descriptor->Export(output.dataFolder + object.relativePathname, output.exportForGPUs[0]);
            }
            else
            {
                descriptor->Save(output.dataFolder + object.relativePathname);
            }
        }
    }

    return texturesExported;
}

bool SceneExporter::ExportDescriptor(DAVA::TextureDescriptor& descriptor, const Params::Output& output)
{
    DAVA::Map<DAVA::eGPUFamily, bool> exportFailed;
    bool shouldSplitHDTextures = (output.useHDTextures && descriptor.dataSettings.GetGenerateMipMaps());

    { // compress images

        DAVA::Vector<DAVA::ImageInfo> sourceImageInfos;
        SceneExporterLocal::CollectSourceImageInfo(descriptor, sourceImageInfos);

        for (DAVA::eGPUFamily gpu : output.exportForGPUs)
        {
            if (gpu == DAVA::eGPUFamily::GPU_ORIGIN)
            {
                DAVA::ImageFormat targetFormat = static_cast<DAVA::ImageFormat>(descriptor.compression[gpu].imageFormat);
                if (shouldSplitHDTextures && (targetFormat != DAVA::ImageFormat::IMAGE_FORMAT_DDS && targetFormat != IMAGE_FORMAT_PVR))
                {
                    Logger::Error("HD texture will not be created for exported %s for GPU 'origin'", descriptor.pathname.GetStringValue().c_str());
                    exportFailed[gpu] = true;
                    continue;
                }
            }
            else if (DAVA::GPUFamilyDescriptor::IsGPUForDevice(gpu))
            {
                DAVA::PixelFormat format = descriptor.GetPixelFormatForGPU(gpu);
                if (format == DAVA::PixelFormat::FORMAT_INVALID)
                {
                    Logger::Error("Texture %s has not pixel format specified for GPU %s", descriptor.pathname.GetStringValue().c_str(), GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(gpu));
                    exportFailed[gpu] = true;
                    continue;
                }

                for (const DAVA::ImageInfo& imgInfo : sourceImageInfos)
                {
                    if (!TextureDescriptorValidator::IsImageValidForFormat(imgInfo, format))
                    {
                        Logger::Error("Can't export non-square texture %s into compression format %s",
                                      descriptor.pathname.GetAbsolutePathname().c_str(), GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(format));
                        exportFailed[gpu] = true;
                        break;
                    }
                    else if (!TextureDescriptorValidator::IsImageSizeValidForTextures(imgInfo))
                    {
                        Logger::Error("Can't export small sized texture %s into compression format %s",
                                      descriptor.pathname.GetAbsolutePathname().c_str(), GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(format));
                        exportFailed[gpu] = true;
                        break;
                    }
                }

                if (exportFailed.count(gpu) == 0)
                {
                    SceneExporterLocal::CompressNotActualTexture(gpu, output.quality, descriptor);
                }
            }
            else if (gpu != DAVA::eGPUFamily::GPU_ORIGIN)
            {
                Logger::Error("Has no code for GPU %d (%s)", gpu, GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(gpu));
                exportFailed[gpu] = true;
            }
        }
    }

    //modify descriptors in data
    descriptor.dataSettings.SetSeparateHDTextures(shouldSplitHDTextures);

    { // copy or separate images
        for (DAVA::eGPUFamily gpu : output.exportForGPUs)
        {
            if (exportFailed.count(gpu) != 0)
            { // found errors on compress step
                continue;
            }

            bool copied = true;

            if (gpu == DAVA::GPU_ORIGIN)
            {
                if (descriptor.IsCubeMap())
                {
                    DAVA::Vector<DAVA::FilePath> faceNames;
                    descriptor.GetFacePathnames(faceNames);
                    for (const auto& faceName : faceNames)
                    {
                        if (faceName.IsEmpty())
                            continue;

                        copied = CopyFileToOutput(faceName, output) && copied;
                    }
                }
                else
                {
                    copied = CopyFileToOutput(descriptor.GetSourceTexturePathname(), output);
                }
            }
            else if (DAVA::GPUFamilyDescriptor::IsGPUForDevice(gpu))
            {
                if (shouldSplitHDTextures)
                {
                    copied = SplitCompressedFile(descriptor, gpu, output);
                }
                else
                {
                    DAVA::FilePath compressedName = descriptor.CreateMultiMipPathnameForGPU(gpu);
                    copied = (compressedName.IsEmpty() ? false : CopyFileToOutput(compressedName, output));
                }
            }

            if (!copied)
            {
                exportFailed[gpu] = true;
            }
        }
    }

    return (exportFailed.size() < output.exportForGPUs.size());
}

bool SceneExporter::SplitCompressedFile(const DAVA::TextureDescriptor& descriptor, DAVA::eGPUFamily gpu, const Params::Output& output) const
{
    using namespace DAVA;

    Vector<Image*> loadedImages;
    SCOPE_EXIT
    {
        for (Image* image : loadedImages)
        {
            SafeRelease(image);
        }
    };

    FilePath compressedTexturePath = descriptor.CreateMultiMipPathnameForGPU(gpu);

    eErrorCode loadError = ImageSystem::Load(compressedTexturePath, loadedImages);
    if (loadError != eErrorCode::SUCCESS || loadedImages.empty())
    {
        Logger::Error("Can't load %s", compressedTexturePath.GetStringValue().c_str());
        return false;
    }

    PixelFormat targetFormat = descriptor.GetPixelFormatForGPU(gpu);
    DVASSERT(targetFormat == loadedImages[0]->format);

    size_type mipmapsCount = loadedImages.size();
    bool isCubemap = loadedImages[0]->cubeFaceID != Texture::INVALID_CUBEMAP_FACE;
    if (isCubemap)
    {
        DAVA::uint32 firstFace = loadedImages[0]->cubeFaceID;
        mipmapsCount = count_if(loadedImages.begin(), loadedImages.end(), [&firstFace](const DAVA::Image* img) { return img->cubeFaceID == firstFace; });
    }

    Vector<FilePath> pathnamesForGPU;
    descriptor.CreateLoadPathnamesForGPU(gpu, pathnamesForGPU);
    size_type outTexturesCount = pathnamesForGPU.size();

    if (mipmapsCount < outTexturesCount)
    {
        Logger::Error("Can't split HD level for %s", compressedTexturePath.GetStringValue().c_str());
        return false;
    }

    auto createOutPathname = [&output, this](const FilePath& pathname)
    {
        String fileLink = pathname.GetRelativePathname(exportingParams.dataSourceFolder);
        return output.dataFolder + fileLink;
    };

    enum class eSavingParam
    {
        SaveOneMip,
        SaveRemainingMips
    };
    auto saveImages = [&](const FilePath& path, size_type mip, eSavingParam param)
    {
        if (isCubemap)
        {
            Vector<Vector<Image*>> savedImages;
            for (DAVA::uint32 face = 0; face < Texture::CUBE_FACE_COUNT; ++face)
            {
                Vector<Image*> faceMips;

                for (Image* loadedImage : loadedImages)
                {
                    if (loadedImage->cubeFaceID == face && loadedImage->mipmapLevel >= static_cast<DAVA::uint32>(mip))
                    {
                        faceMips.push_back(loadedImage);
                        if (param == eSavingParam::SaveOneMip)
                            break;
                    }
                }

                if (!faceMips.empty())
                {
                    savedImages.resize(savedImages.size() + 1);
                    savedImages.back().swap(faceMips);
                }
            }

            eErrorCode saveError = ImageSystem::SaveAsCubeMap(path, savedImages, targetFormat);
            if (saveError != eErrorCode::SUCCESS)
            {
                Logger::Error("Can't save %s", path.GetStringValue().c_str());
                return false;
            }
        }
        else
        {
            DVASSERT(mip < loadedImages.size());
            Vector<Image*> savedImages;
            if (param == eSavingParam::SaveOneMip)
            {
                savedImages.push_back(loadedImages[mip]);
            }
            else
            {
                savedImages.assign(loadedImages.begin() + mip, loadedImages.end());
            }

            eErrorCode saveError = ImageSystem::Save(path, savedImages, targetFormat);
            if (saveError != eErrorCode::SUCCESS)
            {
                Logger::Error("Can't save %s", path.GetStringValue().c_str());
                return false;
            }
        }

        return true;
    };

    // save hd mips, each in separate file
    size_type mip = 0;
    size_type singleMipCount = outTexturesCount - 1;
    for (; mip < singleMipCount; ++mip)
    {
        bool saved = saveImages(createOutPathname(pathnamesForGPU[mip]), mip, eSavingParam::SaveOneMip);
        if (!saved)
        {
            return false;
        }
    }

    // save remaining mips, all in single file
    return saveImages(createOutPathname(pathnamesForGPU[mip]), mip, eSavingParam::SaveRemainingMips);
}

bool SceneExporter::CopyFile(const DAVA::FilePath& fromPath, const DAVA::FilePath& toPath) const
{
    bool retCopy = true;
    if (fromPath != toPath)
    {
        using namespace DAVA;

        FileSystem* fileSystem = GetEngineContext()->fileSystem;
        retCopy = fileSystem->CopyFile(fromPath, toPath, true);
        if (retCopy == false)
        {
            Logger::Error("Can't copy %s to %s", fromPath.GetStringValue().c_str(), toPath.GetStringValue().c_str());
        }
    }
    return retCopy;
}

bool SceneExporter::ExportScene(DAVA::Scene* scene, const DAVA::FilePath& scenePathname, const DAVA::FilePath& outScenePathname, DAVA::Vector<ExportedObjectCollection>& exportedObjects)
{
    DVASSERT(exportedObjects.size() == SceneExporter::OBJECT_COUNT);

    using namespace DAVA;

    SceneExporterInternal::PrepareSceneToExport(scene, exportingParams.optimizeOnExport);

    SceneExporterInternal::CollectHeightmapPathname(scene, exportingParams.dataSourceFolder, exportedObjects[eExportedObjectType::OBJECT_HEIGHTMAP]); //must be first
    SceneExporterInternal::CollectTextureDescriptors(scene, exportingParams.dataSourceFolder, exportedObjects[eExportedObjectType::OBJECT_TEXTURE]);
    SceneExporterInternal::CollectParticleConfigs(scene, exportingParams.dataSourceFolder, exportedObjects[eExportedObjectType::OBJECT_EMITTER_CONFIG]);

    // save scene to new place
    FilePath tempSceneName = FilePath::CreateWithNewExtension(scenePathname, ".exported.sc2");
    scene->SaveScene(tempSceneName, exportingParams.optimizeOnExport);

    FileSystem* fileSystem = GetEngineContext()->fileSystem;
    bool moved = fileSystem->MoveFile(tempSceneName, outScenePathname, true);
    if (!moved)
    {
        Logger::Error("Can't move file %s into %s", tempSceneName.GetStringValue().c_str(), outScenePathname.GetStringValue().c_str());
        fileSystem->DeleteFile(tempSceneName);
        return false;
    }

    return true;
}

bool SceneExporter::ExportObjects(const ExportedObjectCollection& exportedObjects)
{
    using namespace DAVA;

    Array<Function<bool(const ExportedObject&)>, OBJECT_COUNT> exporters =
    { { MakeFunction(this, &SceneExporter::ExportSceneObject),
        MakeFunction(this, &SceneExporter::ExportTextureObject),
        MakeFunction(this, &SceneExporter::CopyObject),
        MakeFunction(this, &SceneExporter::CopyObject) } };

    // divide objects into different collections
    bool exportIsOk = PrepareData(exportedObjects);

    //export scenes only. Add textures, heightmaps to objectsToExport
    const ExportedObjectCollection& scenes = objectsToExport[eExportedObjectType::OBJECT_SCENE];
    CreateFoldersStructure(scenes);
    uint32 oldScenesCount = static_cast<uint32>(scenes.size());
    for (uint32 i = 0; i < static_cast<uint32>(scenes.size()); ++i)
    {
        exportIsOk = ExportSceneObject(scenes[i]) && exportIsOk;
    }
    DVASSERT(oldScenesCount == static_cast<uint32>(scenes.size())); // be shure that we didn't add new scenes to list

    //export objects
    for (int32 i = eExportedObjectType::OBJECT_SCENE + 1; i < eExportedObjectType::OBJECT_COUNT; ++i)
    {
        CreateFoldersStructure(objectsToExport[i]);
        for (const ExportedObject& object : objectsToExport[i])
        {
            exportIsOk = exporters[i](object) && exportIsOk;
        }
    }

    return exportIsOk;
}

bool SceneExporter::PrepareData(const ExportedObjectCollection& exportedObjects)
{
    objectsToExport.clear();
    objectsToExport.resize(eExportedObjectType::OBJECT_COUNT);

    bool dataIsValid = true;
    for (const ExportedObject& object : exportedObjects)
    {
        if (object.type != OBJECT_NONE && object.type < OBJECT_COUNT)
        {
            objectsToExport[object.type].emplace_back(object.type, object.relativePathname);
        }
        else
        {
            Logger::Error("Found wrong path: %s", object.relativePathname.c_str());
            dataIsValid = false;
        }
    }

    return dataIsValid;
}

void SceneExporter::CreateFoldersStructure(const ExportedObjectCollection& exportedObjects)
{
    using namespace DAVA;

    UnorderedSet<String> foldersStructure;
    foldersStructure.reserve(exportedObjects.size());

    for (const ExportedObject& object : exportedObjects)
    {
        const String& link = object.relativePathname;
        const String::size_type slashpos = link.rfind(String("/"));
        if (slashpos != String::npos)
        {
            foldersStructure.insert(link.substr(0, slashpos + 1));
        }
    }

    FileSystem* fileSystem = GetEngineContext()->fileSystem;
    for (const Params::Output& output : exportingParams.outputs)
    {
        for (const String& folder : foldersStructure)
        {
            fileSystem->CreateDirectory(output.dataFolder + folder, true);
        }
    }
}

bool SceneExporter::CopyFileToOutput(const DAVA::FilePath& fromPath, const Params::Output& output) const
{
    using namespace DAVA;

    String relativePathname = fromPath.GetRelativePathname(exportingParams.dataSourceFolder);
    return CopyFile(fromPath, output.dataFolder + relativePathname);
}

bool SceneExporter::CopyObject(const ExportedObject& object)
{
    using namespace DAVA;

    bool filesCopied = true;

    FilePath fromPath = exportingParams.dataSourceFolder + object.relativePathname;
    for (const Params::Output& output : exportingParams.outputs)
    {
        filesCopied = CopyFile(fromPath, output.dataFolder + object.relativePathname) && filesCopied;
    }

    return filesCopied;
}
