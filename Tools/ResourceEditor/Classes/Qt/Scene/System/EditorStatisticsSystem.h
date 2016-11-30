#ifndef __EDITOR_STATISTICS_SYSTEM_V2_H__
#define __EDITOR_STATISTICS_SYSTEM_V2_H__

#include "Entity/SceneSystem.h"
#include "Scene/SceneTypes.h"

namespace DAVA
{
class Entity;
class RenderComponent;
}

class EditorStatisticsSystemUIDelegate;
struct TrianglesData;

class EditorStatisticsSystem : public DAVA::SceneSystem
{
    enum eStatisticsSystemFlag : DAVA::uint32
    {
        FLAG_TRIANGLES = 1 << 0,

        FLAG_NONE = 0
    };

public:
    static const DAVA::int32 INDEX_OF_ALL_LODS_TRIANGLES = 0;
    static const DAVA::int32 INDEX_OF_FIRST_LOD_TRIANGLES = 1;

    EditorStatisticsSystem(DAVA::Scene* scene);

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void AddComponent(DAVA::Entity* entity, DAVA::Component* component) override;
    void RemoveComponent(DAVA::Entity* entity, DAVA::Component* component) override;

    void Process(DAVA::float32 timeElapsed) override;

    const DAVA::Vector<DAVA::uint32>& GetTriangles(eEditorMode mode, bool allTriangles) const;

    void AddDelegate(EditorStatisticsSystemUIDelegate* uiDelegate);
    void RemoveDelegate(EditorStatisticsSystemUIDelegate* uiDelegate);

private:
    void CalculateTriangles();

    //signals
    void EmitInvalidateUI(DAVA::uint32 flags);
    void DispatchSignals();
    //signals

private:
    DAVA::Vector<TrianglesData> triangles;

    DAVA::Vector<EditorStatisticsSystemUIDelegate*> uiDelegates;
    DAVA::uint32 invalidateUIflag = FLAG_NONE;
};

class EditorStatisticsSystemUIDelegate
{
public:
    virtual ~EditorStatisticsSystemUIDelegate() = default;

    virtual void UpdateTrianglesUI(EditorStatisticsSystem* forSystem){};
};



#endif // __SCENE_LOD_SYSTEM_V2_H__
