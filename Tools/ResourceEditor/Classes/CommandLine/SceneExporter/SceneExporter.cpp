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

#include "CommandLine/SceneExporter/SceneExporter.h"

#include "AssetCache/AssetCacheClient.h"
#include "Debug/Stats.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/FileSystem.h"
#include "Functional/Function.h"
#include "Platform/Process.h"
#include "Platform/SystemTimer.h"
#include "Render/GPUFamilyDescriptor.h"
#include "Render/TextureDescriptor.h"
#include "Render/Highlevel/Heightmap.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Image/ImageSystem.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/SceneFile/VersionInfo.h"
#include "Utils/StringUtils.h"
#include "Utils/MD5.h"

#include "TextureCompression/TextureConverter.h"

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

        Memcpy(key.data(), fileDigest.digest.data(), MD5::MD5Digest::DIGEST_SIZE);
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

        MD5::ForData(reinterpret_cast<const uint8*>(params.data()), static_cast<uint32>(params.size()), sceneParamsDigest);
        Memcpy(key.data() + MD5::MD5Digest::DIGEST_SIZE, sceneParamsDigest.digest.data(), MD5::MD5Digest::DIGEST_SIZE);
    }
}

} //namespace SceneExporterCache

namespace SceneExporterInternal
{
void SaveExportedObjects(const FilePath& linkPathname, const SceneExporter::ExportedObjectCollection& exportedObjects)
{
    ScopedPtr<File> linksFile(File::Create(linkPathname, File::CREATE | File::WRITE));
    if (linksFile)
    {
        linksFile->WriteLine(Format("%d", static_cast<int32>(exportedObjects.size())));
        for (const auto& object : exportedObjects)
        {
            linksFile->WriteLine(Format("%d,%s", object.type, object.relativePathname.c_str()));
        }
    }
    else
    {
        Logger::Error("Cannot open file with links: %s", linkPathname.GetAbsolutePathname().c_str());
    }
}

void LoadExportedObjects(const FilePath& linkPathname, SceneExporter::ExportedObjectCollection& exportedObjects)
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

                exportedObjects.emplace_back(static_cast<SceneExporter::eExportedObjectType>(atoi(formatedString.substr(0, dividerPos).c_str())), formatedString.substr(dividerPos + 1));
            }
        }
        else if (number != 1)
        {
            Logger::Error("Cannot read size value from file: %s", linkPathname.GetAbsolutePathname().c_str());
        }
    }
    else
    {
        Logger::Error("Cannot open file with links: %s", linkPathname.GetAbsolutePathname().c_str());
    }
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

} //namespace SceneExporterV2Internal

SceneExporter::~SceneExporter() = default;

void SceneExporter::SetExportingParams(const SceneExporter::Params& exportingParams_)
{
    exportingParams = exportingParams_;

    DVASSERT(exportingParams.dataFolder.IsDirectoryPathname());
    DVASSERT(exportingParams.dataSourceFolder.IsDirectoryPathname());
}


void SceneExporter::SetCacheClient(AssetCacheClient* cacheClient_, String machineName, String runDate, String comment)
{
    cacheClient = cacheClient_;
    cacheItemDescription.machineName = machineName;
    cacheItemDescription.creationDate = runDate;
    cacheItemDescription.comment = comment;
}

void SceneExporter::ExportSceneFile(const FilePath& scenePathname, const String& sceneLink)
{
    Logger::Info("Exporting of %s", sceneLink.c_str());

    FilePath outScenePathname = exportingParams.dataFolder + sceneLink;
    FilePath outSceneFolder = outScenePathname.GetDirectory();
    FilePath linksPathname(outSceneFolder + SceneExporterCache::LINKS_NAME);

    SCOPE_EXIT
    { //delete temporary file
        bool exists = FileSystem::Instance()->Exists(linksPathname); //temporary debugging check
        bool deleted = FileSystem::Instance()->DeleteFile(linksPathname);

        DVASSERT(exists == deleted); //temporary debugging and testing check
    };

    ExportedObjectCollection externalLinks;

    AssetCache::CacheItemKey cacheKey;
    if (cacheClient != nullptr && cacheClient->IsConnected())
    { //request Scene from cache
        SceneExporterCache::CalculateSceneKey(scenePathname, sceneLink, cacheKey, static_cast<uint32>(exportingParams.optimizeOnExport));

        AssetCache::Error requested = cacheClient->RequestFromCacheSynchronously(cacheKey, outScenePathname.GetDirectory());
        if (requested == AssetCache::Error::NO_ERRORS)
        {
            SceneExporterInternal::LoadExportedObjects(linksPathname, externalLinks);
            ExportObjects(externalLinks);
            return;
        }
        else
        {
            Logger::Info("%s - failed to retrieve from cache(%s)", scenePathname.GetAbsolutePathname().c_str(), AssetCache::ErrorToString(requested).c_str());
        }
    }

    { //has no scene in cache or using of cache is disabled. Export scene directly
        ExportSceneFileInternal(scenePathname, externalLinks);
        ExportObjects(externalLinks);
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
}

void SceneExporter::ExportSceneFileInternal(const FilePath& scenePathname, ExportedObjectCollection& exportedObjects)
{
    //Load scene from *.sc2
    ScopedPtr<Scene> scene(new Scene());
    if (SceneFileV2::ERROR_NO_ERROR == scene->LoadScene(scenePathname))
    {
        ExportScene(scene, scenePathname, exportedObjects);
    }
    else
    {
        Logger::Error("[SceneExporterV2::%s] Can't open file %s", __FUNCTION__, scenePathname.GetAbsolutePathname().c_str());
    }

    RenderObjectsFlusher::Flush();
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
};
}

namespace SceneExporterLocal
{
DAVA::FilePath CompressTexture(const DAVA::eGPUFamily gpu, DAVA::TextureConverter::eConvertQuality quality, DAVA::TextureDescriptor& descriptor)
{
    DVASSERT(GPUFamilyDescriptor::IsGPUForDevice(gpu));

    const bool needToConvert = !descriptor.IsCompressedTextureActual(gpu);
    if (needToConvert)
    {
        DAVA::Logger::Warning("Need recompress texture: %s", descriptor.GetSourceTexturePathname().GetAbsolutePathname().c_str());
        return TextureConverter::ConvertTexture(descriptor, gpu, true, quality);
    }

    return descriptor.CreateSavePathnameForGPU(gpu);
}
}

void SceneExporter::ExportTextureFile(const FilePath& descriptorPathname, const String& descriptorLink)
{
    std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(descriptorPathname));
    if (!descriptor)
    {
        Logger::Error("Can't create descriptor for pathname %s", descriptorPathname.GetAbsolutePathname().c_str());
        return;
    }

    bool texturesExported = ExportTextures(*descriptor);
    if (texturesExported)
    {
        if (exportingParams.exportForGPUs.size() == 1)
        {
            descriptor->Export(exportingParams.dataFolder + descriptorLink, exportingParams.exportForGPUs[0]);
        }
        else
        {
            descriptor->Save(exportingParams.dataFolder + descriptorLink);
        }
    }
}

bool SceneExporter::ExportTextures(DAVA::TextureDescriptor& descriptor)
{
    DAVA::FilePath sourceFilePath = descriptor.GetSourceTexturePathname();
    DAVA::ImageInfo imgInfo = DAVA::ImageSystem::Instance()->GetImageInfo(sourceFilePath);

    DAVA::Map<DAVA::eGPUFamily, bool> exportedStatus;

    { // compress images
        for (DAVA::eGPUFamily gpu : exportingParams.exportForGPUs)
        {
            if (gpu == DAVA::eGPUFamily::GPU_ORIGIN)
            {
                DAVA::ImageFormat targetFormat = static_cast<DAVA::ImageFormat>(descriptor.compression[gpu].imageFormat);
                if (exportingParams.useHDTextures && (targetFormat != DAVA::ImageFormat::IMAGE_FORMAT_DDS && targetFormat != IMAGE_FORMAT_PVR))
                {
                    Logger::Error("Can't create HD texure for ", descriptor.pathname.GetStringValue().c_str(), GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(gpu));

                    exportedStatus[gpu] = false;
                    continue;
                }
            }
            else if (DAVA::GPUFamilyDescriptor::IsGPUForDevice(gpu))
            {
                DAVA::PixelFormat format = descriptor.GetPixelFormatForGPU(gpu);
                if (format == DAVA::PixelFormat::FORMAT_INVALID)
                {
                    Logger::Error("Not selected export format for pathname %s for %s", descriptor.pathname.GetStringValue().c_str(), GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(gpu));

                    exportedStatus[gpu] = false;
                    continue;
                }
                if (!TextureDescriptorValidator::IsImageValidForFormat(imgInfo, format))
                {
                    Logger::Error("Can't export non-square texture %s into compression format %s",
                                  descriptor.pathname.GetAbsolutePathname().c_str(), GlobalEnumMap<DAVA::PixelFormat>::Instance()->ToString(format));

                    exportedStatus[gpu] = false;
                    continue;
                }

                SceneExporterLocal::CompressTexture(gpu, exportingParams.quality, descriptor);
            }
            else if (gpu != DAVA::eGPUFamily::GPU_ORIGIN)
            {
                Logger::Error("Has no code for GPU %d (%s)", gpu, GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(gpu));
                exportedStatus[gpu] = false;
            }
        }
    }

    //modify descriptors in data
    descriptor.dataSettings.SetSeparateHDTextures(exportingParams.useHDTextures);

    { // copy or separate images
        for (DAVA::eGPUFamily gpu : exportingParams.exportForGPUs)
        {
            if (exportedStatus.count(gpu) != 0)
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

                        copied = copied && CopyFile(faceName);
                    }
                }
                else
                {
                    copied = CopyFile(descriptor.GetSourceTexturePathname());
                }
            }
            else if (DAVA::GPUFamilyDescriptor::IsGPUForDevice(gpu))
            {
                DAVA::FilePath compressedName = descriptor.CreateSavePathnameForGPU(gpu);
                if (exportingParams.useHDTextures)
                {
                    copied = SplitHDTexture(descriptor, gpu, compressedName);
                }
                else
                {
                    copied = CopyFile(compressedName);
                }
            }

            if (!copied)
            {
                exportedStatus[gpu] = false;
            }
        }
    }

    return exportedStatus.empty();
}

void SceneExporter::ExportHeightmapFile(const FilePath& heightmapPathname, const String& heightmapLink)
{
    CopyFile(heightmapPathname, heightmapLink);
}

bool SceneExporter::SplitHDTexture(const DAVA::TextureDescriptor& descriptor, DAVA::eGPUFamily gpu, const DAVA::FilePath& compressedTexturePath) const
{
    DAVA::Vector<DAVA::Image*> loadedImages;
    SCOPE_EXIT
    {
        for (DAVA::Image* image : loadedImages)
        {
            SafeRelease(image);
        }
    };

    DAVA::eErrorCode loadError = DAVA::ImageSystem::Instance()->LoadWithoutDecompession(compressedTexturePath, loadedImages, 0, 0);
    if (loadError != DAVA::eErrorCode::SUCCESS || loadedImages.empty())
    {
        Logger::Error("Can't load %s", compressedTexturePath.GetStringValue().c_str());
        return false;
    }

    DAVA::PixelFormat targetFormat = descriptor.GetPixelFormatForGPU(gpu);
    DVASSERT(targetFormat == loadedImages[0]->format);

    auto createOutPathname = [&](const DAVA::FilePath& pathname)
    {
        DAVA::String fileLink = pathname.GetRelativePathname(exportingParams.dataSourceFolder);
        return exportingParams.dataFolder + fileLink;
    };

    auto saveImages = [&](const DAVA::FilePath& path, const DAVA::Vector<DAVA::Image*> images)
    {
        DAVA::eErrorCode saveError = DAVA::ImageSystem::Instance()->Save(path, images, targetFormat);
        if (saveError != DAVA::eErrorCode::SUCCESS)
        {
            Logger::Error("Can't save %s", path.GetStringValue().c_str());
            return false;
        }
        return true;
    };

    DAVA::Vector<DAVA::FilePath> imagePathnames = descriptor.CreateLoadPathnamesForGPU(gpu);
    DAVA::size_type mipmapsCount = loadedImages.size();
    DAVA::size_type imagesCount = imagePathnames.size();

    if (mipmapsCount < imagesCount)
    {
        Logger::Error("Can't split HD level for for %s", compressedTexturePath.GetStringValue().c_str());
        return false;
    }

    DAVA::size_type multipleImageIndex = imagesCount - 1;
    for (DAVA::size_type mip = 0; mip < multipleImageIndex; ++mip)
    {
        bool saved = saveImages(createOutPathname(imagePathnames[mip]), { loadedImages[mip] });
        if (!saved)
        {
            return false;
        }
    }

    auto mipImagesIterator = loadedImages.begin();
    std::advance(mipImagesIterator, multipleImageIndex);

    DAVA::Vector<DAVA::Image*> multipleMipImages(mipImagesIterator, loadedImages.end());
    return saveImages(createOutPathname(imagePathnames[multipleImageIndex]), multipleMipImages);
}

bool SceneExporter::CopyFile(const FilePath& filePath) const
{
    String workingPathname = filePath.GetRelativePathname(exportingParams.dataSourceFolder);
    return CopyFile(filePath, workingPathname);
}

bool SceneExporter::CopyFile(const FilePath& filePath, const String& fileLink) const
{
    FilePath newFilePath = exportingParams.dataFolder + fileLink;

    bool retCopy = FileSystem::Instance()->CopyFile(filePath, newFilePath, true);
    if (!retCopy)
    {
        Logger::Error("Can't copy %s to %s", fileLink.c_str(), newFilePath.GetAbsolutePathname().c_str());
    }

    return retCopy;
}

bool SceneExporter::ExportScene(Scene* scene, const FilePath& scenePathname, ExportedObjectCollection& exportedObjects)
{
    String relativeSceneFilename = scenePathname.GetRelativePathname(exportingParams.dataSourceFolder);
    FilePath outScenePathname = exportingParams.dataFolder + relativeSceneFilename;

    SceneExporterInternal::PrepareSceneToExport(scene, exportingParams.optimizeOnExport);

    SceneExporterInternal::CollectHeightmapPathname(scene, exportingParams.dataSourceFolder, exportedObjects); //must be first
    SceneExporterInternal::CollectTextureDescriptors(scene, exportingParams.dataSourceFolder, exportedObjects);

    // save scene to new place
    FilePath tempSceneName = FilePath::CreateWithNewExtension(scenePathname, ".exported.sc2");
    scene->SaveScene(tempSceneName, exportingParams.optimizeOnExport);

    bool moved = FileSystem::Instance()->MoveFile(tempSceneName, outScenePathname, true);
    if (!moved)
    {
        Logger::Error("Can't move file %s into %s", tempSceneName.GetAbsolutePathname().c_str(), outScenePathname.GetAbsolutePathname().c_str());
        FileSystem::Instance()->DeleteFile(tempSceneName);
        return false;
    }

    return true;
}

void SceneExporter::ExportObjects(const ExportedObjectCollection& exportedObjects)
{
    UnorderedSet<String> folders;
    folders.reserve(exportedObjects.size());
    folders.insert(""); // To create root directory for scene

    String inFolderString = exportingParams.dataSourceFolder.GetAbsolutePathname();
    String outFolderString = exportingParams.dataFolder.GetAbsolutePathname();

    //enumerate target folders for exported objects
    for (const auto& object : exportedObjects)
    {
        const String& link = object.relativePathname;

        const String::size_type slashpos = link.rfind(String("/"));
        if (slashpos != String::npos)
        {
            folders.insert(link.substr(0, slashpos + 1));
        }
    }

    //Create folders in Data Folder to copy objects
    for (const auto& folder : folders)
    {
        FileSystem::Instance()->CreateDirectory(outFolderString + folder, true);
    }

    using ExporterFunction = Function<void(const FilePath&, const String&)>;
    Array<ExporterFunction, OBJECT_COUNT> exporters =
    { { MakeFunction(this, &SceneExporter::ExportSceneFile),
        MakeFunction(this, &SceneExporter::ExportTextureFile),
        MakeFunction(this, &SceneExporter::ExportHeightmapFile) } };

    for (const auto& object : exportedObjects)
    {
        if (object.type != OBJECT_NONE && object.type < OBJECT_COUNT)
        {
            FilePath path(inFolderString + object.relativePathname);
            exporters[object.type](path, object.relativePathname);
        }
        else
        {
            Logger::Error("Found wrong path: %s", object.relativePathname.c_str());
            continue; //need continue exporting of resources in any case.
        }
    }
}
