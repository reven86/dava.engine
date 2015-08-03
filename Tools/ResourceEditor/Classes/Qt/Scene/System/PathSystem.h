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


#ifndef __PATH_SYSTEM_H__
#define __PATH_SYSTEM_H__

#include "Entity/SceneSystem.h"
#include "Base/FastName.h"

#include "Scene/System/SelectionSystem.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"

#include "Scene/System/ModifSystem.h"

static const DAVA::uint32 WAYPOINTS_DRAW_LIFTING = 1;

class SceneEditor2;
class PathSystem : public DAVA::SceneSystem, public EntityModificationSystemDelegate
{
    friend class SceneEditor2;

public:
    PathSystem(DAVA::Scene * scene);
    ~PathSystem() override;

    void EnablePathEdit(bool enable);
    bool IsPathEditEnabled() const;

    void AddEntity(DAVA::Entity * entity) override;
    void RemoveEntity(DAVA::Entity * entity) override;

    void Process(DAVA::float32 timeElapsed) override;

    DAVA::Entity * GetCurrrentPath() const;
    const DAVA::Vector<DAVA::Entity *> & GetPathes() const;

    void AddPath(DAVA::Entity* pathEntity);
    
    DAVA::PathComponent* CreatePathComponent();

    void WillClone(DAVA::Entity *originalEntity) override;
    void DidCloned(DAVA::Entity *originalEntity, DAVA::Entity *newEntity) override;

protected:
    void Draw();
    void DrawInEditableMode();
    void DrawInViewOnlyMode();
    void DrawArrow(const DAVA::Vector3 & start, const DAVA::Vector3 & finish, const DAVA::Color & color);

    void ProcessCommand(const Command2 *command, bool redo);

    DAVA::FastName GeneratePathName() const;
    const DAVA::Color & GetNextPathColor() const;

    void ExpandPathEntity(const DAVA::Entity*);
    void CollapsePathEntity(const DAVA::Entity*);

    SceneEditor2* sceneEditor;
    
    DAVA::Vector<DAVA::Entity *> pathes;
   
    EntityGroup currentSelection;
    DAVA::Entity * currentPath;

    bool isEditingEnabled;
};

inline const DAVA::Vector<DAVA::Entity *> & PathSystem::GetPathes() const
{
    return pathes;
}

inline DAVA::Entity * PathSystem::GetCurrrentPath() const
{
    return currentPath;
}

inline bool PathSystem::IsPathEditEnabled() const
{
    return isEditingEnabled;
}

#endif // __PATH_SYSTEM_H__
