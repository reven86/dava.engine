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

#include "Scene3D/Systems/FoliageSystem.h"

#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/VegetationRenderObject.h"
#include "Scene3D/Components/ComponentHelpers.h"

namespace DAVA
{
FoliageSystem::FoliageSystem(Scene* scene) : SceneSystem(scene),
        landscapeEntity(NULL),
        foliageEntity(NULL)
{
        
}
    
FoliageSystem::~FoliageSystem()
{
    SafeRelease(landscapeEntity);
    SafeRelease(foliageEntity);
}
    
void FoliageSystem::AddEntity(Entity * entity)
{
    Landscape* landscapeRO = GetLandscape(entity);
    if(landscapeRO &&
        entity != landscapeEntity)
    {
        SafeRelease(landscapeEntity);
        landscapeEntity = SafeRetain(entity);
            landscapeRO->SetFoliageSystem(this);
        
        SyncFoliageWithLandscape();
    }
    
    VegetationRenderObject* vegetationRO = GetVegetation(entity);
    if(vegetationRO &&
       entity != foliageEntity)
    {
        SafeRelease(foliageEntity);
        foliageEntity = SafeRetain(entity);
        
        SyncFoliageWithLandscape();
    }
}

void FoliageSystem::RemoveEntity(Entity * entity)
{
    if(entity == foliageEntity)
    {
        SafeRelease(foliageEntity);
    }
    
    if(entity == landscapeEntity)
    {
        SafeRelease(landscapeEntity);
    }
}

void FoliageSystem::SyncFoliageWithLandscape()
{
    if(landscapeEntity && foliageEntity)
    {
        Landscape* landscapeRO = GetLandscape(landscapeEntity);
        VegetationRenderObject* vegetationRO = GetVegetation(foliageEntity);
        
        vegetationRO->SetHeightmap(landscapeRO->GetHeightmap());
        vegetationRO->SetHeightmapPath(landscapeRO->GetHeightmapPathname());
        vegetationRO->SetWorldSize(Vector3(landscapeRO->GetLandscapeSize(),
                                           landscapeRO->GetLandscapeSize(),
                                           landscapeRO->GetLandscapeHeight()));
    }
}

void FoliageSystem::SetPerturbation(const Vector3& point,
                                    const Vector3& force,
                                    float32 distance)
{
    VegetationRenderObject* vegetationRO = GetVegetation(foliageEntity);
    if(vegetationRO != NULL)
    {
        vegetationRO->SetPerturbation(point, force, distance);
    }
}

void FoliageSystem::SetFoliageVisible(bool show)
{
    VegetationRenderObject* vegetationRO = GetVegetation(foliageEntity);
    if(NULL != vegetationRO)
    {
        vegetationRO->SetVegetationVisible(show);
    }
}

bool FoliageSystem::IsFoliageVisible() const
{
    VegetationRenderObject* vegetationRO = GetVegetation(foliageEntity);
    
    return (NULL != vegetationRO) ? vegetationRO->GetVegetationVisible() : false;;
}

};