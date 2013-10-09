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



#include "DAEConvertAction.h"
#include "Collada/ColladaConvert.h"

#include "Classes/SceneEditor/SceneValidator.h"
#include "Classes/Qt/DockLODEditor/EditorLODData.h"

#include "Classes/Qt/Scene/SceneHelper.h"
#include "Classes/Commands2/ConvertToShadowCommand.h"

using namespace DAVA;

DAEConvertAction::DAEConvertAction(const DAVA::FilePath &path)
	: CommandAction(CMDID_DAE_CONVERT, "DAE to SC2 Convert")
	, daePath(path)
{ }

void DAEConvertAction::Redo()
{
	if(daePath.Exists() && daePath.IsEqualToExtension(".dae"))
	{
		eColladaErrorCodes code = ConvertDaeToSce(daePath.GetAbsolutePathname());
		if(code == COLLADA_OK)
		{
            ConvertFromSceToSc2();
        }
		else if(code == COLLADA_ERROR_OF_ROOT_NODE)
		{
			Logger::Error("Can't convert from DAE. Looks like one of materials has same name as root node.");
		}
		else
		{
			Logger::Error("Can't convert from DAE.");
		}
	}
}

void DAEConvertAction::ConvertFromSceToSc2() const
{
    Scene *scene = CreateSceneFromSce();
    FilePath sc2Path = FilePath::CreateWithNewExtension(daePath, ".sc2");
	SceneHelper::SaveScene(scene, sc2Path);
    scene->Release();
}


DAVA::Scene * DAEConvertAction::CreateSceneFromSce() const
{
    FilePath scePath = FilePath::CreateWithNewExtension(daePath, ".sce");
    
    Scene *scene = new Scene();
    Entity *rootNode = scene->GetRootNode(scePath);
    scene->AddNode(rootNode);
    scene->BakeTransforms();
    
    return scene;
}


DAEConvertWithSettingsAction::DAEConvertWithSettingsAction(const DAVA::FilePath &path)
    : DAEConvertAction(path)
{
    
}

void DAEConvertWithSettingsAction::ConvertFromSceToSc2() const
{
    FilePath sc2Path = FilePath::CreateWithNewExtension(daePath, ".sc2");
    if(sc2Path.Exists())
    {
        Scene *scene = CreateSceneFromSce();
        
        FilePath newSc2Path = sc2Path;
        newSc2Path.ReplaceBasename(sc2Path.GetBasename() + "_new");
        SceneHelper::SaveScene(scene, newSc2Path);
        SafeRelease(scene);
        
        TryToMergeScenes(sc2Path, newSc2Path);
        
        FileSystem::Instance()->DeleteFile(newSc2Path);
    }
    else
    {
        DAEConvertAction::ConvertFromSceToSc2();
    }
}


void DAEConvertWithSettingsAction::TryToMergeScenes(const DAVA::FilePath &originalPath, const DAVA::FilePath &newPath)
{
    Scene * oldScene = CreateSceneFromSc2(originalPath);
    Scene * newScene = CreateSceneFromSc2(newPath);
    
    CopyMaterialsSettings(oldScene, newScene);
    CopyLODSettings(oldScene, newScene);
	CopyShadowSettings(oldScene, newScene);
    
    SceneHelper::SaveScene(newScene, originalPath);
    
    oldScene->Release();
    newScene->Release();
}

DAVA::Scene * DAEConvertWithSettingsAction::CreateSceneFromSc2(const DAVA::FilePath &scenePathname)
{
    Scene * scene = new Scene();
    
    Entity * rootNode = scene->GetRootNode(scenePathname);
	if(rootNode)
	{
		Vector<Entity*> tmpEntities;
		uint32 entitiesCount = (uint32)rootNode->GetChildrenCount();
        
		// optimize scene
        SceneFileV2 *sceneFile = new SceneFileV2();
		sceneFile->OptimizeScene(rootNode);
		sceneFile->Release();
        
		// remember all child pointers, but don't add them to scene in this cycle
		// because when entity is adding it is automatically removing from its old hierarchy
		tmpEntities.reserve(entitiesCount);
		for (uint32 i = 0; i < entitiesCount; ++i)
		{
			tmpEntities.push_back(rootNode->GetChild(i));
		}
        
		// now we can safely add entities into our hierarchy
		for (uint32 i = 0; i < (uint32) tmpEntities.size(); ++i)
		{
			scene->AddNode(tmpEntities[i]);
		}

        Set<String> errorsLog;
        SceneValidator::Instance()->ValidateScene(scene, errorsLog);
	}
    
    return scene;
}


void DAEConvertWithSettingsAction::CopyMaterialsSettings(DAVA::Scene * srcScene, DAVA::Scene * dstScene)
{
    Vector<Material *> srcMaterials = CreateMaterialsVector(srcScene);
    Vector<Material *> dstMaterials = CreateMaterialsVector(dstScene);
    
    if (srcMaterials.size() == dstMaterials.size())
    {
        uint32 count = srcMaterials.size();
        for (uint32 i = 0; i < count; ++i)
        {
            if(srcMaterials[i]->GetName() == dstMaterials[i]->GetName())
            {
				dstMaterials[i]->CopySettings(srcMaterials[i]);
            }
        }
    }
}

DAVA::Vector<DAVA::Material *> DAEConvertWithSettingsAction::CreateMaterialsVector(DAVA::Scene *scene)
{
    Vector<Material *> materials;
    SceneHelper::EnumerateMaterials(scene, materials);
    
    if(materials.size())
    {
        std::sort(materials.begin(), materials.end(), DAEConvertWithSettingsAction::CompareMaterials);
    }
    
    return materials;
}

bool DAEConvertWithSettingsAction::CompareMaterials(const DAVA::Material *left, const DAVA::Material *right)
{
    return (left->GetName() < right->GetName());
}


void DAEConvertWithSettingsAction::CopyLODSettings(DAVA::Scene * srcScene, DAVA::Scene * dstScene)
{
    Vector<LodComponent *> srcLODs;
    EditorLODData::EnumerateLODsRecursive(srcScene, srcLODs);

    Vector<LodComponent *> dstLODs;
    EditorLODData::EnumerateLODsRecursive(dstScene, dstLODs);

    if(srcLODs.size() == dstLODs.size())
    {
        uint32 count = srcLODs.size();
        for (uint32 i = 0; i < count; ++i)
        {
            if(srcLODs[i]->GetEntity()->GetName() == dstLODs[i]->GetEntity()->GetName())
            {
                dstLODs[i]->CopyLODSettings(srcLODs[i]);
            }
        }
    }
}

void DAEConvertWithSettingsAction::CopyShadowSettings( DAVA::Scene * srcScene, DAVA::Scene * dstScene )
{
	uint32 srcCount = srcScene->GetChildrenCount();
	uint32 dstCount = dstScene->GetChildrenCount();
	if(srcCount == dstCount)
	{
		for(uint32 i = 0; i < srcCount; ++i)
		{
			CopyShadowSettingsRecursive(srcScene->GetChild(i), dstScene->GetChild(i));
		}
	}
}


void DAEConvertWithSettingsAction::CopyShadowSettingsRecursive(const DAVA::Entity * srcEntity, DAVA::Entity * dstEntity )
{
	if((srcEntity->GetName() == dstEntity->GetName()) && ConvertToShadowCommand::IsEntityWithShadowVolume(srcEntity))
	{
		DAVA::RenderBatch *oldBatch = ConvertToShadowCommand::ConvertToShadowVolume(dstEntity);
		SafeRelease(oldBatch);
	}

	uint32 srcCount = srcEntity->GetChildrenCount();
	uint32 dstCount = dstEntity->GetChildrenCount();
	if(srcCount == dstCount)
	{
		for(uint32 i = 0; i < srcCount; ++i)
		{
			CopyShadowSettingsRecursive(srcEntity->GetChild(i), dstEntity->GetChild(i));
		}
	}
}
