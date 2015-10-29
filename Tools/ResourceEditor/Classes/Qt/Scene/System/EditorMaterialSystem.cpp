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


#include "EditorMaterialSystem.h"
#include "Settings/SettingsManager.h"
#include "Project/ProjectManager.h"
#include "Scene3D/Scene.h"
#include "Commands2/Command2.h"
#include "Commands2/CommandBatch.h"
#include "Commands2/DeleteRenderBatchCommand.h"
#include "Commands2/ConvertToShadowCommand.h"
#include "Commands2/DeleteLODCommand.h"
#include "Commands2/CreatePlaneLODCommand.h"
#include "Commands2/CloneLastBatchCommand.h"
#include "Commands2/CopyLastLODCommand.h"

EditorMaterialSystem::EditorMaterialSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
{
}

EditorMaterialSystem::~EditorMaterialSystem()
{
    while (!materialToObjectsMap.empty())
    {
        RemoveMaterial(materialToObjectsMap.begin()->first);
    }

    for (auto op : ownedParents)
    {
        DAVA::SafeRelease(op);
    }
}

DAVA::Entity* EditorMaterialSystem::GetEntity(DAVA::NMaterial* material) const
{
    auto it = materialToObjectsMap.find(material);
    return (it == materialToObjectsMap.end()) ? nullptr : it->second.entity;
}

const DAVA::RenderBatch* EditorMaterialSystem::GetRenderBatch(DAVA::NMaterial* material) const
{
    auto it = materialToObjectsMap.find(material);
    if (it == materialToObjectsMap.end())
        return nullptr;

    const auto& mapping = it->second;
    if (mapping.mode == MaterialMapping::Mode::RetainedRenderBatch)
    {
        DVASSERT(nullptr != mapping.renderBatch);
        return mapping.renderBatch;
    }
    else if (mapping.mode == MaterialMapping::Mode::RenderBatchIndexInRenderObject)
    {
        auto renderObject = GetRenderObject(mapping.entity);
        if ((nullptr == renderObject) || (mapping.renderBatchIndexInRenderObject >= renderObject->GetRenderBatchCount()))
            return nullptr;

        return renderObject->GetRenderBatch(mapping.renderBatchIndexInRenderObject);
    }

    return nullptr;
}

const DAVA::Set<DAVA::NMaterial*>& EditorMaterialSystem::GetTopParents() const
{
    return ownedParents;
}

int EditorMaterialSystem::GetLightViewMode()
{
    return curViewMode;
}

bool EditorMaterialSystem::GetLightViewMode(EditorMaterialSystem::MaterialLightViewMode viewMode) const
{
    return (bool)(curViewMode & viewMode);
}

void EditorMaterialSystem::SetLightViewMode(int fullViewMode)
{
    if (curViewMode != fullViewMode)
    {
        curViewMode = fullViewMode;
        ApplyViewMode();
    }
}

void EditorMaterialSystem::SetLightViewMode(EditorMaterialSystem::MaterialLightViewMode viewMode, bool set)
{
    int newMode = curViewMode;

    if (set)
    {
        newMode |= viewMode;
    }
    else
    {
        newMode &= ~viewMode;
    }

    SetLightViewMode(newMode);
}

void EditorMaterialSystem::SetLightmapCanvasVisible(bool enable)
{
    if (enable != showLightmapCanvas)
    {
        showLightmapCanvas = enable;
        ApplyViewMode();
    }
}

bool EditorMaterialSystem::IsLightmapCanvasVisible() const
{
    return showLightmapCanvas;
}

void EditorMaterialSystem::AddEntity(DAVA::Entity* entity)
{
    DAVA::RenderObject* ro = GetRenderObject(entity);
    if (nullptr != ro)
    {
        AddMaterialsFromEntity(entity);
    }
}

void EditorMaterialSystem::AddMaterialsFromEntity(DAVA::Entity* entity)
{
    DAVA::RenderObject* ro = GetRenderObject(entity);
    DVASSERT(nullptr != ro);

    for (DAVA::uint32 i = 0; i < ro->GetRenderBatchCount(); ++i)
    {
        MaterialMapping mapping;
        mapping.entity = entity;
        mapping.renderBatchIndexInRenderObject = i;
        mapping.mode = MaterialMapping::Mode::RenderBatchIndexInRenderObject;
        AddMaterial(ro->GetRenderBatch(i)->GetMaterial(), mapping);
    }
}

void EditorMaterialSystem::AddMaterialFromRenderBatchWithEntity(DAVA::RenderBatch* renderBatch, DAVA::Entity* entity)
{
    MaterialMapping mapping;
    mapping.entity = entity;
    mapping.renderBatch = renderBatch;
    mapping.mode = MaterialMapping::Mode::RetainedRenderBatch;
    AddMaterial(renderBatch->GetMaterial(), mapping);
    DAVA::SafeRetain(renderBatch);
}

void EditorMaterialSystem::RemoveEntity(DAVA::Entity* entity)
{
    DAVA::RenderObject* ro = GetRenderObject(entity);
    if (nullptr != ro)
    {
        for (DAVA::uint32 i = 0; i < ro->GetRenderBatchCount(); ++i)
        {
            DAVA::RenderBatch* rb = ro->GetRenderBatch(i);
            DAVA::NMaterial* material = rb->GetMaterial();
            RemoveMaterial(material);
        }
    }
}

void EditorMaterialSystem::ApplyViewMode()
{
    DAVA::Set<DAVA::NMaterial*>::const_iterator i = ownedParents.begin();
    DAVA::Set<DAVA::NMaterial*>::const_iterator end = ownedParents.end();

    for (; i != end; ++i)
    {
        ApplyViewMode(*i);
    }
}

void EditorMaterialSystem::ApplyViewMode(DAVA::NMaterial* material)
{
    auto SetMaterialFlag = [](DAVA::NMaterial* material, const DAVA::FastName& flagName, DAVA::int32 value) {
        if (value == 0)
        {
            if (material->HasLocalFlag(flagName))
            {
                material->RemoveFlag(flagName);
            }
        }
        else
        {
            if (material->HasLocalFlag(flagName))
            {
                material->SetFlag(flagName, value);
            }
            else
            {
                material->AddFlag(flagName, value);
            }
        }
    };

    if (curViewMode & LIGHTVIEW_ALBEDO)
    {
        SetMaterialFlag(material, DAVA::NMaterialFlagName::FLAG_LIGHTMAPONLY, 0);
    }
    else
    {
        SetMaterialFlag(material, DAVA::NMaterialFlagName::FLAG_LIGHTMAPONLY, 1);
    }

    if (showLightmapCanvas)
    {
        SetMaterialFlag(material, DAVA::NMaterialFlagName::FLAG_SETUPLIGHTMAP, 1);
    }
    else
    {
        SetMaterialFlag(material, DAVA::NMaterialFlagName::FLAG_SETUPLIGHTMAP, 0);
    }

    //materials
    auto UpdateFlags = [this, SetMaterialFlag](DAVA::NMaterial* material, DAVA::int32 mode, const DAVA::FastName& flagName) {
        if (curViewMode & mode)
        {
            SetMaterialFlag(material, flagName, 1);
        }
        else
        {
            SetMaterialFlag(material, flagName, 0);
        }
    };

    UpdateFlags(material, LIGHTVIEW_ALBEDO, DAVA::NMaterialFlagName::FLAG_VIEWALBEDO);
    UpdateFlags(material, LIGHTVIEW_DIFFUSE, DAVA::NMaterialFlagName::FLAG_VIEWDIFFUSE);
    UpdateFlags(material, LIGHTVIEW_SPECULAR, DAVA::NMaterialFlagName::FLAG_VIEWSPECULAR);
    UpdateFlags(material, LIGHTVIEW_AMBIENT, DAVA::NMaterialFlagName::FLAG_VIEWAMBIENT);
}

void EditorMaterialSystem::ProcessCommand(const Command2* command, bool redo)
{
    // TODO: VK: need to be redesigned after command notification will be changed
    auto commandID = command->GetId();
    if (commandID == CMDID_LOD_DELETE)
    {
        DeleteLODCommand* lodCommand = (DeleteLODCommand*)command;
        const DAVA::Vector<DeleteRenderBatchCommand*> batchCommands = lodCommand->GetRenderBatchCommands();

        const DAVA::uint32 count = (const DAVA::uint32)batchCommands.size();
        for (DAVA::uint32 i = 0; i < count; ++i)
        {
            DAVA::RenderBatch* batch = batchCommands[i]->GetRenderBatch();
            if (redo)
            {
                RemoveMaterial(batch->GetMaterial());
            }
            else
            {
                AddMaterialFromRenderBatchWithEntity(batch, lodCommand->GetEntity());
            }
        }
    }
    else if (commandID == CMDID_LOD_CREATE_PLANE)
    {
        CreatePlaneLODCommand* lodCommand = (CreatePlaneLODCommand*)command;
        DAVA::RenderBatch* batch = lodCommand->GetRenderBatch();
        if (redo)
        {
            AddMaterialFromRenderBatchWithEntity(batch, lodCommand->GetEntity());
        }
        else
        {
            RemoveMaterial(batch->GetMaterial());
        }
    }
    else if (commandID == CMDID_DELETE_RENDER_BATCH)
    {
        DeleteRenderBatchCommand* rbCommand = (DeleteRenderBatchCommand*)command;
        if (redo)
        {
            RemoveMaterial(rbCommand->GetRenderBatch()->GetMaterial());
        }
        else
        {
            AddMaterialFromRenderBatchWithEntity(rbCommand->GetRenderBatch(), rbCommand->GetEntity());
        }
    }
    else if (commandID == CMDID_CONVERT_TO_SHADOW)
    {
        ConvertToShadowCommand* swCommand = (ConvertToShadowCommand*)command;
        if (redo)
        {
            RemoveMaterial(swCommand->oldBatch->GetMaterial());
        }
        else
        {
            AddMaterialFromRenderBatchWithEntity(swCommand->oldBatch, swCommand->GetEntity());
        }
    }
    else if (commandID == CMDID_LOD_COPY_LAST_LOD)
    {
        CopyLastLODToLod0Command* copyCommand = (CopyLastLODToLod0Command*)command;
        DAVA::uint32 batchCount = copyCommand->newBatches.size();
        for (DAVA::uint32 i = 0; i < batchCount; ++i)
        {
            if (redo)
            {
                AddMaterialFromRenderBatchWithEntity(copyCommand->newBatches[i], copyCommand->GetEntity());
            }
            else
            {
                RemoveMaterial(copyCommand->newBatches[i]->GetMaterial());
            }
        }
    }
}

bool EditorMaterialSystem::HasMaterial(DAVA::NMaterial* material) const
{
    return (materialToObjectsMap.count(material) > 0) || (ownedParents.count(material) > 0);
}

void EditorMaterialSystem::AddMaterial(DAVA::NMaterial* material, const MaterialMapping& mapping)
{
    if (nullptr != material)
    {
        DAVA::NMaterial* curGlobalMaterial = nullptr;
        if (nullptr != mapping.entity->GetScene())
        {
            curGlobalMaterial = mapping.entity->GetScene()->GetGlobalMaterial();
        }

        materialToObjectsMap[material] = mapping;

        // remember parent material, if still isn't
        DAVA::NMaterial* parent = material;
        for (;;)
        {
            DAVA::NMaterial* nextParent = parent->GetParent();
            if (nullptr == nextParent || curGlobalMaterial == nextParent)
            {
                break;
            }
            else
            {
                parent = nextParent;
            }
        }

        if (0 == ownedParents.count(parent))
        {
            if (IsEditable(parent))
            {
                ownedParents.insert(parent);
                DAVA::SafeRetain(parent);
                ApplyViewMode(parent);
            }
        }
    }
}

void EditorMaterialSystem::RemoveMaterial(DAVA::NMaterial* material)
{
    auto it = materialToObjectsMap.find(material);
    if (it == materialToObjectsMap.end())
        return;

    if (it->second.mode == MaterialMapping::Mode::RetainedRenderBatch)
    {
        DAVA::SafeRelease(it->second.renderBatch);
    }
    materialToObjectsMap.erase(it);
}

bool EditorMaterialSystem::IsEditable(DAVA::NMaterial* material) const
{
    return (!material->IsRuntime());
}
