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

#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Entity.h"

#include "Utils/Utils.h"

#include "Scene/SceneEditor2.h"


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
{
    pathDrawState = DAVA::RenderManager::Instance()->Subclass3DRenderState(DAVA::RenderStateData::STATE_BLEND |
                                                                          DAVA::RenderStateData::STATE_COLORMASK_ALL |
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
    
    RenderManager::Instance()->SetDynamicParam(PARAM_WORLD, &Matrix4::IDENTITY, (pointer_size)&Matrix4::IDENTITY);
    
    for(DAVA::uint32 p = 0; p < count; ++p)
    {
        DAVA::Entity * path = pathes[p];
        DAVA::Color color;
        
        KeyedArchive *props = GetCustomPropertiesArchieve(path);
        if(props)
        {
            color = DAVA::Color(props->GetVector4(PATH_COLOR_PROP_NAME));
        }

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
                    if(nextEntity)
                    {
                        const Vector3 finishPosition = GetTransformComponent(nextEntity)->GetWorldTransform().GetTranslationVector();
                        
                        RenderHelper::Instance()->DrawArrow(startPosition, finishPosition, (finishPosition - startPosition).Length() / 4.f, 7.f, pathDrawState);
                    }
                }
            }
        }
    }
}

void PathSystem::Process(DAVA::float32 timeElapsed)
{
    SceneEditor2 *sceneEditor = static_cast<SceneEditor2 *>(GetScene());
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
        }
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

