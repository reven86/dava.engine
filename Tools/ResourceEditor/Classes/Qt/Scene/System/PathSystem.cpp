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
}

PathSystem::~PathSystem()
{
    currentPath = NULL;
    
    pathes.clear();
    currentSelection.Clear();
}


void PathSystem::AddEntity(DAVA::Entity * entity)
{
    pathes.push_back(entity);
    
    if(!currentPath)
    {
        currentPath = entity;
    }
    
    //validate color
    KeyedArchive *props = GetCustomPropertiesArchieve(entity);
    if(props)
    {
        if(!props->IsKeyExists(PATH_COLOR_PROP_NAME))
        {
            const DAVA::Color & color = GetNextPathColor();
            props->SetVector4(PATH_COLOR_PROP_NAME, DAVA::Vector4(color.r, color.g, color.b, color.a));
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

    const DAVA::uint32 count = pathes.size();
    for(DAVA::uint32 p = 0; p < count; ++p)
    {
        DAVA::Entity * path = pathes[p];
        if(path->GetVisible() == false)
        {   // we don't need draw hidden pathes
            continue;
        }
        
        DAVA::Color color = GetPathColor(path);
        RenderManager::Instance()->SetColor(color);
        
        const DAVA::uint32 childrenCount = path->GetChildrenCount();
        for(DAVA::uint32 c = 0; c < childrenCount; ++c)
        {
            DAVA::Entity * waypoint = path->GetChild(c);
            
            const DAVA::uint32 edgesCount = waypoint->GetComponentCount(DAVA::Component::EDGE_COMPONENT);
            if(edgesCount)
            {
                const Vector3 startPosition = GetTransformComponent(waypoint)->GetWorldTransform().GetTranslationVector();
                for(DAVA::uint32 e = 0; e < edgesCount; ++e)
                {
                    DAVA::EdgeComponent * edge = static_cast<DAVA::EdgeComponent *>(waypoint->GetComponent(DAVA::Component::EDGE_COMPONENT, e));
                    DAVA::Entity *nextEntity = edge->GetNextEntity();
                    if(nextEntity && nextEntity->GetParent())
                    {
                        const Vector3 finishPosition = GetTransformComponent(nextEntity)->GetWorldTransform().GetTranslationVector();
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

    SceneEditor2 *sceneEditor = GetSceneEditor();
    EntityGroup gruop = sceneEditor->selectionSystem->GetSelection();

    const DAVA::float32 boxScale = SettingsManager::GetValue(Settings::Scene_DebugBoxWaypointScale).AsFloat();

    
    const size_t count = gruop.Size();
    for(size_t p = 0; p < count; ++p)
    {
        DAVA::Entity * path = gruop.GetEntity(p);
        DAVA::PathComponent *pathComponent = DAVA::GetPathComponent(path);
        if(path->GetVisible() == false || !pathComponent)
        {
            continue;
        }
     
        RenderManager::SetDynamicParam(PARAM_WORLD, &path->GetWorldTransform(), (pointer_size)&path->GetWorldTransform());

        DAVA::Color pathColor = GetPathColor(path);

        
        const Vector<PathComponent::Waypoint *> & waypoints = pathComponent->GetPoints();
        const DAVA::uint32 waypointsCount = (const DAVA::uint32)waypoints.size();
        for(DAVA::uint32 w = 0; w < waypointsCount; ++w)
        {
            const DAVA::AABBox3 wpBoundingBox(waypoints[w]->position, boxScale);
            bool isStarting = waypoints[w]->IsStarting();
            
            DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.3f, 0.3f, isStarting? 1.0f : 0.0f, 0.3f));
            DAVA::RenderHelper::Instance()->FillBox(wpBoundingBox, pathDrawState);
            DAVA::RenderManager::Instance()->SetColor(DAVA::Color(0.7f, 0.7f, isStarting ? 0.7f : 0.0f, 1.0f));
            DAVA::RenderHelper::Instance()->DrawBox(wpBoundingBox, 1.0f, pathDrawState);
        
            //draw edges
            const DAVA::uint32 edgesCount = (const DAVA::uint32)waypoints[w]->edges.size();
            if(edgesCount)
            {
                RenderManager::Instance()->SetColor(pathColor);

                
                const Vector3 & startPosition = waypoints[w]->position;
                for(DAVA::uint32 e = 0; e < edgesCount; ++e)
                {
                    const DAVA::PathComponent::Edge *edge = waypoints[w]->edges[e];
                    const Vector3 & finishPosition = edge->destination->position;
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



DAVA::Color PathSystem::GetPathColor(DAVA::Entity *path)
{
    DVASSERT(path);
    
    KeyedArchive *props = GetCustomPropertiesArchieve(path);
    if(props)
    {
        return DAVA::Color(props->GetVector4(PATH_COLOR_PROP_NAME));
    }
    
    return DAVA::Color::White;
}

SceneEditor2* PathSystem::GetSceneEditor() const
{
    return static_cast<SceneEditor2 *>(GetScene());
}

void PathSystem::Process(DAVA::float32 timeElapsed)
{
    SceneEditor2 *sceneEditor = GetSceneEditor();
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
    
    //Enable system without runnig commands
    if(CMDID_COLLAPSE_PATH == commandId)
    {
        isEditingEnabled = !redo;
    }
    else if(CMDID_EXPAND_PATH == commandId)
    {
        isEditingEnabled = redo;
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
    SceneEditor2 *sceneEditor = GetSceneEditor();
    DVASSERT(sceneEditor);

    isEditingEnabled = enable;
    if(isEditingEnabled)
    {
        sceneEditor->BeginBatch("Expand pathes to entities");

        const DAVA::uint32 parentsCount = pathes.size();
        for(DAVA::uint32 p = 0; p < parentsCount; ++p)
        {
            DAVA::Entity * pathParent = pathes[p];
            DAVA::uint32 pathComponentCount = pathParent->GetComponentCount(DAVA::Component::PATH_COMPONENT);
            for (DAVA::uint32 i=0; i<pathComponentCount; ++i)
            {
                DAVA::PathComponent* pathComponent = static_cast<DAVA::PathComponent*>(pathParent->GetComponent(DAVA::Component::PATH_COMPONENT,i));
                DVASSERT(pathComponent);
                sceneEditor->Exec(new ExpandPathCommand(pathComponent));
            }
        }

        sceneEditor->EndBatch();
    }
    else
    {
        sceneEditor->BeginBatch("Collapse entities to pathes");

        const DAVA::uint32 parentsCount = pathes.size();
        for(DAVA::uint32 p = 0; p < parentsCount; ++p)
        {
            DAVA::Entity * pathParent = pathes[p];
            DAVA::uint32 pathComponentCount = pathParent->GetComponentCount(DAVA::Component::PATH_COMPONENT);
            for (DAVA::uint32 i=0; i<pathComponentCount; ++i)
            {
                DAVA::PathComponent* pathComponent = static_cast<DAVA::PathComponent*>(pathParent->GetComponent(DAVA::Component::PATH_COMPONENT,i));
                DVASSERT(pathComponent);
                sceneEditor->Exec(new CollapsePathCommand(pathComponent));
            }
        }

        sceneEditor->EndBatch();
    }
}


