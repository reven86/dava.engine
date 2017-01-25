#pragma once

#include "Classes/Selection/SelectableGroup.h"
#include "Scene/SceneTypes.h"
#include "Classes/Qt/Scene/System/EditorSceneSystem.h"

#include "SystemDelegates.h"

// framework
#include "UI/UIEvent.h"
#include "Entity/SceneSystem.h"
#include "Render/RenderHelper.h"
#include "Scene3D/Components/Waypoint/EdgeComponent.h"

// editor systems
#include "Scene/System/CollisionSystem.h"

// delegate
#include "Scene/System/StructureSystem.h"
#include "Scene/System/ModifSystem.h"

class RECommandNotificationObject;
class SceneEditor2;

class WayEditSystem : public DAVA::SceneSystem,
                      public EntityModificationSystemDelegate,
                      public StructureSystemDelegate,
                      public SelectionSystemDelegate,
                      public EditorSceneSystem

{
    friend class SceneEditor2;

public:
    WayEditSystem(DAVA::Scene* scene, SceneCollisionSystem* collisionSystem);

    void EnableWayEdit(bool enable);
    bool IsWayEditEnabled() const;

    void Process(DAVA::float32 timeElapsed) override;
    bool Input(DAVA::UIEvent* event) override;

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;

    void WillClone(DAVA::Entity* originalEntity) override;
    void DidCloned(DAVA::Entity* originalEntity, DAVA::Entity* newEntity) override;

    void WillRemove(DAVA::Entity* removedEntity) override;
    void DidRemoved(DAVA::Entity* removedEntity) override;

protected:
    void Draw() override;
    void ProcessCommand(const RECommandNotificationObject& commandNotification) override;

    DAVA::Entity* CreateWayPoint(DAVA::Entity* parent, DAVA::Vector3 pos);

    void RemoveEdge(DAVA::Entity* entity, DAVA::EdgeComponent* edgeComponent);

    void DefineAddOrRemoveEdges(const SelectableGroup& srcPoints, DAVA::Entity* dstPoint, SelectableGroup& toAddEdge, SelectableGroup& toRemoveEdge);
    void AddEdges(const SelectableGroup& group, DAVA::Entity* nextEntity);
    void RemoveEdges(const SelectableGroup& group, DAVA::Entity* nextEntity);
    bool IsAccessible(DAVA::Entity* startPoint, DAVA::Entity* breachPoint, DAVA::Entity* excludedPoint, DAVA::EdgeComponent* excludingEdge, DAVA::Set<DAVA::Entity*>& passedPoints) const;

    void ResetSelection();
    void ProcessSelection(const SelectableGroup& selection);
    void UpdateSelectionMask();
    void FilterPrevSelection(DAVA::Entity* parentEntity, SelectableGroup& selection);

    bool AllowPerformSelectionHavingCurrent(const SelectableGroup& currentSelection) override;
    bool AllowChangeSelectionReplacingCurrent(const SelectableGroup& currentSelection, const SelectableGroup& newSelection) override;

private:
    SelectableGroup currentSelection;
    SelectableGroup selectedWaypoints;
    SelectableGroup prevSelectedWaypoints;
    SceneEditor2* sceneEditor = nullptr;
    SceneCollisionSystem* collisionSystem = nullptr;
    DAVA::Vector<DAVA::Entity*> waypointEntities;
    DAVA::Map<DAVA::Entity*, DAVA::Entity*> mapStartPoints; // mapping [path parent -> path start point]
    DAVA::Entity* underCursorPathEntity = nullptr;
    DAVA::Entity* startPointForRemove = nullptr;
    bool inCloneState = false;
    bool isEnabled = false;
};
