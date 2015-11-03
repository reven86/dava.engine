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


#ifndef __SCENE_WAYEDIT_SYSTEM_H__
#define __SCENE_WAYEDIT_SYSTEM_H__

#include <QMap>
#include "Scene/EntityGroup.h"
#include "Scene/SceneTypes.h"
#include "Commands2/Command2.h"

// framework
#include "UI/UIEvent.h"
#include "Entity/SceneSystem.h"
#include "Render/RenderHelper.h"
#include "Scene3D/Components/Waypoint/EdgeComponent.h"

// editor systems
#include "Scene/System/SelectionSystem.h"
#include "Scene/System/CollisionSystem.h"

// delegate
#include "Scene/System/StructureSystem.h"
#include "Scene/System/ModifSystem.h"

class SceneEditor2;

class WayEditSystem : public DAVA::SceneSystem
        , public EntityModificationSystemDelegate
        , public StructureSystemDelegate
{
    friend class SceneEditor2;

public:
    WayEditSystem(DAVA::Scene * scene, SceneSelectionSystem *selectionSystem, SceneCollisionSystem *collisionSystem);
    ~WayEditSystem() override;

    void EnableWayEdit(bool enable);
    bool IsWayEditEnabled() const;

    void Process(DAVA::float32 timeElapsed) override;
    void Input(DAVA::UIEvent *event) override;

    void AddEntity(DAVA::Entity * entity) override;
    void RemoveEntity(DAVA::Entity * entity) override;

    void WillClone(DAVA::Entity *originalEntity) override;
    void DidCloned(DAVA::Entity *originalEntity, DAVA::Entity *newEntity) override;

    void WillRemove(DAVA::Entity *removedEntity) override;
    void DidRemoved(DAVA::Entity *removedEntity) override;

protected:
    void Draw();

    void ProcessCommand(const Command2 *command, bool redo);

    DAVA::Entity* CreateWayPoint(DAVA::Entity *parent, DAVA::Vector3 pos);

    void RemoveEdge(DAVA::Entity* entity, DAVA::EdgeComponent * edgeComponent);

    void DefineAddOrRemoveEdges(const EntityGroup& srcPoints, DAVA::Entity* dstPoint, EntityGroup& toAddEdge, EntityGroup& toRemoveEdge);
    void AddEdges(const EntityGroup & group, DAVA::Entity *nextEntity);
    void RemoveEdges(const EntityGroup & group, DAVA::Entity *nextEntity);
    bool IsAccessible(DAVA::Entity* startPoint, DAVA::Entity* breachPoint, DAVA::Entity* excludedPoint, DAVA::EdgeComponent* excludingEdge, DAVA::Set<DAVA::Entity*>& passedPoints) const;

    void ResetSelection();
    void ProcessSelection();
    void UpdateSelectionMask();
    EntityGroup FilterPrevSelection(DAVA::Entity *parentEntity);

protected:
    bool isEnabled;

    EntityGroup currentSelection;
    EntityGroup selectedWaypoints;
    EntityGroup prevSelectedWaypoints;

    SceneEditor2 *sceneEditor;
    SceneSelectionSystem *selectionSystem;
    SceneCollisionSystem* collisionSystem;

    DAVA::Vector<DAVA::Entity *> waypointEntities;
    DAVA::Map<DAVA::Entity*, DAVA::Entity*> mapStartPoints; // mapping [path parent -> path start point]

    DAVA::Entity * underCursorPathEntity;
    bool inCloneState = false;

    DAVA::Entity *startPointForRemove;
};

#endif // __SCENE_WAYEDIT_SYSTEM_H__
