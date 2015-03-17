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

#include "PathSystem.h"

#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Components/Waypoint/WaypointComponent.h"
#include "Scene3D/Components/Waypoint/EdgeComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"

#include "Commands2/ConvertPathCommands.h"

#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Entity.h"

#include "Utils/Utils.h"

#include "Scene/SceneEditor2.h"
#include "Commands2/InspMemberModifyCommand.h"
#include "Commands2/WayEditCommands.h"


static DAVA::Color PathColorPallete[] =
{
    DAVA::Color(0x00ffffff),
    DAVA::Color(0x000000ff),
    DAVA::Color(0x0000ffff),
    DAVA::Color(0xff00ffff),

    DAVA::Color(0x808080ff),
    DAVA::Color(0x008000ff),
    DAVA::Color(0x00ff00ff),
    DAVA::Color(0x80000ff),

    DAVA::Color(0x000080ff),
    DAVA::Color(0x808000ff),
    DAVA::Color(0x800080ff),
    DAVA::Color(0xff0000ff),
    
    DAVA::Color(0xc0c0c0ff),
    DAVA::Color(0x008080ff),
    DAVA::Color(0xffffffff),
    DAVA::Color(0xffff00ff)
};

static const uint32 PALLETE_SIZE = COUNT_OF(PathColorPallete);
static const String PATH_COLOR_PROP_NAME = "pathColor";


PathSystem::PathSystem(DAVA::Scene * scene)
    : DAVA::SceneSystem(scene)
    , currentPath(NULL)
    , isEditingEnabled(false)
{
    pathDrawState = DAVA::RenderManager::Instance()->Subclass3DRenderState(DAVA::RenderStateData::STATE_COLORMASK_ALL |
                                                                          DAVA::RenderStateData::STATE_DEPTH_TEST);
    sceneEditor = static_cast<SceneEditor2 *>(GetScene());
}

PathSystem::~PathSystem()
{
    currentPath = NULL;
    
    pathes.clear();
    currentSelection.Clear();
}

void PathSystem::AddPath(DAVA::Entity * entity)
{
    sceneEditor->BeginBatch("Add path at scene");
    sceneEditor->Exec(new EntityAddCommand(entity, sceneEditor));

    if (isEditingEnabled)
        ExpandPathEntity(entity);

    sceneEditor->EndBatch();
}

void PathSystem::AddEntity(DAVA::Entity * entity)
{
    pathes.push_back(entity);
    
    if(!currentPath)
    {
        currentPath = entity;
    }

    // extract color data from custom properties for old scenes
    PathComponent* pc = GetPathComponent(entity);
    if (pc && pc->GetColor() == Color())
    {
        KeyedArchive *props = GetCustomPropertiesArchieve(entity);
        if (props && props->IsKeyExists(PATH_COLOR_PROP_NAME))
        {
            pc->SetColor(DAVA::Color(props->GetVector4(PATH_COLOR_PROP_NAME)));
            props->DeleteKey(PATH_COLOR_PROP_NAME);
        }
    }
}

void PathSystem::RemoveEntity(DAVA::Entity * entity)
{
    DAVA::FindAndRemoveExchangingWithLast(pathes, entity);
    
    if(pathes.size())
    {
        if(entity == currentPath)
        {
            currentPath = pathes[0];
        }
    }
    else
    {
        currentPath = NULL;
    }
}



void PathSystem::Draw()
{
    const DAVA::uint32 count = pathes.size();
    if(!count) return;

    if(isEditingEnabled)
    {
        DrawInEditableMode();
    }
    else
    {
        DrawInViewOnlyMode();
    }
}

void PathSystem::DrawInEditableMode()
{
    RenderManager::Instance()->SetDynamicParam(PARAM_WORLD, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);

    for (DAVA::Entity* path : pathes)
    {
        PathComponent* pc = GetPathComponent(path);
        if (!path->GetVisible() || !pc)
        {
            continue;
        }
        
        RenderManager::Instance()->SetColor(pc->GetColor());
        
        const DAVA::uint32 childrenCount = path->GetChildrenCount();
        for(DAVA::uint32 c = 0; c < childrenCount; ++c)
        {
            DAVA::Entity * waypoint = path->GetChild(c);
            
            const DAVA::uint32 edgesCount = waypoint->GetComponentCount(DAVA::Component::EDGE_COMPONENT);
            if(edgesCount)
            {
                Vector3 startPosition = GetTransformComponent(waypoint)->GetWorldTransform().GetTranslationVector();
                startPosition.z += WAYPOINTS_DRAW_LIFTING;
                for(DAVA::uint32 e = 0; e < edgesCount; ++e)
                {
                    DAVA::EdgeComponent * edge = static_cast<DAVA::EdgeComponent *>(waypoint->GetComponent(DAVA::Component::EDGE_COMPONENT, e));
                    DAVA::Entity *nextEntity = edge->GetNextEntity();
                    if(nextEntity && nextEntity->GetParent())
                    {
                        Vector3 finishPosition = GetTransformComponent(nextEntity)->GetWorldTransform().GetTranslationVector();
                        finishPosition.z += WAYPOINTS_DRAW_LIFTING;
                        DrawArrow(startPosition, finishPosition);
                    }
                }
            }
        }
    }
}

void PathSystem::DrawInViewOnlyMode()
{
    RenderManager::Instance()->SetDynamicParam(PARAM_WORLD, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
    const DAVA::float32 boxScale = SettingsManager::GetValue(Settings::Scene_DebugBoxWaypointScale).AsFloat();

    EntityGroup selection = sceneEditor->selectionSystem->GetSelection();

    const size_t count = selection.Size();
    for(size_t p = 0; p < count; ++p)
    {
        DAVA::Entity * path = selection.GetEntity(p);
        DAVA::PathComponent *pathComponent = DAVA::GetPathComponent(path);
        if(path->GetVisible() == false || !pathComponent)
        {
            continue;
        }
     
        RenderManager::SetDynamicParam(PARAM_WORLD, &path->GetWorldTransform(), (pointer_size)&path->GetWorldTransform());

        const Vector<PathComponent::Waypoint *> & waypoints = pathComponent->GetPoints();
        for (auto waypoint : waypoints)
        {
            Vector3 startPosition = waypoint->position;
            const DAVA::AABBox3 wpBoundingBox(startPosition, boxScale);
            bool isStarting = waypoint->IsStarting();
            
            DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.3f, 0.3f, isStarting ? 1.0f : 0.0f, 0.3f));
            DAVA::RenderHelper::Instance()->FillBox(wpBoundingBox, pathDrawState);
            DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.7f, 0.7f, isStarting ? 0.7f : 0.0f, 1.0f));
            DAVA::RenderHelper::Instance()->DrawBox(wpBoundingBox, 1.0f, pathDrawState);
        
            //draw edges
            if (!waypoint->edges.empty())
            {
                RenderManager::Instance()->SetColor(pathComponent->GetColor());
                startPosition.z += WAYPOINTS_DRAW_LIFTING;
                for (auto edge : waypoint->edges)
                {
                    Vector3 finishPosition = edge->destination->position;
                    finishPosition.z += WAYPOINTS_DRAW_LIFTING;
                    DrawArrow(startPosition, finishPosition);
                }
            }
        }
    }
}

void PathSystem::DrawArrow(const DAVA::Vector3 & start, const DAVA::Vector3 & finish)
{
    RenderHelper::Instance()->DrawArrow(start, finish, (finish - start).Length() / 4.f, 7.f, pathDrawState);
}

void PathSystem::Process(DAVA::float32 timeElapsed)
{
    const EntityGroup selection = sceneEditor->selectionSystem->GetSelection();
    if(currentSelection != selection)
    {
        currentSelection = selection;
        
        const size_t count = currentSelection.Size();
        for(size_t i = 0; i < count; ++i)
        {
            Entity * entity = currentSelection.GetEntity(i);
            if(entity->GetComponent(Component::PATH_COMPONENT))
            {
                currentPath = entity;
                break;
            }
            
            if(GetWaypointComponent(entity) && GetPathComponent(entity->GetParent()))
            {
                currentPath = entity->GetParent();
                break;
            }
        }
    }
}

void PathSystem::ProcessCommand(const Command2 *command, bool redo)
{
    const int commandId = command->GetId();
    if(CMDID_INSP_MEMBER_MODIFY == commandId)
    {
        const InspMemberModifyCommand* cmd = static_cast<const InspMemberModifyCommand*>(command);
        if (String("name") == cmd->member->Name())
        {
            const DAVA::uint32 count = pathes.size();
            for(DAVA::uint32 p = 0; p < count; ++p)
            {
                const DAVA::PathComponent *pc = DAVA::GetPathComponent(pathes[p]);

                if(cmd->object == pc)
                {
                    FastName newPathName = (redo) ? cmd->newValue.AsFastName(): cmd->oldValue.AsFastName();
                    FastName oldPathName = (redo) ? cmd->oldValue.AsFastName(): cmd->newValue.AsFastName();
                    
                    const DAVA::uint32 childrenCount = pathes[p]->GetChildrenCount();
                    for(DAVA::uint32 c = 0; c < childrenCount; ++c)
                    {
                        DAVA::WaypointComponent *wp = GetWaypointComponent(pathes[p]->GetChild(c));
                        
                        if(wp && wp->GetPathName() == oldPathName)
                        {
                            wp->SetPathName(newPathName);
                        }
                    }
                    
                    break;
                }
            }
        }
    }
    else if (commandId == CMDID_ENABLE_WAYEDIT)
    {
        isEditingEnabled = redo;
    }
    else if (commandId == CMDID_DISABLE_WAYEDIT)
    {
        isEditingEnabled = !redo;
    }
}


DAVA::FastName PathSystem::GeneratePathName() const
{
    const DAVA::uint32 count = pathes.size();
    
    for(DAVA::uint32 i = 0; i <= count; ++i)
    {
        DAVA::FastName generatedName(DAVA::Format("path_%02d", i));
        
        bool found = false;
        
        for(DAVA::uint32 p = 0; p < count; ++p)
        {
            const DAVA::PathComponent *pc = DAVA::GetPathComponent(pathes[p]);
            if(generatedName == pc->GetName())
            {
                found = true;
                break;
            }
        }
        
        if(!found)
            return generatedName;
    }
    
    return DAVA::FastName();
}

const DAVA::Color & PathSystem::GetNextPathColor() const
{
    const DAVA::uint32 count = pathes.size();
    const DAVA::uint32 index = count % PALLETE_SIZE;
    
    return PathColorPallete[index];
}

void PathSystem::EnablePathEdit(bool enable)
{
    if (enable)
    {
        sceneEditor->BeginBatch("Enable waypoints edit");
        sceneEditor->Exec(new EnableWayEditCommand);

        for (auto path : pathes)
        {
            ExpandPathEntity(path);
        }

        sceneEditor->EndBatch();
    }
    else
    {
        sceneEditor->BeginBatch("Disable waypoints edit");
        sceneEditor->Exec(new DisableWayEditCommand);

        for (auto path : pathes)
        {
            CollapsePathEntity(path);
        }

        sceneEditor->EndBatch();
    }
}

void PathSystem::ExpandPathEntity(const DAVA::Entity* pathEntity)
{
    DAVA::uint32 pathComponentCount = pathEntity->GetComponentCount(DAVA::Component::PATH_COMPONENT);
    for (DAVA::uint32 i = 0; i < pathComponentCount; ++i)
    {
        DAVA::PathComponent* pathComponent = static_cast<DAVA::PathComponent*>(pathEntity->GetComponent(DAVA::Component::PATH_COMPONENT, i));
        DVASSERT(pathComponent);
        sceneEditor->Exec(new ExpandPathCommand(pathComponent));
    }
}

void PathSystem::CollapsePathEntity(const DAVA::Entity* pathEntity)
{
    DAVA::uint32 pathComponentCount = pathEntity->GetComponentCount(DAVA::Component::PATH_COMPONENT);
    for (DAVA::uint32 i = 0; i < pathComponentCount; ++i)
    {
        DAVA::PathComponent* pathComponent = static_cast<DAVA::PathComponent*>(pathEntity->GetComponent(DAVA::Component::PATH_COMPONENT, i));
        DVASSERT(pathComponent);
        sceneEditor->Exec(new CollapsePathCommand(pathComponent));
    }
}

DAVA::PathComponent* PathSystem::CreatePathComponent()
{
    DAVA::PathComponent *pc = new PathComponent();
    pc->SetName(GeneratePathName());
    pc->SetColor(GetNextPathColor());
    return pc;
}

