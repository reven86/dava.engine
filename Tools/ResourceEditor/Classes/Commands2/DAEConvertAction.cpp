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
			FileSystem::Instance()->DeleteFile(FilePath::CreateWithNewExtension(daePath, ".sce"));
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
    scene->Save(sc2Path);
    scene->Release();
}


DAVA::Scene * DAEConvertAction::CreateSceneFromSce() const
{
    FilePath scePath = FilePath::CreateWithNewExtension(daePath, ".sce");
    
    Scene *scene = new Scene();
    Entity *rootNode = scene->GetRootNode(scePath);
	if(rootNode)
	{
		rootNode = rootNode->Clone();
		scene->AddNode(rootNode);
		scene->BakeTransforms();
		rootNode->Release();
	}
    
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
        scene->Save(newSc2Path);
        scene->Release();
        
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
    
	CopyGeometryRecursive(newScene, oldScene);
	oldScene->Save(originalPath);

    oldScene->Release();
    newScene->Release();
}

DAVA::Scene * DAEConvertWithSettingsAction::CreateSceneFromSc2(const DAVA::FilePath &scenePathname)
{
    Scene * scene = new Scene();
    
    Entity * rootNode = scene->GetRootNode(scenePathname);
	if(rootNode)
	{
		rootNode = rootNode->Clone();

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

		rootNode->Release();

        Set<String> errorsLog;
        SceneValidator::Instance()->ValidateScene(scene, scenePathname, errorsLog);
	}
    
    return scene;
}


void DAEConvertWithSettingsAction::CopyGeometryRecursive( DAVA::Entity *srcEntity, DAVA::Entity *dstEntity )
{
	CopyGeometry(srcEntity, dstEntity);

	uint32 srcCount = srcEntity->GetChildrenCount();
	uint32 dstCount = dstEntity->GetChildrenCount();
	for(uint32 iSrc = 0, iDst = 0; iSrc < srcCount && iDst < dstCount; )
	{
		Entity * src = srcEntity->GetChild(iSrc);
		Entity * dst = dstEntity->GetChild(iDst);

		if(src->GetName() == dst->GetName())
		{
			CopyGeometryRecursive(src, dst);
			++iSrc;
			++iDst;
		}
		else
		{
			++iDst;
		}
	}
}

void DAEConvertWithSettingsAction::CopyGeometry(DAVA::Entity *srcEntity, DAVA::Entity *dstEntity)
{
	if(ConvertToShadowCommand::IsEntityWithShadowVolume(dstEntity))
	{
		DAVA::RenderBatch *oldBatch = ConvertToShadowCommand::ConvertToShadowVolume(srcEntity);
		SafeRelease(oldBatch);
	}

	RenderObject *srcRo = GetRenderObject(srcEntity);
	RenderObject *dstRo = GetRenderObject(dstEntity);
	if(srcRo && dstRo)
	{
		bool ret = CopyRenderObjects(srcRo, dstRo);
        if(ret)
        {
            dstEntity->SetLocalTransform(srcEntity->GetLocalTransform());
        }
		else
		{
			Logger::Error("Can't copy geometry from %s to %s", srcEntity->GetName().c_str(), dstEntity->GetName().c_str());
		}
	}
}


bool DAEConvertWithSettingsAction::CopyRenderObjects( DAVA::RenderObject *srcRo, DAVA::RenderObject *dstRo )
{
	uint32 srcBatchCount = srcRo->GetRenderBatchCount();
	uint32 dstBatchCount = dstRo->GetRenderBatchCount();

	if(srcBatchCount != dstBatchCount)
	{
		return false;
	}

	for(uint32 i = 0; i < srcBatchCount; ++i)
	{
		RenderBatch *srcBatch = srcRo->GetRenderBatch(i);
		RenderBatch *dstBatch = dstRo->GetRenderBatch(i);

		dstBatch->SetPolygonGroup(srcBatch->GetPolygonGroup());
	}

	return true;
}

