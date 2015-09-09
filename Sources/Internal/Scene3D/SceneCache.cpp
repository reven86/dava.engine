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


#include "SceneCache.h"
#include "Scene3D/Systems/MaterialSystem.h"
#include "Scene3D/Systems/ParticleEffectSystem.h"

namespace DAVA
{
    
void SceneCache::InsertScene(DAVA::Scene *scene)
{
    Set<Scene*>::iterator it = sceneSet.find(scene);
    if(it == sceneSet.end())
    {
        sceneSet.insert(scene);
    }
    else
    {
        Logger::Error("[SceneCache::InsertScene] Trying to add scene already in cache");
    }
}
    
void SceneCache::RemoveScene(DAVA::Scene *scene)
{
    Set<Scene*>::iterator it = sceneSet.find(scene);
    if(it != sceneSet.end())
    {
        sceneSet.erase(it);
    }
    else
    {
        Logger::Error("[SceneCache::RemoveScene] Trying to remove scene not in cache");
    }
}
    
void SceneCache::InvalidateSceneMaterials()
{
    Set<Scene*>::iterator it = sceneSet.begin();
    for(; it != sceneSet.end(); ++it)
    {
        Scene *scene = *it;
        Set<NMaterial *> materialList;
        MaterialSystem *matSystem = scene->GetMaterialSystem();
        matSystem->BuildMaterialList(scene, materialList, NMaterial::MATERIALTYPE_NONE, true);

        ParticleEffectSystem* partEffectSys = scene->particleEffectSystem;
        if (partEffectSys != nullptr)
        {
            const Map<uint32, NMaterial*>& particleInstances = partEffectSys->GetMaterialInstances();
            for (const auto& particle : particleInstances)
            {
                materialList.insert(particle.second);
                if (particle.second->GetParent())
                {
                    materialList.insert(particle.second->GetParent());
                }
            }
        }

        if(scene->GetGlobalMaterial())
        {
            materialList.insert(scene->GetGlobalMaterial());
        }
        
        for (NMaterial * material : materialList)
        {
            material->BuildActiveUniformsCacheParamsCache();
            material->BuildTextureParamsCache();
        }
    }
}
    
}
