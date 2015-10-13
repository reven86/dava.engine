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


#include "SceneExporterTool.h"
#include "SceneExporter.h"

#include "CommandLine/CommandLineParser.h"

using namespace DAVA;


void SceneExporterTool::PrintUsage() const
{
    printf("\n");
    printf("-sceneexporter [-scene|-texture] [-indir [directory]] [-outdir [directory]] [-processdir [directory]] [-processfile [directory]] [-format]\n");
    printf("\twill export scene file from DataSource/3d to Data/3d\n");
    printf("\t-scene - target object is scene, so we need to export *.sc2 files\n");
    printf("\t-texture - target object is texture, so we need to export *.tex files\n");
    printf("\t-indir - path for Poject/DataSource/3d/ folder \n");
    printf("\t-outdir - path for Poject/Data/3d/ folder\n");
    printf("\t-processdir - foldername from DataSource/3d/ for exporting\n");
    printf("\t-processfile - filename from DataSource/3d/ for exporting\n");
    printf("\t-gpu - PoverVR_iOS, PoverVR_Android, tegra, mali, adreno, origin\n");
	printf("\t-saveNormals - disable removing of normals from vertexes\n");
	printf("\t-quality [0-4] - quality of pvr/etc compression. default is 4 - the best quality\n");
    printf("\t-qualitycfgpath - path for quality.yaml file\n");

    printf("\n");
    printf("Samples:\n");
    printf("-sceneexporter -scene -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processdir Maps/objects/ -quality 3\n");
    printf("-sceneexporter -scene -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processfile Maps/level.sc2\n");
    printf("-sceneexporter -texture -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processdir Maps/objects/images/ -quality 3\n");
    printf("-sceneexporter -texture -indir /Users/User/Project/DataSource/3d -outdir /Users/User/Project/Data/3d/ -processfile Maps/objects/images/stone.tex\n");
}

DAVA::String SceneExporterTool::GetCommandLineKey() const
{
    return "-sceneexporter";
}

bool SceneExporterTool::InitializeFromCommandLine()
{
    commandAction = ACTION_NONE;
    commandObject = OBJECT_SCENE;
    
    inFolder = CommandLineParser::GetCommandParam(String("-indir"));
    outFolder = CommandLineParser::GetCommandParam(String("-outdir"));
    if(inFolder.IsEmpty() && outFolder.IsEmpty())
    {
        errors.insert(Format("[SceneExporterTool] Incorrect indir (%s) or outdir (%s) parameter",inFolder.GetAbsolutePathname().c_str(), outFolder.GetAbsolutePathname().c_str()));
        return false;
    }
    
    inFolder.MakeDirectoryPathname();
    outFolder.MakeDirectoryPathname();

    
	String qualityName = CommandLineParser::GetCommandParam(String("-quality"));
	if(qualityName.empty())
	{
		quality = DAVA::TextureConverter::ECQ_DEFAULT;
	}
	else
	{
		quality = Clamp((DAVA::TextureConverter::eConvertQuality)atoi(qualityName.c_str()), DAVA::TextureConverter::ECQ_FASTEST, DAVA::TextureConverter::ECQ_VERY_HIGH);
	}
	
    filename = CommandLineParser::GetCommandParam(String("-processfile"));
    foldername = CommandLineParser::GetCommandParam(String("-processdir"));
    qualityConfigPath = CommandLineParser::GetCommandParam(String("-qualitycfgpath"));

    if(!filename.empty())
    {
        commandAction = ACTION_EXPORT_FILE;
    }
    else if(!foldername.empty())
    {
        commandAction = ACTION_EXPORT_FOLDER;
    }
    else
    {
        errors.insert("[SceneExporterTool] File or folder for export is not set");
        return false;
    }
    
    if(CommandLineParser::CommandIsFound("-scene"))
    {
        commandObject = OBJECT_SCENE;
    }
    else if(CommandLineParser::CommandIsFound("-texture"))
    {
        commandObject = OBJECT_TEXTURE;
    }
    
	String gpu = CommandLineParser::GetCommandParam(String("-gpu"));
	requestedGPU = GPUFamilyDescriptor::GetGPUByName(gpu);
	if (GPU_INVALID == requestedGPU)
	{
		errors.insert(Format("[SceneExporterTool] wrong gpu parameter (%s)", gpu.c_str()));
		return false;
	}

	optimizeOnExport = (CommandLineParser::CommandIsFound(String("-saveNormals")) == false);

    return true;
}

void SceneExporterTool::DumpParams() const
{
    Logger::Info("Export started with params:\n\tIn folder: %s\n\tOut folder: %s\n\tQuality: %d\n\tGPU: %d\n\tFilename: %s\n\tFoldername: %s", inFolder.GetStringValue().c_str(), outFolder.GetStringValue().c_str(), quality, requestedGPU, filename.c_str(), foldername.c_str());
}

void SceneExporterTool::Process() 
{
    SceneExporter exporter;

    exporter.SetOutFolder(outFolder);
    exporter.SetInFolder(inFolder);
	exporter.SetGPUForExporting(requestedGPU);
	exporter.EnableOptimizations(optimizeOnExport);
	exporter.SetCompressionQuality(quality);
    
    if(ACTION_EXPORT_FILE == commandAction)
    {
        if(OBJECT_SCENE == commandObject)
        {
            exporter.ExportSceneFile(filename, errors);
        }
        else if(OBJECT_TEXTURE == commandObject)
        {
            exporter.ExportTextureFile(filename, errors);
        }
    }
    else if(commandAction == ACTION_EXPORT_FOLDER)
    {
        if(OBJECT_SCENE == commandObject)
        {
            exporter.ExportSceneFolder(foldername, errors);
        }
        else if(OBJECT_TEXTURE == commandObject)
        {
            exporter.ExportTextureFolder(foldername, errors);
        }
    }
}


FilePath SceneExporterTool::GetQualityConfigPath() const
{
    return qualityConfigPath;
}

