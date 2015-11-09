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

#include "Deprecated/SceneValidator.h"

#include "Scene/SceneHelper.h"
#include "Commands2/ConvertToShadowCommand.h"

using namespace DAVA;

DAEConvertAction::DAEConvertAction(const DAVA::FilePath &path)
	: CommandAction(CMDID_DAE_CONVERT, "DAE to SC2 Convert")
	, daePath(path)
{ }

void DAEConvertAction::Redo()
{
    if (FileSystem::Instance()->Exists(daePath) && daePath.IsEqualToExtension(".dae"))
    {
		eColladaErrorCodes code = ConvertDaeToSce(daePath);
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
    FixLODFarDistance(scene);
    
    
    FilePath sc2Path = FilePath::CreateWithNewExtension(daePath, ".sc2");
    scene->SaveScene(sc2Path);
    scene->Release();
}

void DAEConvertAction::FixLODFarDistance(Scene *scene) const
{
    Vector<Entity*> lodEntities;
    scene->GetChildEntitiesWithComponent(lodEntities, Component::LOD_COMPONENT);

    const uint32 size = (uint32)lodEntities.size();
    for(uint32 k = 0; k < size; ++k)
    {
        int32 maxLodIndex = FindMaxLodLevel(lodEntities[k]);
        if ((maxLodIndex < (LodComponent::MAX_LOD_LAYERS - 1)) && (maxLodIndex != LodComponent::INVALID_LOD_LAYER))
        {
            LodComponent * lodComponent = GetLodComponent(lodEntities[k]);
            lodComponent->SetLodLayerDistance(maxLodIndex + 1, LodComponent::MAX_LOD_DISTANCE);
        }
    }
}

DAVA::int32 DAEConvertAction::FindMaxLodLevel(DAVA::Entity *entity) const
{
    Vector<Entity*> roEntities;
    entity->GetChildEntitiesWithComponent(roEntities, Component::RENDER_COMPONENT);
    if(GetRenderObject(entity))
    {
        roEntities.push_back(entity);
    }

    int32 maxLod = LodComponent::INVALID_LOD_LAYER;
    
    const uint32 size = (uint32)roEntities.size();
    for(uint32 k = 0; k < size; ++k)
    {
        RenderObject *ro = GetRenderObject(roEntities[k]);
        if(ro)
        {
            maxLod = Max(maxLod, ro->GetMaxLodIndex());
        }
    }

    return maxLod;
}



DAVA::Scene * DAEConvertAction::CreateSceneFromSce() const
{
    FilePath scePath = FilePath::CreateWithNewExtension(daePath, ".sce");
    
    Scene *scene = new Scene();
    if(SceneFileV2::ERROR_NO_ERROR == scene->LoadScene(scePath))
	{
		ScopedPtr<SceneFileV2> sceneFile(new SceneFileV2());
		sceneFile->OptimizeScene(scene);
		Logger::Info("Scene Optimization finished");
	}

    return scene;
}

