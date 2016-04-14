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
<<<<<<< HEAD
    uint64 moveTime = SystemTimer::Instance()->AbsoluteMS() - moveStart;

    SceneValidator::Instance()->SetPathForChecking(oldPath);

    if (!sceneWasExportedCorrectly)
    { // *** to highlight this message from other error messages
        Logger::Error("***  Scene %s was exported with errors!", fileName.GetAbsolutePathname().c_str());
    }

    uint64 exportTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
    Logger::Info("Export Status\n\tScene: %s\n\tExport time: %ldms\n\tRemove editor nodes time: %ldms\n\tRemove custom properties: %ldms\n\tExport descriptors: %ldms\n\tValidation time: %ldms\n\tLandscape time: %ldms\n\tSave time: %ldms\n\tMove time: %ldms\n\tErrors occured: %d",
                 fileName.GetStringValue().c_str(), exportTime, removeEditorNodesTime, removeEditorCPTime, exportDescriptorsTime, validationTime, landscapeTime, saveTime, moveTime, !sceneWasExportedCorrectly);

    return;
=======
>>>>>>> re_stable
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

void SceneExporter::SetCompressionParams(const eGPUFamily gpu, TextureConverter::eConvertQuality quality_)
{
    exportForGPU = gpu;
    exportForAllGPUs = (gpu == GPU_FAMILY_COUNT);

    quality = quality_;
}

namespace TextureDescriptorValidator
{
bool IsFormatSelectedForGPU(const DAVA::TextureDescriptor& descriptor, DAVA::eGPUFamily gpu)
{
    if (descriptor.compression[gpu].format == DAVA::FORMAT_INVALID)
    {
        Logger::Error("Not selected export format for pathname %s for %s", descriptor.pathname.GetStringValue().c_str(), GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(gpu));
        return false;
    }
    return true;
}

bool IsImageValidForFormat(const DAVA::ImageInfo& info, const DAVA::PixelFormat format, const DAVA::FilePath& pathname)
{
    if (info.width != info.height && (format == DAVA::FORMAT_PVR2 || format == DAVA::FORMAT_PVR4))
    {
        Logger::Error("Can't export non-square texture %s into compression format %s",
            pathname.GetAbsolutePathname().c_str(), GlobalEnumMap<PixelFormat>::Instance()->ToString(format));

        return false;
    }

    return true;
};

}

void SceneExporter::SetFolders(const FilePath& dataFolder_, const FilePath& dataSourceFolder_)
{
    DVASSERT(dataFolder_.IsDirectoryPathname());
    DVASSERT(dataSourceFolder_.IsDirectoryPathname());

    dataFolder = dataFolder_;
    dataSourceFolder = dataSourceFolder_;
}

void SceneExporter::EnableOptimizations(bool enable)
{
    optimizeOnExport = enable;
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

    FilePath outScenePathname = dataFolder + sceneLink;
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
        SceneExporterCache::CalculateSceneKey(scenePathname, sceneLink, cacheKey, static_cast<uint32>(optimizeOnExport));

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

void SceneExporter::ExportTextureFile(const FilePath& descriptorPathname, const String& descriptorLink)
{
    std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(descriptorPathname));
    if (!descriptor)
    {
        Logger::Error("Can't create descriptor for pathname %s", descriptorPathname.GetAbsolutePathname().c_str());
        return;
    }

    descriptor->exportedAsGpuFamily = exportForGPU;
    descriptor->format = descriptor->GetPixelFormatForGPU(exportForGPU);
    if (GPUFamilyDescriptor::IsGPUForDevice(exportForGPU))
    {
        if (descriptor->format == FORMAT_INVALID)
        {
            Logger::Error("Not selected export format for pathname %s", descriptorPathname.GetAbsolutePathname().c_str());
            return;
        }

        FilePath sourceFilePath = descriptor->GetSourceTexturePathname();
        DAVA::ImageInfo imgInfo = DAVA::ImageSystem::Instance()->GetImageInfo(sourceFilePath);
        if (imgInfo.width != imgInfo.height && (descriptor->format == FORMAT_PVR2 || descriptor->format == FORMAT_PVR4))
        {
            Logger::Error("Can't export non-square texture %s into compression format %s",
                          descriptorPathname.GetAbsolutePathname().c_str(),
                          GlobalEnumMap<PixelFormat>::Instance()->ToString(descriptor->format));
            return;
        }

        FilePath compressedTexturePathname = CompressTexture(*descriptor);
        CopyFile(compressedTexturePathname);
    }
    else
    {
        CopySourceTexture(*descriptor);
    }

    FilePath exportedDescriptorPath = dataFolder + descriptorLink;
    descriptor->Export(exportedDescriptorPath);
}

void SceneExporter::ExportHeightmapFile(const FilePath& heightmapPathname, const String& heightmapLink)
{
    CopyFile(heightmapPathname, heightmapLink);
}

FilePath SceneExporter::CompressTexture(TextureDescriptor& descriptor) const
{
    DVASSERT(GPUFamilyDescriptor::IsGPUForDevice(exportForGPU));

    FilePath compressedTexureName = descriptor.CreatePathnameForGPU(exportForGPU);

    const bool needToConvert = !descriptor.IsCompressedTextureActual(exportForGPU);
    if (needToConvert)
    {
        Logger::Warning("Need recompress texture: %s", descriptor.GetSourceTexturePathname().GetAbsolutePathname().c_str());
        return TextureConverter::ConvertTexture(descriptor, exportForGPU, true, quality);
    }

    return compressedTexureName;
}

void SceneExporter::CopySourceTexture(TextureDescriptor& descriptor) const
{
    if (descriptor.IsCubeMap())
    {
        Vector<FilePath> faceNames;
        descriptor.GetFacePathnames(faceNames);
        for (const auto& faceName : faceNames)
        {
            if (faceName.IsEmpty())
                continue;

            CopyFile(faceName);
        }
    }
    else
    {
        CopyFile(descriptor.GetSourceTexturePathname());
    }
}

bool SceneExporter::CopyFile(const FilePath& filePath) const
{
    String workingPathname = filePath.GetRelativePathname(dataSourceFolder);
    return CopyFile(filePath, workingPathname);
}

bool SceneExporter::CopyFile(const FilePath& filePath, const String& fileLink) const
{
    FilePath newFilePath = dataFolder + fileLink;

    bool retCopy = FileSystem::Instance()->CopyFile(filePath, newFilePath, true);
    if (!retCopy)
    {
        Logger::Error("Can't copy %s to %s", fileLink.c_str(), newFilePath.GetAbsolutePathname().c_str());
    }

    return retCopy;
}

namespace SceneExporterLocal
{
void CompressTexture(const Vector<eGPUFamily> gpus, TextureConverter::eConvertQuality quality, const TextureDescriptor* descriptor)
{
    for (auto& gpu : gpus)
    {
        if (!GPUFamilyDescriptor::IsGPUForDevice(gpu))
        {
            continue;
        }

        FilePath compressedTexureName = descriptor->CreatePathnameForGPU(gpu);

        bool fileExists = FileSystem::Instance()->Exists(compressedTexureName);
        bool needToConvert = SceneValidator::IsTextureChanged(descriptor, gpu);

        if (needToConvert || !fileExists)
        {
            //TODO: convert to pvr/dxt
            //TODO: do we need to convert to pvr if needToConvert is false, but *.pvr file isn't at filesystem
            TextureConverter::ConvertTexture(*descriptor, gpu, true, quality);
        }
    }
}

bool CopyCompressedTexure(const Vector<eGPUFamily> gpus, SceneUtils& sceneUtils, const TextureDescriptor* descriptor, Set<String>& errorLog)
{
    bool result = true;

    for (auto& gpu : gpus)
    {
        if (GPUFamilyDescriptor::IsGPUForDevice(gpu))
        {
            FilePath compressedTexureName = descriptor->CreatePathnameForGPU(gpu);
            result = result && sceneUtils.CopyFile(compressedTexureName, errorLog);
        }
        else
        {
            if (descriptor->IsCubeMap())
            {
                Vector<FilePath> faceNames;
                descriptor->GetFacePathnames(faceNames);
                for (auto& faceName : faceNames)
                {
                    if (faceName.IsEmpty())
                        continue;
                    result = result && sceneUtils.CopyFile(faceName, errorLog);
                }
            }
            else
            {
                result = result && sceneUtils.CopyFile(descriptor->GetSourceTexturePathname(), errorLog);
            }
        }
    }

    return result;
}
}

bool SceneExporter::ExportScene(Scene* scene, const FilePath& scenePathname, ExportedObjectCollection& exportedObjects)
{
    String relativeSceneFilename = scenePathname.GetRelativePathname(dataSourceFolder);
    FilePath outScenePathname = dataFolder + relativeSceneFilename;

    SceneExporterInternal::PrepareSceneToExport(scene, optimizeOnExport);

    SceneExporterInternal::CollectHeightmapPathname(scene, dataSourceFolder, exportedObjects); //must be first
    SceneExporterInternal::CollectTextureDescriptors(scene, dataSourceFolder, exportedObjects);

    // save scene to new place
    FilePath tempSceneName = FilePath::CreateWithNewExtension(scenePathname, ".exported.sc2");
    scene->SaveScene(tempSceneName, optimizeOnExport);

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

    String inFolderString = dataSourceFolder.GetAbsolutePathname();
    String outFolderString = dataFolder.GetAbsolutePathname();

    //enumerate target folders for exported objects
    for (const auto& object : exportedObjects)
    {
        const String& link = object.relativePathname;

        const String::size_type slashpos = link.rfind(String("/"));
        if (slashpos != String::npos)
        {
            folders.insert(link.substr(0, slashpos + 1));
        }
        else
        {
            if (descriptor->IsCubeMap())
            {
                Vector<FilePath> faceNames;
                descriptor->GetFacePathnames(faceNames);
                for (auto& faceName : faceNames)
                {
                    if (faceName.IsEmpty())
                        continue;
                    result = result && sceneUtils.CopyFile(faceName, errorLog);
                }
            }
            else
            {
                result = result && sceneUtils.CopyFile(descriptor->GetSourceTexturePathname(), errorLog);
            }
        }
    }

    return result;
}

void SceneExporter::CompressTexture(const TextureDescriptor* descriptor)
{
    if (exportForAllGPUs)
    {
        static const Vector<eGPUFamily> deviceGPUs = { GPU_POWERVR_IOS, GPU_POWERVR_ANDROID, GPU_TEGRA, GPU_MALI, GPU_ADRENO, GPU_DX11 };
        SceneExporterLocal::CompressTexture(deviceGPUs, quality, descriptor);
    }
    else
    {
        SceneExporterLocal::CompressTexture({ exportForGPU }, quality, descriptor);
    }
}

bool SceneExporter::CopyCompressedTexture(const TextureDescriptor* descriptor, Set<String>& errorLog)
{
    if (exportForAllGPUs)
    {
        static const Vector<eGPUFamily> GPUs = { GPU_ORIGIN, GPU_POWERVR_IOS, GPU_POWERVR_ANDROID, GPU_TEGRA, GPU_MALI, GPU_ADRENO, GPU_DX11 };
        return SceneExporterLocal::CopyCompressedTexure(GPUs, sceneUtils, descriptor, errorLog);
    }
    else
    {
        return SceneExporterLocal::CopyCompressedTexure({ exportForGPU }, sceneUtils, descriptor, errorLog);
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
