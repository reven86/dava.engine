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
#include "Deprecated/SceneValidator.h"

#include "TextureCompression/TextureConverter.h"

#include "Render/TextureDescriptor.h"
#include "Render/GPUFamilyDescriptor.h"

#include "StringConstants.h"

#include "Qt/Scene/SceneHelper.h"
#include "Qt/Main/QtUtils.h"

using namespace DAVA;

void SceneExporter::SetInFolder(const FilePath& folderPathname)
{
    sceneUtils.SetInFolder(folderPathname);
}

void SceneExporter::SetOutFolder(const FilePath& folderPathname)
{
    sceneUtils.SetOutFolder(folderPathname);
}

void SceneExporter::SetGPUForExporting(const eGPUFamily newGPU)
{
    exportForGPU = newGPU;
    exportForAllGPUs = (newGPU == GPU_FAMILY_COUNT);
}

void SceneExporter::ExportSceneFolder(const String& folderName, Set<String>& errorLog)
{
    FilePath folderPathname = sceneUtils.dataSourceFolder + folderName;
    folderPathname.MakeDirectoryPathname();

    ScopedPtr<FileList> fileList(new FileList(folderPathname));
    for (int32 i = 0; i < fileList->GetCount(); ++i)
    {
        FilePath pathname = fileList->GetPathname(i);
        if (fileList->IsDirectory(i))
        {
            if (!fileList->IsNavigationDirectory(i))
            {
                String workingPathname = pathname.GetRelativePathname(sceneUtils.dataSourceFolder);
                ExportSceneFolder(workingPathname, errorLog);
            }
        }
        else
        {
            if (pathname.IsEqualToExtension(".sc2"))
            {
                String::size_type exportedPos = pathname.GetAbsolutePathname().find(".exported.sc2");
                if (exportedPos != String::npos)
                {
                    //Skip temporary files, created during export
                    continue;
                }

                String workingPathname = pathname.GetRelativePathname(sceneUtils.dataSourceFolder);
                ExportSceneFile(workingPathname, errorLog);
            }
        }
    }
}

void SceneExporter::ExportSceneFile(const String& fileName, Set<String>& errorLog)
{
    FilePath filePath = sceneUtils.dataSourceFolder + fileName;
    Logger::Info("Export of %s", filePath.GetStringValue().c_str());

    //Load scene from *.sc2
    ScopedPtr<Scene> scene(new Scene());
    if (SceneFileV2::ERROR_NO_ERROR == scene->LoadScene(filePath))
    {
        ExportScene(scene, filePath, errorLog);
    }
    else
    {
        errorLog.insert(Format("[SceneExporter::ExportFile] Can't open file %s", filePath.GetAbsolutePathname().c_str()));
    }

    RenderObjectsFlusher::Flush();
}

void SceneExporter::ExportTextureFolder(const String& folderName, Set<String>& errorLog)
{
    FilePath folderPathname = sceneUtils.dataSourceFolder + folderName;
    folderPathname.MakeDirectoryPathname();

    ScopedPtr<FileList> fileList(new FileList(folderPathname));
    for (int32 i = 0; i < fileList->GetCount(); ++i)
    {
        FilePath pathname = fileList->GetPathname(i);
        if (fileList->IsDirectory(i))
        {
            if (!fileList->IsNavigationDirectory(i))
            {
                String workingPathname = pathname.GetRelativePathname(sceneUtils.dataSourceFolder);
                ExportTextureFolder(workingPathname, errorLog);
            }
        }
        else
        {
            if (pathname.IsEqualToExtension(".tex"))
            {
                String workingPathname = pathname.GetRelativePathname(sceneUtils.dataSourceFolder);
                ExportTextureFile(workingPathname, errorLog);
            }
        }
    }
}

void SceneExporter::ExportTextureFile(const String& fileName, Set<String>& errorLog)
{
    FilePath filePath = sceneUtils.dataSourceFolder + fileName;
    Logger::Info("Export of %s", filePath.GetStringValue().c_str());

    ExportTextureDescriptor(filePath, errorLog);
}

void SceneExporter::ExportScene(Scene* scene, const FilePath& fileName, Set<String>& errorLog)
{
    uint64 startTime = SystemTimer::Instance()->AbsoluteMS();

    //Create destination folder
    String relativeFilename = fileName.GetRelativePathname(sceneUtils.dataSourceFolder);
    sceneUtils.workingFolder = fileName.GetDirectory().GetRelativePathname(sceneUtils.dataSourceFolder);

    FileSystem::Instance()->CreateDirectory(sceneUtils.dataFolder + sceneUtils.workingFolder, true);

    uint64 removeEditorNodesStart = SystemTimer::Instance()->AbsoluteMS();
    //Export scene data
    RemoveEditorNodes(scene);
    uint64 removeEditorNodesTime = SystemTimer::Instance()->AbsoluteMS() - removeEditorNodesStart;

    uint64 removeEditorCPStart = SystemTimer::Instance()->AbsoluteMS();
    if (optimizeOnExport)
    {
        RemoveEditorCustomProperties(scene);
    }
    uint64 removeEditorCPTime = SystemTimer::Instance()->AbsoluteMS() - removeEditorCPStart;

    uint64 exportDescriptorsStart = SystemTimer::Instance()->AbsoluteMS();
    bool sceneWasExportedCorrectly = ExportDescriptors(scene, errorLog);
    uint64 exportDescriptorsTime = SystemTimer::Instance()->AbsoluteMS() - exportDescriptorsStart;

    uint64 validationStart = SystemTimer::Instance()->AbsoluteMS();
    FilePath oldPath = SceneValidator::Instance()->SetPathForChecking(sceneUtils.dataSourceFolder);
    SceneValidator::Instance()->ValidateScene(scene, fileName, errorLog);
    //SceneValidator::Instance()->ValidateScales(scene, errorLog);
    uint64 validationTime = SystemTimer::Instance()->AbsoluteMS() - validationStart;

    uint64 landscapeStart = SystemTimer::Instance()->AbsoluteMS();
    sceneWasExportedCorrectly &= ExportLandscape(scene, errorLog);
    uint64 landscapeTime = SystemTimer::Instance()->AbsoluteMS() - landscapeStart;

    // save scene to new place
    uint64 saveStart = SystemTimer::Instance()->AbsoluteMS();
    FilePath tempSceneName = FilePath::CreateWithNewExtension(sceneUtils.dataSourceFolder + relativeFilename, ".exported.sc2");
    scene->SaveScene(tempSceneName, optimizeOnExport);
    uint64 saveTime = SystemTimer::Instance()->AbsoluteMS() - saveStart;

    uint64 moveStart = SystemTimer::Instance()->AbsoluteMS();
    bool moved = FileSystem::Instance()->MoveFile(tempSceneName, sceneUtils.dataFolder + relativeFilename, true);
    if (!moved)
    {
        errorLog.insert(Format("Can't move file %s", fileName.GetAbsolutePathname().c_str()));
        sceneWasExportedCorrectly = false;
    }
    uint64 moveTime = SystemTimer::Instance()->AbsoluteMS() - moveStart;

    SceneValidator::Instance()->SetPathForChecking(oldPath);

    if (!sceneWasExportedCorrectly)
    { // *** to highlight this message from other error messages
        Logger::Error("***  Scene %s was exported with errors!", fileName.GetAbsolutePathname().c_str());
    }

    uint64 exportTime = SystemTimer::Instance()->AbsoluteMS() - startTime;
    Logger::Info("Export Status\n\tScene: %s\n\tExport time: %ldms\n\tRemove editor nodes time: %ldms\n\tRemove custom properties: %ldms\n\tExport descriptors: %ldms\n\tValidation time: %ldms\n\tLandscape time: %ldms\n\tVegetation time: %ldms\n\tSave time: %ldms\n\tMove time: %ldms\n\tErrors occured: %d",
                 fileName.GetStringValue().c_str(), exportTime, removeEditorNodesTime, removeEditorCPTime, exportDescriptorsTime, validationTime, landscapeTime, saveTime, moveTime, !sceneWasExportedCorrectly);

    return;
}

void SceneExporter::RemoveEditorNodes(DAVA::Entity* rootNode)
{
    DVASSERT(rootNode != nullptr);

    //Remove scene nodes
    Vector<Entity*> scenenodes;
    rootNode->GetChildNodes(scenenodes);

    //remove nodes from hierarhy
    for (auto& entity : scenenodes)
    {
        String::size_type pos = entity->GetName().find(ResourceEditor::EDITOR_BASE);
        if (String::npos != pos)
        {
            DVASSERT(entity->GetParent() != nullptr);
            entity->GetParent()->RemoveNode(entity);
        }
    }
}

void SceneExporter::RemoveEditorCustomProperties(Entity* rootNode)
{
    Vector<Entity*> scenenodes;
    rootNode->GetChildNodes(scenenodes);

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

    Vector<Entity*>::const_iterator endIt = scenenodes.end();
    for (Vector<Entity*>::iterator it = scenenodes.begin(); it != endIt; ++it)
    {
        Entity* node = *it;

        KeyedArchive* props = GetCustomPropertiesArchieve(node);
        if (props)
        {
            const KeyedArchive::UnderlyingMap propsMap = props->GetArchieveData();

            auto endIt = propsMap.end();
            for (auto it = propsMap.begin(); it != endIt; ++it)
            {
                String key = it->first;

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
                node->RemoveComponent(DAVA::Component::CUSTOM_PROPERTIES_COMPONENT);
            }
        }
    }
}

bool SceneExporter::ExportDescriptors(DAVA::Scene* scene, Set<String>& errorLog)
{
    bool allDescriptorsWereExported = true;

    SceneHelper::TextureCollector collector(SceneHelper::TextureCollector::IncludeNullTextures);
    SceneHelper::EnumerateSceneTextures(scene, collector);

    for (const auto& scTex : collector.GetTextures())
    {
        const DAVA::FilePath& path = scTex.first;
        if (path.GetType() == DAVA::FilePath::PATH_IN_MEMORY)
        {
            continue;
        }

        DVASSERT(path.IsEmpty() == false);

        allDescriptorsWereExported &= ExportTextureDescriptor(path, errorLog);
    }

    return allDescriptorsWereExported;
}

bool SceneExporter::ExportTextureDescriptor(const FilePath& pathname, Set<String>& errorLog)
{
    std::unique_ptr<TextureDescriptor> descriptor(TextureDescriptor::CreateFromFile(pathname));
    if (!descriptor)
    {
        errorLog.insert(Format("Can't create descriptor for pathname %s", pathname.GetAbsolutePathname().c_str()));
        return false;
    }

    if (GPUFamilyDescriptor::IsGPUForDevice(exportForGPU) && (descriptor->GetPixelFormatForGPU(exportForGPU) == FORMAT_INVALID))
    {
        errorLog.insert(Format("Not selected export format for pathname %s", pathname.GetAbsolutePathname().c_str()));
        return false;
    }

    FilePath sourceFilePath = descriptor->GetSourceTexturePathname();
    ImageInfo imgInfo = ImageSystem::Instance()->GetImageInfo(sourceFilePath);
    if (imgInfo.width != imgInfo.height && (descriptor->format == FORMAT_PVR2 || descriptor->format == FORMAT_PVR4))
    {
        errorLog.insert(Format("Can't export non-square texture %s into compression format %s",
                               pathname.GetAbsolutePathname().c_str(),
                               GlobalEnumMap<PixelFormat>::Instance()->ToString(descriptor->format)));
        return false;
    }

    String workingPathname = descriptor->pathname.GetRelativePathname(sceneUtils.dataSourceFolder);
    sceneUtils.PrepareFolderForCopyFile(workingPathname, errorLog);

    bool isExported = ExportTexture(descriptor.get(), errorLog);
    if (isExported)
    {
        if (exportForAllGPUs)
        {
            sceneUtils.CopyFile(pathname, errorLog);
        }
        else
        {
            descriptor->Export(sceneUtils.dataFolder + workingPathname, exportForGPU);
        }
    }

    return isExported;
}

bool SceneExporter::ExportTexture(const TextureDescriptor* descriptor, Set<String>& errorLog)
{
    CompressTexture(descriptor);
    return CopyCompressedTexture(descriptor, errorLog);
}

bool SceneExporter::ExportLandscape(Scene* scene, Set<String>& errorLog)
{
    DVASSERT(scene);

    Landscape* landscape = FindLandscape(scene);
    if (landscape)
    {
        return sceneUtils.CopyFile(landscape->GetHeightmapPathname(), errorLog);
    }

    return true;
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
            result &= sceneUtils.CopyFile(compressedTexureName, errorLog);
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
                    result &= sceneUtils.CopyFile(faceName, errorLog);
                }
            }
            else
            {
                result &= sceneUtils.CopyFile(descriptor->GetSourceTexturePathname(), errorLog);
            }
        }
    }

    return result;
}
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
}

void SceneExporter::EnableOptimizations(bool enable)
{
    optimizeOnExport = enable;
}

void SceneExporter::SetCompressionQuality(TextureConverter::eConvertQuality _quality)
{
    quality = _quality;
}
