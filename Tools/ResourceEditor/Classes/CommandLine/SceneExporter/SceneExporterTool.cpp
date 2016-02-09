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

#include "CommandLine/SceneExporter/SceneExporterTool.h"
#include "CommandLine/SceneExporter/SceneExporter.h"
#include "CommandLine/OptionName.h"

#include "FileSystem/File.h"
#include "FileSystem/FileList.h"
#include "Render/GPUFamilyDescriptor.h"
#include "Render/Highlevel/Heightmap.h"

using namespace DAVA;

namespace SceneExporterToolInternal
{
void CollectObjectsFromFolder(const DAVA::FilePath& folderPathname, const FilePath& inFolder, const SceneExporter::eExportedObjectType objectType, SceneExporter::ExportedObjectCollection& exportedObjects)
{
    DVASSERT(folderPathname.IsDirectoryPathname())

    ScopedPtr<FileList> fileList(new FileList(folderPathname));
    for (int32 i = 0, count = fileList->GetCount(); i < count; ++i)
    {
        const FilePath& pathname = fileList->GetPathname(i);
        if (fileList->IsDirectory(i))
        {
            if (!fileList->IsNavigationDirectory(i))
            {
                CollectObjectsFromFolder(pathname, inFolder, objectType, exportedObjects);
            }
        }
        else
        {
            if ((SceneExporter::OBJECT_SCENE == objectType) && (pathname.IsEqualToExtension(".sc2")))
            {
                String::size_type exportedPos = pathname.GetAbsolutePathname().find(".exported.sc2");
                if (exportedPos != String::npos)
                {
                    Logger::Warning("[SceneExporterTool] Found temporary file: %s\nPlease delete it manualy", pathname.GetAbsolutePathname().c_str());
                    continue;
                }

                exportedObjects.emplace_back(SceneExporter::OBJECT_SCENE, pathname.GetRelativePathname(inFolder));
            }
            else if ((SceneExporter::OBJECT_TEXTURE == objectType) && (pathname.IsEqualToExtension(".tex")))
            {
                exportedObjects.emplace_back(SceneExporter::OBJECT_TEXTURE, pathname.GetRelativePathname(inFolder));
            }
        }
    }
}

SceneExporter::eExportedObjectType GetObjectType(const FilePath& pathname)
{
    static const Vector<std::pair<SceneExporter::eExportedObjectType, String>> objectDefinition =
    {
      { SceneExporter::OBJECT_TEXTURE, ".tex" },
      { SceneExporter::OBJECT_SCENE, ".sc2" },
      { SceneExporter::OBJECT_HEIGHTMAP, Heightmap::FileExtension() },
    };

    for (const auto& def : objectDefinition)
    {
        if (pathname.IsEqualToExtension(def.second))
        {
            return def.first;
        }
    }

    return SceneExporter::OBJECT_NONE;
}

bool CollectObjectFromFileList(const FilePath& fileListPath, const FilePath& inFolder, SceneExporter::ExportedObjectCollection& exportedObjects)
{
    ScopedPtr<File> fileWithLinks(File::Create(fileListPath, File::OPEN | File::READ));
    if (!fileWithLinks)
    {
        Logger::Error("[SceneExporterTool] cannot open file with links %s", fileListPath.GetAbsolutePathname().c_str());
        return false;
    }

    bool isEof = false;
    do
    {
        String link = fileWithLinks->ReadLine();
        if (link.empty())
        {
            Logger::Warning("[SceneExporterTool] found empty string in file %s", fileListPath.GetAbsolutePathname().c_str());
            break;
        }

        FilePath exportedPathname = inFolder + link;
        if (exportedPathname.IsDirectoryPathname())
        {
            CollectObjectsFromFolder(exportedPathname, inFolder, SceneExporter::OBJECT_SCENE, exportedObjects);
        }
        else
        {
            const SceneExporter::eExportedObjectType objType = GetObjectType(exportedPathname);
            if (objType != SceneExporter::OBJECT_NONE)
            {
                exportedObjects.emplace_back(objType, std::move(link));
            }
        }

        isEof = fileWithLinks->IsEof();
    } while (!isEof);

    return true;
}

}



SceneExporterTool::SceneExporterTool()
    : CommandLineTool("-sceneexporter")
{
    options.AddOption(OptionName::Scene, VariantType(false), "Target object is scene, so we need to export *.sc2 files from folder");
    options.AddOption(OptionName::Texture, VariantType(false), "Target object is texture, so we need to export *.tex files from folder");

    options.AddOption(OptionName::InDir, VariantType(String("")), "Path for Project/DataSource/3d/ folder");
    options.AddOption(OptionName::OutDir, VariantType(String("")), "Path for Project/Data/3d/ folder");
    options.AddOption(OptionName::ProcessDir, VariantType(String("")), "Foldername from DataSource/3d/ for exporting");
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Filename from DataSource/3d/ for exporting");
    options.AddOption(OptionName::ProcessFileList, VariantType(String("")), "Pathname to file with filenames for exporting");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Full path for quality.yaml file");

    options.AddOption(OptionName::GPU, VariantType(String("origin")), "GPU family: PoverVR_iOS, PoverVR_Android, tegra, mali, adreno, origin, dx11");
    options.AddOption(OptionName::Quality, VariantType(static_cast<uint32>(TextureConverter::ECQ_DEFAULT)), "Quality of pvr/etc compression. Default is 4 - the best quality. Available values [0-4]");

    options.AddOption(OptionName::SaveNormals, VariantType(false), "Disable removing of normals from vertexes");
    options.AddOption(OptionName::deprecated_Export, VariantType(false), "Option says that we are doing export. Need remove after unification of command line options");

    options.AddOption(OptionName::UseAssetCache, VariantType(false), "Enables using AssetCache for scene");
    options.AddOption(OptionName::AssetCacheIP, VariantType(String("127.0.0.1")), "ip of adress of Asset Cache Server");
    options.AddOption(OptionName::AssetCachePort, VariantType(static_cast<uint32>(AssetCache::ASSET_SERVER_PORT)), "port of adress of Asset Cache Server");
    options.AddOption(OptionName::AssetCacheTimeout, VariantType(static_cast<uint32>(1)), "timeout for caching operations");
}

void SceneExporterTool::ConvertOptionsToParamsInternal()
{
    inFolder = options.GetOption(OptionName::InDir).AsString();
    outFolder = options.GetOption(OptionName::OutDir).AsString();
    filename = options.GetOption(OptionName::ProcessFile).AsString();
    foldername = options.GetOption(OptionName::ProcessDir).AsString();
    fileListPath = options.GetOption(OptionName::ProcessFileList).AsString();
    qualityConfigPath = options.GetOption(OptionName::QualityConfig).AsString();

    if (options.GetOption(OptionName::Texture).AsBool())
    {
        commandObject = SceneExporter::OBJECT_TEXTURE;
    }
    else if (options.GetOption(OptionName::Scene).AsBool() || options.GetOption(OptionName::deprecated_Export).AsBool())
    {
        commandObject = SceneExporter::OBJECT_SCENE;
    }

    String gpuName = options.GetOption(OptionName::GPU).AsString();
    requestedGPU = GPUFamilyDescriptor::GetGPUByName(gpuName);

    const uint32 qualityValue = options.GetOption(OptionName::Quality).AsUInt32();
    quality = Clamp(static_cast<TextureConverter::eConvertQuality>(qualityValue), TextureConverter::ECQ_FASTEST, TextureConverter::ECQ_VERY_HIGH);

    const bool saveNormals = options.GetOption(OptionName::SaveNormals).AsBool();
    optimizeOnExport = !saveNormals;

    useAssetCache = options.GetOption(OptionName::UseAssetCache).AsBool();
    if (useAssetCache)
    {
        connectionsParams.ip = options.GetOption(OptionName::AssetCacheIP).AsString();
        connectionsParams.port = static_cast<uint16>(options.GetOption(OptionName::AssetCachePort).AsUInt32());
        connectionsParams.timeoutms = options.GetOption(OptionName::AssetCacheTimeout).AsUInt32() * 1000; //ms
    }
}

bool SceneExporterTool::InitializeInternal()
{
    if (inFolder.IsEmpty())
    {
        Logger::Error("[SceneExporterTool] Input folder was not selected");
        return false;
    }
    inFolder.MakeDirectoryPathname();

    if (outFolder.IsEmpty())
    {
        Logger::Error("[SceneExporterTool] Output folder was not selected");
        return false;
    }

    outFolder.MakeDirectoryPathname();

    if (filename.empty() == false)
    {
        commandAction = ACTION_EXPORT_FILE;
    }
    else if (foldername.empty() == false)
    {
        commandAction = ACTION_EXPORT_FOLDER;
    }
    else if (fileListPath.IsEmpty() == false)
    {
        commandAction = ACTION_EXPORT_FILELIST;
    }
    else
    {
        Logger::Error("[SceneExporterTool] Target for exporting was not selected");
        return false;
    }

    if (requestedGPU == GPU_INVALID)
    {
        Logger::Error("[SceneExporterTool] Unsupported gpu parameter was selected");
        return false;
    }

    return true;
}

void SceneExporterTool::ProcessInternal()
{
    AssetCacheClient cacheClient(true);

    SceneExporter exporter;
    exporter.SetFolders(outFolder, inFolder);
    exporter.SetCompressionParams(requestedGPU, quality);
    exporter.EnableOptimizations(optimizeOnExport);

    if (useAssetCache)
    {
        AssetCache::ErrorCodes connected = cacheClient.ConnectBlocked(connectionsParams);
        if (connected == AssetCache::ERROR_OK)
        {
            String mashineName = WStringToString(DeviceInfo::GetName());
            DateTime timeNow = DateTime::Now();
            String timeString = WStringToString(timeNow.GetLocalizedDate()) + WStringToString(timeNow.GetLocalizedTime());

            exporter.SetCacheClient(&cacheClient, mashineName, timeString, "Resource Editor. Export scene");
        }
        else
        {
            useAssetCache = false;
        }
    }

    if (commandAction == ACTION_EXPORT_FILE)
    {
        commandObject = SceneExporterToolInternal::GetObjectType(inFolder + filename);
        if (commandObject == SceneExporter::OBJECT_NONE)
        {
            Logger::Error("[SceneExporterTool] found wrong filename %s", filename.c_str());
            return;
        }
        exportedObjects.emplace_back(commandObject, std::move(filename));
    }
    else if (commandAction == ACTION_EXPORT_FOLDER)
    {
        FilePath folderPathname(inFolder + foldername);
        folderPathname.MakeDirectoryPathname();

        SceneExporterToolInternal::CollectObjectsFromFolder(folderPathname, inFolder, commandObject, exportedObjects);
    }
    else if (commandAction == ACTION_EXPORT_FILELIST)
    {
        bool collected = SceneExporterToolInternal::CollectObjectFromFileList(fileListPath, inFolder, exportedObjects);
        if (!collected)
        {
            Logger::Error("[SceneExporterTool] Can't collect links from file %s", fileListPath.GetAbsolutePathname().c_str());
            return;
        }
    }

    exporter.ExportObjects(exportedObjects);

    if (useAssetCache)
    {
        cacheClient.Disconnect();
    }
}

FilePath SceneExporterTool::GetQualityConfigPath() const
{
    if (qualityConfigPath.IsEmpty())
    {
        return CreateQualityConfigPath(inFolder);
    }

    return qualityConfigPath;
}

