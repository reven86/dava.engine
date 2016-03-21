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


#ifndef __SCENE_STRUCTURE_SYSTEM_H__
#define __SCENE_STRUCTURE_SYSTEM_H__

#include "Commands2/Base/Command2.h"
#include "Scene/SelectableGroup.h"
#include "StringConstants.h"
#include "SystemDelegates.h"

// framework
#include "Entity/SceneSystem.h"
#include "Scene3D/Entity.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "UI/UIEvent.h"
#include "Render/Highlevel/Landscape.h"
#include "Functional/Function.h"

class StructureSystem : public DAVA::SceneSystem
{
public:
    using InternalMapping = DAVA::Map<DAVA::Entity*, DAVA::Entity*>;

public:
    StructureSystem(DAVA::Scene* scene);
    ~StructureSystem();

    void Move(const SelectableGroup& objects, DAVA::Entity* newParent, DAVA::Entity* newBefore);
    void MoveEmitter(const DAVA::Vector<DAVA::ParticleEmitterInstance*>& emitters, const DAVA::Vector<DAVA::ParticleEffectComponent*>& oldEffects, DAVA::ParticleEffectComponent* newEffect, int dropAfter);
    void MoveLayer(const DAVA::Vector<DAVA::ParticleLayer*>& layers, const DAVA::Vector<DAVA::ParticleEmitterInstance*>& oldEmitters, DAVA::ParticleEmitterInstance* newEmitter, DAVA::ParticleLayer* newBefore);
    void MoveForce(const DAVA::Vector<DAVA::ParticleForce*>& forces, const DAVA::Vector<DAVA::ParticleLayer*>& oldLayers, DAVA::ParticleLayer* newLayer);

    void Remove(const SelectableGroup& objects);

    SelectableGroup ReloadEntities(const SelectableGroup& objects, bool saveLightmapSettings = false);

    // Mapping is link between old entity and new entity
    void ReloadRefs(const DAVA::FilePath& modelPath, InternalMapping& mapping, bool saveLightmapSettings = false);
    SelectableGroup ReloadEntitiesAs(const SelectableGroup& objects, const DAVA::FilePath& newModelPath, bool saveLightmapSettings = false);
    void Add(const DAVA::FilePath& newModelPath, const DAVA::Vector3 pos = DAVA::Vector3());

    void EmitChanged();

    DAVA::Entity* Load(const DAVA::FilePath& sc2path);

    void AddDelegate(StructureSystemDelegate* delegate);
    void RemoveDelegate(StructureSystemDelegate* delegate);

private:
    friend class SceneEditor2;

    void Process(DAVA::float32 timeElapsed) override;

    void ProcessCommand(const Command2* command, bool redo);

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void ReloadInternal(InternalMapping& mapping, const DAVA::FilePath& newModelPath, bool saveLightmapSettings);
    DAVA::Entity* LoadInternal(const DAVA::FilePath& sc2path, bool clearCached);

    bool CopyLightmapSettings(DAVA::Entity* fromState, DAVA::Entity* toState) const;
    void CopyLightmapSettings(DAVA::NMaterial* fromEntity, DAVA::NMaterial* toEntity) const;
    void FindMeshesRecursive(DAVA::Entity* entity, DAVA::Vector<DAVA::RenderObject*>& objects) const;

    void CheckAndMarkSolid(DAVA::Entity* entity);

    void SearchEntityByRef(DAVA::Entity* parent, const DAVA::FilePath& refToOwner, const DAVA::Function<void(DAVA::Entity*)>& callback);

    void RemoveEntities(DAVA::Vector<DAVA::Entity*>& entitiesToRemove);

private:
    DAVA::List<StructureSystemDelegate*> delegates;
    bool structureChanged = false;
};

#endif // __SCENE_STRUCTURE_SYSTEM_H__
