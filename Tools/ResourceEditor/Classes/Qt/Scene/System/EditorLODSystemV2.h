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


#ifndef __SCENE_LOD_SYSTEM_V2_H__
#define __SCENE_LOD_SYSTEM_V2_H__

#include "Entity/SceneSystem.h"
#include "Scene3D/Components/LodComponent.h"

namespace DAVA
{
    class Entity;
    class RenderObject;
}

struct ForceValues
{
    enum eApplyFlag: DAVA::uint32
    {
        APPLY_DISTANCE = 1 << 0,
        APPLY_LAYER = 1 << 1,

        APPLY_NONE = 0,
        APPLY_BOTH = APPLY_DISTANCE | APPLY_LAYER,

        APPLY_DEFAULT = APPLY_LAYER,
    };

    ForceValues(DAVA::float32 distance_ = DAVA::LodComponent::INVALID_DISTANCE,
                DAVA::int32 layer_ = DAVA::LodComponent::INVALID_LOD_LAYER, 
                eApplyFlag flag_ = APPLY_DEFAULT) 
        : distance(distance_), layer(layer_), flag(flag_) 
    {
    };

    DAVA::float32 distance;
    DAVA::int32 layer;
    eApplyFlag flag;
};

class SceneEditor2;
class Command2;
class LODComponentHolder
{
    friend class EditorLODSystemV2;
public:

    bool IsMultyComponent() const;

    DAVA::int32 GetMaxLODLayer() const;
    DAVA::uint32 GetLODLayersCount() const;

    const DAVA::LodComponent & GetLODComponent() const;
    const DAVA::Array<DAVA::uint32, DAVA::LodComponent::MAX_LOD_LAYERS> &GetTriangles() const;

protected:

    void BindToSystem(EditorLODSystemV2 *system, SceneEditor2 *scene);

    void SummarizeValues();
    void PropagateValues();

    void ApplyForce(const ForceValues &force);
    bool DeleteLOD(DAVA::int32 layer);

private:
    
    void InsertObjectWithTriangles(DAVA::Entity *entity, DAVA::UnorderedSet<DAVA::RenderObject*> &renderObjects);
    void CalculateTriangles(const DAVA::UnorderedSet<DAVA::RenderObject *> &renderObjects);

protected:

    DAVA::int32 maxLodLayerIndex = DAVA::LodComponent::INVALID_LOD_LAYER;
    DAVA::LodComponent mergedComponent;
    DAVA::Vector<DAVA::LodComponent *> lodComponents;

    DAVA::Array<DAVA::uint32, DAVA::LodComponent::MAX_LOD_LAYERS> trianglesCount;

    EditorLODSystemV2 *system = nullptr;
    SceneEditor2 * scene = nullptr;
};

class EntityGroup;
class EditorLODSystemV2UIDelegate;
class EditorLODSystemV2 : public DAVA::SceneSystem
{
    friend class SceneEditor2;
public:

    enum eMode: DAVA::uint32 
    {
        MODE_ALL_SCENE = 0,
        MODE_SELECTION,

        MODE_COUNT,
        MODE_DEFAULT = MODE_SELECTION
    };

    EditorLODSystemV2(DAVA::Scene * scene);
    ~EditorLODSystemV2() override;

    void AddEntity(DAVA::Entity * entity) override;
    void RemoveEntity(DAVA::Entity * entity) override;
    void AddComponent(DAVA::Entity * entity, DAVA::Component * component);
    void RemoveComponent(DAVA::Entity * entity, DAVA::Component * component);

    void Process(DAVA::float32 timeElapsed) override;

    eMode GetMode() const;
    void SetMode(eMode mode);

    //actions
    bool CanDeleteLOD() const;
    bool CanCreateLOD() const;

    void CreatePlaneLOD();
    void DeleteFirstLOD();
    void DeleteLastLOD();
    void CopyLastLODToFirst();
    //end of actions

    const ForceValues &GetForceValues() const;
    void SetForceValues(const ForceValues & values);

    const LODComponentHolder * GetActiveLODData() const;

    void SetLODDistances(const DAVA::Array<DAVA::float32, DAVA::LodComponent::MAX_LOD_LAYERS> &distances);

    //scene signals
    void SolidChanged(const DAVA::Entity *entity, bool value);
    void SelectionChanged(const EntityGroup *selected, const EntityGroup *deselected);

    void SetDelegate(EditorLODSystemV2UIDelegate *uiDelegate);

protected:
    void ProcessCommand(const Command2 *command, bool redo);

private:

    //actions
    void CopyLOD(DAVA::int32 fromLayer, DAVA::int32 toLayer);
    void DeleteLOD(DAVA::int32 layer); 

    //signals
    void EmitUpdateModeUI() { updateModeUI = true; };
    void EmitUpdateForceUI() { updateForceUI = true; };
    void EmitUpdateDistanceUI() { updateDistanceUI = true; };
    void EmitUpdateActionsUI() { updateActionUI = true; };
    void DispatchSignals();
    //signals

private:

    LODComponentHolder lodData[MODE_COUNT];
    LODComponentHolder *activeLodData = nullptr;

    ForceValues forceValues;
    eMode mode = MODE_DEFAULT;

    bool updateModeUI = false;
    bool updateForceUI = false;
    bool updateDistanceUI = false;
    bool updateActionUI = false;

    EditorLODSystemV2UIDelegate *uiDelegate = nullptr;
};

class EditorLODSystemV2UIDelegate
{
public:

    virtual ~EditorLODSystemV2UIDelegate() = default;

    virtual void UpdateModeUI(EditorLODSystemV2 *forSystem, const EditorLODSystemV2::eMode mode){};
    virtual void UpdateForceUI(EditorLODSystemV2 *forSystem, const ForceValues & forceValues){};
    virtual void UpdateDistanceUI(EditorLODSystemV2 *forSystem, const LODComponentHolder *lodData){};
    virtual void UpdateActionUI(EditorLODSystemV2 *forSystem){};
};


#endif // __SCENE_LOD_SYSTEM_V2_H__

