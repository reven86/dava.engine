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



#include "SceneExporter.h"
#include "Deprecated/SceneValidator.h"

#include "TextureCompression/TextureConverter.h"

#include "Render/TextureDescriptor.h"
#include "Qt/Scene/SceneHelper.h"
#include "Render/GPUFamilyDescriptor.h"

#include "../StringConstants.h"

#include "../Qt/Main/QtUtils.h"

using namespace DAVA;

SceneExporter::SceneExporter()
{
    exportForGPU = GPU_UNKNOWN;
	optimizeOnExport = true;
}

SceneExporter::~SceneExporter()
{
}


void SceneExporter::SetInFolder(const FilePath &folderPathname)
{
    sceneUtils.SetInFolder(folderPathname);
}

void SceneExporter::SetOutFolder(const FilePath &folderPathname)
{
    sceneUtils.SetOutFolder(folderPathname);
}

void SceneExporter::SetGPUForExporting(const String &newGPU)
{
    SetGPUForExporting(GPUFamilyDescriptor::GetGPUByName(newGPU));
}

void SceneExporter::SetGPUForExporting(const eGPUFamily newGPU)
{
    exportForGPU = newGPU;
}


void SceneExporter::ExportFile(const String &fileName, Set<String> &errorLog)
{
    Logger::FrameworkDebug("[SceneExporter::ExportFile] %s", fileName.c_str());
    
    FilePath filePath = sceneUtils.dataSourceFolder + fileName;
    
    //Load scene with *.sc2
    Scene *scene = new Scene();
    Entity *rootNode = scene->GetRootNode(filePath);
    if(rootNode)
    {
        int32 count = rootNode->GetChildrenCount();
		Vector<Entity*> tempV;
		tempV.reserve((count));
        for(int32 i = 0; i < count; ++i)
        {
			tempV.push_back(rootNode->GetChild(i));
        }
		for(int32 i = 0; i < count; ++i)
		{
			scene->AddNode(tempV[i]);
		}
		
		ExportScene(scene, filePath, errorLog);
    }
	else
	{
		errorLog.insert(Format("[SceneExporter::ExportFile] Can't open file %s", filePath.GetAbsolutePathname().c_str()));
	}

    SafeRelease(scene);
}

void SceneExporter::ExportScene(Scene *scene, const FilePath &fileName, Set<String> &errorLog)
{
    //Create destination folder
    String relativeFilename = fileName.GetRelativePathname(sceneUtils.dataSourceFolder);
    sceneUtils.workingFolder = fileName.GetDirectory().GetRelativePathname(sceneUtils.dataSourceFolder);
    
    FileSystem::Instance()->CreateDirectory(sceneUtils.dataFolder + sceneUtils.workingFolder, true); 
    
    scene->Update(0.1f);
    //Export scene data
    RemoveEditorNodes(scene);
    
    if(optimizeOnExport)
    {
        RemoveEditorCustomProperties(scene);
    }

    ExportDescriptors(scene, errorLog);

    FilePath oldPath = SceneValidator::Instance()->SetPathForChecking(sceneUtils.dataSourceFolder);
    SceneValidator::Instance()->ValidateScene(scene, fileName, errorLog);
	//SceneValidator::Instance()->ValidateScales(scene, errorLog);

    ExportLandscape(scene, errorLog);

    //save scene to new place
    FilePath tempSceneName = FilePath::CreateWithNewExtension(sceneUtils.dataSourceFolder + relativeFilename, ".exported.sc2");
    scene->Save(tempSceneName, optimizeOnExport);

    bool moved = FileSystem::Instance()->MoveFile(tempSceneName, sceneUtils.dataFolder + relativeFilename, true);
	if(!moved)
	{
		errorLog.insert(Format("Can't move file %s", fileName.GetAbsolutePathname().c_str()));
	}
    
    SceneValidator::Instance()->SetPathForChecking(oldPath);
}

void SceneExporter::RemoveEditorNodes(DAVA::Entity *rootNode)
{
    //Remove scene nodes
    Vector<Entity *> scenenodes;
    rootNode->GetChildNodes(scenenodes);
        
    //remove nodes from hierarhy
    Vector<Entity *>::reverse_iterator endItDeletion = scenenodes.rend();
    for (Vector<Entity *>::reverse_iterator it = scenenodes.rbegin(); it != endItDeletion; ++it)
    {
        Entity * node = *it;
		String::size_type pos = node->GetName().find(ResourceEditor::EDITOR_BASE);
        if(String::npos != pos)
        {
            node->GetParent()->RemoveNode(node);
        }
    }
}

void SceneExporter::RemoveEditorCustomProperties(Entity *rootNode)
{
    Vector<Entity *> scenenodes;
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
    
    Vector<Entity *>::const_iterator endIt = scenenodes.end();
    for (Vector<Entity *>::iterator it = scenenodes.begin(); it != endIt; ++it)
    {
        Entity * node = *it;

        if(node->GetComponent(Component::CUSTOM_PROPERTIES_COMPONENT))
        {
            KeyedArchive *props = node->GetCustomProperties();
            const Map<String, VariantType*> propsMap = props->GetArchieveData();
            
            auto endIt = propsMap.end();
            for(auto it = propsMap.begin(); it != endIt; ++it)
            {
                String key = it->first;
                
                if(key.find(ResourceEditor::EDITOR_BASE) == 0)
                {
                    if((key != ResourceEditor::EDITOR_DO_NOT_REMOVE) && (key != ResourceEditor::EDITOR_DYNAMIC_LIGHT_ENABLE))
                    {
                        props->DeleteKey(key);
                    }
                }
            }
        }
    }
}


void SceneExporter::ExportDescriptors(DAVA::Scene *scene, Set<String> &errorLog)
{
    DAVA::TexturesMap textures;
    SceneHelper::EnumerateSceneTextures(scene, textures);

    auto endIt = textures.end();
    for(auto it = textures.begin(); it != endIt; ++it)
    {
        DAVA::FilePath pathname = it->first;
        if((pathname.GetType() == DAVA::FilePath::PATH_IN_MEMORY) || pathname.IsEmpty())
        {
            continue;
        }
        
        ExportTextureDescriptor(it->first, errorLog);
    }

    textures.clear();
}


bool SceneExporter::ExportTextureDescriptor(const FilePath &pathname, Set<String> &errorLog)
{
    TextureDescriptor *descriptor = TextureDescriptor::CreateFromFile(pathname);
    if(!descriptor)
    {
        errorLog.insert(Format("Can't create descriptor for pathname %s", pathname.GetAbsolutePathname().c_str()));
        return false;
    }
    
    descriptor->exportedAsGpuFamily = exportForGPU;
    descriptor->exportedAsPixelFormat = descriptor->GetPixelFormatForCompression(exportForGPU);

    if((descriptor->exportedAsGpuFamily != GPU_UNKNOWN) && (descriptor->exportedAsPixelFormat == FORMAT_INVALID))
    {
        errorLog.insert(Format("Not selected export format for pathname %s", pathname.GetAbsolutePathname().c_str()));
        
		delete descriptor;
        return false;
    }
    
    
    String workingPathname = descriptor->pathname.GetRelativePathname(sceneUtils.dataSourceFolder);
    sceneUtils.PrepareFolderForCopyFile(workingPathname, errorLog);

    bool isExported = ExportTexture(descriptor, errorLog);
    if(isExported)
    {
        descriptor->Export(sceneUtils.dataFolder + workingPathname);
    }
    
	delete descriptor;
    return isExported;
}

bool SceneExporter::ExportTexture(const TextureDescriptor * descriptor, Set<String> &errorLog)
{
    CompressTextureIfNeed(descriptor, errorLog);
    
    if(descriptor->exportedAsGpuFamily == GPU_UNKNOWN)
    {
		bool copyResult = true;
		
		if(descriptor->IsCubeMap())
		{
			Vector<FilePath> faceNames;
			Texture::GenerateCubeFaceNames(descriptor->pathname.GetAbsolutePathname().c_str(), faceNames);
			for(Vector<FilePath>::iterator it = faceNames.begin();
				it != faceNames.end();
				++it)
			{
				bool result = sceneUtils.CopyFile(*it, errorLog);
				copyResult = copyResult && result;
			}
		}
		else
		{
			FilePath sourceTexturePathname =  FilePath::CreateWithNewExtension(descriptor->pathname, ".png");
			copyResult = sceneUtils.CopyFile(sourceTexturePathname, errorLog);
		}
		
		return copyResult;
    }

    FilePath compressedTexureName = GPUFamilyDescriptor::CreatePathnameForGPU(descriptor, (eGPUFamily)descriptor->exportedAsGpuFamily);
    return sceneUtils.CopyFile(compressedTexureName, errorLog);
}



void SceneExporter::ExportFolder(const String &folderName, Set<String> &errorLog)
{
    FilePath folderPathname = sceneUtils.dataSourceFolder + folderName;
    folderPathname.MakeDirectoryPathname();
    
	FileList * fileList = new FileList(folderPathname);
    for (int32 i = 0; i < fileList->GetCount(); ++i)
	{
        FilePath pathname = fileList->GetPathname(i);
		if(fileList->IsDirectory(i))
		{
            if(!fileList->IsNavigationDirectory(i))
            {
                String workingPathname = pathname.GetRelativePathname(sceneUtils.dataSourceFolder);
                ExportFolder(workingPathname, errorLog);
            }
        }
        else 
        {
            if(pathname.IsEqualToExtension(".sc2"))
            {
                String::size_type exportedPos = pathname.GetAbsolutePathname().find(".exported.sc2");
                if(exportedPos != String::npos)
                {
                    //Skip temporary files, created during export
                    continue;
                }
                
                String workingPathname = pathname.GetRelativePathname(sceneUtils.dataSourceFolder);
                ExportFile(workingPathname, errorLog);
            }
        }
    }
    
    SafeRelease(fileList);
}



void SceneExporter::ExportLandscape(Scene *scene, Set<String> &errorLog)
{
    DVASSERT(scene);

    Landscape *landscape = FindLandscape(scene);
    if (landscape)
    {
        sceneUtils.CopyFile(landscape->GetHeightmapPathname(), errorLog);
    }
}

void SceneExporter::CompressTextureIfNeed(const TextureDescriptor * descriptor, Set<String> &errorLog)
{
    if(descriptor->exportedAsGpuFamily == GPU_UNKNOWN)
        return;
    
    
    FilePath compressedTexureName = GPUFamilyDescriptor::CreatePathnameForGPU(descriptor, (eGPUFamily)descriptor->exportedAsGpuFamily);
    FilePath sourceTexturePathname =  FilePath::CreateWithNewExtension(descriptor->pathname, ".png");

    bool fileExcists = FileSystem::Instance()->IsFile(compressedTexureName);
    bool needToConvert = SceneValidator::IsTextureChanged(descriptor, (eGPUFamily)descriptor->exportedAsGpuFamily);
    
    if(needToConvert || !fileExcists)
    {
        //TODO: convert to pvr/dxt
        //TODO: do we need to convert to pvr if needToConvert is false, but *.pvr file isn't at filesystem
        
		eGPUFamily gpuFamily = (eGPUFamily)descriptor->exportedAsGpuFamily;
		TextureConverter::CleanupOldTextures(descriptor, gpuFamily, (PixelFormat)descriptor->exportedAsPixelFormat);
		TextureConverter::ConvertTexture(*descriptor, gpuFamily, true);
        
        DAVA::TexturesMap texturesMap = Texture::GetTextureMap();
        DAVA::Texture *tex = texturesMap[FILEPATH_MAP_KEY(descriptor->pathname)];
        tex->Reload();
    }
}

void SceneExporter::EnableOptimizations( bool enable )
{
	optimizeOnExport = enable;
}

