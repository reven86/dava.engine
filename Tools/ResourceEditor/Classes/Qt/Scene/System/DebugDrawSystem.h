#pragma once

#include "Classes/Constants.h"

#include "Scene/System/CollisionSystem.h"
#include "Classes/Qt/Scene/System/EditorSceneSystem.h"

#include "Render/UniqueStateSet.h"

class DebugDrawSystem : public DAVA::SceneSystem, public EditorSceneSystem
{
    friend class SceneEditor2;
    friend class EditorScene;

public:
    static DAVA::float32 HANGING_OBJECTS_HEIGHT;

public:
    DebugDrawSystem(DAVA::Scene* scene);
    virtual ~DebugDrawSystem();

    void SetRequestedObjectType(ResourceEditor::eSceneObjectType objectType);
    ResourceEditor::eSceneObjectType GetRequestedObjectType() const;

    inline void EnableHangingObjectsMode(bool enabled);
    inline bool HangingObjectsModeEnabled() const;

    //need be moved to testing tool
    DAVA_DEPRECATED(inline void EnableSwithcesWithDifferentLODsMode(bool enabled));
    DAVA_DEPRECATED(inline bool SwithcesWithDifferentLODsModeEnabled() const);

protected:
    void Draw() override;

private:
    void Draw(DAVA::Entity* entity);

    void DrawObjectBoxesByType(DAVA::Entity* entity);
    void DrawUserNode(DAVA::Entity* entity);
    void DrawLightNode(DAVA::Entity* entity);
    void DrawSoundNode(DAVA::Entity* entity);
    void DrawSelectedSoundNode(DAVA::Entity* entity);
    void DrawHangingObjects(DAVA::Entity* entity);
    void DrawEntityBox(DAVA::Entity* entity, const DAVA::Color& color);
    void DrawSwitchesWithDifferentLods(DAVA::Entity* entity);
    void DrawWindNode(DAVA::Entity* entity);

    //hanging objects
    using RenderBatchWithTransform = std::pair<DAVA::RenderBatch*, DAVA::Matrix4>;
    using RenderBatchesWithTransforms = DAVA::Vector<RenderBatchWithTransform>;

    void CollectRenderBatchesRecursively(DAVA::Entity* entity, RenderBatchesWithTransforms& batches) const;
    DAVA::float32 GetMinimalZ(const RenderBatchesWithTransforms& batches) const;
    void GetLowestVertexes(const RenderBatchesWithTransforms& batches, DAVA::Vector<DAVA::Vector3>& vertexes) const;
    DAVA::Vector3 GetLandscapePointAtCoordinates(const DAVA::Vector2& centerXY) const;
    bool IsObjectHanging(DAVA::Entity* entity) const;

    SceneCollisionSystem* collSystem = nullptr;
    DAVA::Color objectTypeColor = DAVA::Color::White;
    ResourceEditor::eSceneObjectType objectType = ResourceEditor::ESOT_NONE;
    bool hangingObjectsModeEnabled = false;
    bool switchesWithDifferentLodsEnabled = false;
};

inline void DebugDrawSystem::EnableHangingObjectsMode(bool enabled)
{
    hangingObjectsModeEnabled = enabled;
}

inline bool DebugDrawSystem::HangingObjectsModeEnabled() const
{
    return hangingObjectsModeEnabled;
}

inline void DebugDrawSystem::EnableSwithcesWithDifferentLODsMode(bool enabled)
{
    switchesWithDifferentLodsEnabled = enabled;
}

inline bool DebugDrawSystem::SwithcesWithDifferentLODsModeEnabled() const
{
    return switchesWithDifferentLodsEnabled;
}
