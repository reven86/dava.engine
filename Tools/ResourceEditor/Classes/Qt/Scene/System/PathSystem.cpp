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
#include "Scene3D/Components/ComponentHelpers.h"

#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Entity.h"

#include "Utils/Utils.h"

PathSystem::PathSystem(DAVA::Scene * scene)
    : DAVA::SceneSystem(scene)
{
}

PathSystem::~PathSystem()
{
    pathes.clear();
}


void PathSystem::AddEntity(DAVA::Entity * entity)
{
    pathes.push_back(entity);
}
    
void PathSystem::RemoveEntity(DAVA::Entity * entity)
{
    DAVA::FindAndRemoveExchangingWithLast(pathes, entity);
}
    

DAVA::Entity * PathSystem::GetCurrrentPath() const
{
    //Temporary returns 0 element. Need return current selected entity with PathComponent
    
    if(pathes.size())
    {
        return pathes[0];
    }
    
    return NULL;
}

DAVA::FastName PathSystem::GeneratePathName() const
{
    DAVA::uint32 count = pathes.size();
    
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

