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


#include "Scene3D/Systems/MaterialSystem.h"
#include "Render/Material/NMaterialTemplate.h"

#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/Landscape.h"

#include "Utils/StringFormat.h"
#include "Scene3D/Scene.h"

namespace DAVA
{
    
MaterialSystem::MaterialSystem(Scene * scene)
    : SceneSystem(scene)
{
    SetDefaultMaterialQuality(NMaterial::DEFAULT_QUALITY_NAME); //TODO: add code setting material quality based on device specs
}

MaterialSystem::~MaterialSystem()
{
}
    

void MaterialSystem::AddEntity(Entity * entity)
{
}

void MaterialSystem::BuildMaterialList(Entity *forEntity, Set<NMaterial*>& materialList, NMaterial::eMaterialType materialType, bool includeRuntime) const
{
    if(!forEntity) return;
    
    List<NMaterial*> materials;
    forEntity->GetDataNodes(materials);
    
    List<NMaterial *>::const_iterator endIt = materials.end();
    for(List<NMaterial *>::const_iterator it = materials.begin(); it != endIt; ++it)
    {
        if( (materialType == NMaterial::MATERIALTYPE_NONE || materialType == (*it)->GetMaterialType()) && // filter by material type
			(includeRuntime || !((*it)->GetNodeGlags() & DataNode::NodeRuntimeFlag)))
        {
            materialList.insert(*it);
        }
    }
}

void MaterialSystem::SetDefaultMaterialQuality(const FastName& qualityLevelName)
{
    defaultMaterialQuality = qualityLevelName;
}

const FastName& MaterialSystem::GetDefaultMaterialQuality() const
{
    return defaultMaterialQuality;
}

const FastName& MaterialSystem::GetCurrentMaterialQuality() const
{
    return currentMaterialQuality;
}

void MaterialSystem::SwitchMaterialQuality(const FastName& qualityLevelName, bool force)
{
    Set<NMaterial*> materials;
    BuildMaterialList(GetScene(), materials, NMaterial::MATERIALTYPE_MATERIAL);
    
    Set<NMaterial *>::const_iterator endIt = materials.end();
    for(Set<NMaterial *>::const_iterator it = materials.begin(); it != endIt; ++it)
    {
		NMaterial* material = *it;
		
		material->SetQuality(qualityLevelName);
        material->ReloadQuality(force);
    }
}
    
};

