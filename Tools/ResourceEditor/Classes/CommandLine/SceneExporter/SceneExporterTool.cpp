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

#include "Project/ProjectManager.h"
#include "Utils/StringUtils.h"

using namespace DAVA;

namespace OptionName
{
static const String Scene = "-scene";
static const String Texture = "-texture";
static const String InDir = "-indir";
static const String OutDir = "-outdir";
static const String ProcessDir = "-processdir";
static const String ProcessFile = "-processfile";
static const String ProcessFileList = "-processfilelist";
static const String QualityConfig = "-qualitycfgpath";
static const String GPU = "-gpu";
static const String Quality = "-quality";
static const String SaveNormals = "-saveNormals";
}

SceneExporterTool::SceneExporterTool()
    : CommandLineTool("-sceneexporter")
{
    options.AddOption(OptionName::Scene, VariantType(false), "Target object is scene, so we need to export *.sc2 files");
    options.AddOption(OptionName::Texture, VariantType(false), "Target object is texture, so we need to export *.tex files");

    options.AddOption(OptionName::InDir, VariantType(String("")), "Path for Project/DataSource/3d/ folder");
    options.AddOption(OptionName::OutDir, VariantType(String("")), "Path for Project/Data/3d/ folder");
    options.AddOption(OptionName::ProcessDir, VariantType(String("")), "Foldername from DataSource/3d/ for exporting");
    options.AddOption(OptionName::ProcessFile, VariantType(String("")), "Filename from DataSource/3d/ for exporting");
    options.AddOption(OptionName::ProcessFileList, VariantType(String("")), "Pathname to file with filenames for exporting");
    options.AddOption(OptionName::QualityConfig, VariantType(String("")), "Full path for quality.yaml file");

    options.AddOption(OptionName::GPU, VariantType(String("origin")), "GPU family: PoverVR_iOS, PoverVR_Android, tegra, mali, adreno, origin, dx11");
    options.AddOption(OptionName::Quality, VariantType(static_cast<uint32>(TextureConverter::ECQ_DEFAULT)), "Quality of pvr/etc compression. Default is 4 - the best quality. Available values [0-4]");

    options.AddOption(OptionName::SaveNormals, VariantType(false), "Disable removing of normals from vertexes");
}

void SceneExporterTool::ConvertOptionsToParamsInternal()
{
    inFolder = options.GetOption(OptionName::InDir).AsString();
    outFolder = options.GetOption(OptionName::OutDir).AsString();
    filename = options.GetOption(OptionName::ProcessFile).AsString();
    foldername = options.GetOption(OptionName::ProcessDir).AsString();
    fileListPath = options.GetOption(OptionName::ProcessFileList).AsString();
    qualityConfigPath = options.GetOption(OptionName::QualityConfig).AsString();

    if (options.GetOption(OptionName::Scene).AsBool())
    {
        commandObject = OBJECT_SCENE;
    }
    else if (options.GetOption(OptionName::Texture).AsBool())
    {
        commandObject = OBJECT_TEXTURE;
    }

    const String gpuName = options.GetOption(OptionName::GPU).AsString();
    requestedGPU = GPUFamilyDescriptor::GetGPUByName(gpuName);

    const uint32 qualityValue = options.GetOption(OptionName::Quality).AsUInt32();
    quality = Clamp(static_cast<TextureConverter::eConvertQuality>(qualityValue), TextureConverter::ECQ_FASTEST, TextureConverter::ECQ_VERY_HIGH);

    const bool saveNormals = options.GetOption(OptionName::SaveNormals).AsBool();
    optimizeOnExport = !saveNormals;
}

bool SceneExporterTool::InitializeInternal()
{
    if (inFolder.IsEmpty())
    {
        AddError("Input folder was not selected");
        return false;
    }
    inFolder.MakeDirectoryPathname();

    if (outFolder.IsEmpty())
    {
        AddError("Output folder was not selected");
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
        AddError("Target for exporting was not selected");
        return false;
    }

    if (requestedGPU == GPU_INVALID)
    {
        AddError("Unsupported gpu parameter was selected");
        return false;
    }

    return true;
}

void SceneExporterTool::ProcessInternal()
{
    SceneExporter exporter;

    exporter.SetOutFolder(outFolder);
    exporter.SetInFolder(inFolder);
	exporter.SetGPUForExporting(requestedGPU);
	exporter.EnableOptimizations(optimizeOnExport);
	exporter.SetCompressionQuality(quality);

    if (commandAction == ACTION_EXPORT_FILE)
    {
        ExportFile(exporter);
    }
    else if (commandAction == ACTION_EXPORT_FOLDER)
    {
        ExportFolder(exporter);
    }
    if (commandAction == ACTION_EXPORT_FILELIST)
    {
        ExportFileList(exporter);
    }
}

void SceneExporterTool::ExportFolder(SceneExporter& exporter)
{
    if (OBJECT_SCENE == commandObject)
    {
        exporter.ExportSceneFolder(foldername, errors);
    }
    else if (OBJECT_TEXTURE == commandObject)
    {
        exporter.ExportTextureFolder(foldername, errors);
    }
}

void SceneExporterTool::ExportFile(SceneExporter& exporter)
{
    if (OBJECT_SCENE == commandObject)
    {
        exporter.ExportSceneFile(filename, errors);
    }
    else if (OBJECT_TEXTURE == commandObject)
    {
        exporter.ExportTextureFile(filename, errors);
    }
}

void SceneExporterTool::ExportFileList(SceneExporter& exporter)
{
    ScopedPtr<File> fileWithLinks(File::Create(fileListPath, File::OPEN | File::READ));
    if (!fileWithLinks)
    {
        AddError(Format("[SceneExporterTool] Can't open file %s", fileListPath.GetAbsolutePathname().c_str()));
        return;
    }

    bool isEof = false;

    do
    {
        String link = fileWithLinks->ReadLine();
        if (link.empty())
        {
            break;
        }

        const FilePath exportedPathname = inFolder + link;
        if (exportedPathname.IsDirectoryPathname())
        {
            commandObject = OBJECT_SCENE;
            foldername = link;
            ExportFolder(exporter);
        }
        else if (exportedPathname.IsEqualToExtension(".sc2"))
        {
            commandObject = OBJECT_SCENE;
            filename = link;
            ExportFile(exporter);
        }
        else if (exportedPathname.IsEqualToExtension(".tex"))
        {
            commandObject = OBJECT_TEXTURE;
            filename = link;
            ExportFile(exporter);
        }

        isEof = fileWithLinks->IsEof();
    } while (!isEof);
}


FilePath SceneExporterTool::GetQualityConfigPath() const
{
    if (qualityConfigPath.IsEmpty())
    {
        return CreateQualityConfigPath(inFolder);
    }

    return qualityConfigPath;
}
