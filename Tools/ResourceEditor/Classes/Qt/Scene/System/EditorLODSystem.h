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
#include "Scene3D/Lod/LodComponent.h"
#include "Commands2/CreatePlaneLODCommandHelper.h"
#include "Scene/SceneTypes.h"

namespace DAVA
{
class Entity;
class RenderObject;
}

class Command2;
class SelectableGroup;

struct ForceValues
{
    enum eApplyFlag : DAVA::uint32
    {
        APPLY_DISTANCE = 1 << 0,
        APPLY_LAYER = 1 << 1,

        APPLY_NONE = 0,
        APPLY_ALL = APPLY_DISTANCE | APPLY_LAYER,

        APPLY_DEFAULT = APPLY_LAYER,
    };

    ForceValues(DAVA::float32 distance_ = DAVA::LodComponent::INVALID_DISTANCE,
                DAVA::int32 layer_ = DAVA::LodComponent::INVALID_LOD_LAYER,
                eApplyFlag flag_ = APPLY_DEFAULT)
        : distance(distance_)
        , layer(layer_)
        , flag(flag_)
          {
          };

    DAVA::float32 distance;
    DAVA::int32 layer;
    eApplyFlag flag;
};

class SceneEditor2;
class Command2;
class EditorLODSystem;
class LODComponentHolder
{
    friend class EditorLODSystem;

public:
    DAVA::int32 GetMaxLODLayer() const;
    DAVA::uint32 GetLODLayersCount() const;

    const DAVA::LodComponent& GetLODComponent() const;

protected:
    void BindToSystem(EditorLODSystem* system, SceneEditor2* scene);

    void SummarizeValues();
    void PropagateValues();

    void ApplyForce(const ForceValues& force);
    bool DeleteLOD(DAVA::int32 layer);
    bool CopyLod(DAVA::int32 from, DAVA::int32 to);

protected:
    DAVA::int32 maxLodLayerIndex = DAVA::LodComponent::INVALID_LOD_LAYER;
    DAVA::LodComponent mergedComponent;
    DAVA::Vector<DAVA::LodComponent*> lodComponents;

    EditorLODSystem* system = nullptr;
    SceneEditor2* scene = nullptr;
};

class EditorLODSystemUIDelegate;
class EditorLODSystem : public DAVA::SceneSystem
{
    friend class SceneEditor2;

    enum eLODSystemFlag : DAVA::uint32
    {
        FLAG_MODE = 1 << 0,
        FLAG_FORCE = 1 << 1,
        FLAG_DISTANCE = 1 << 2,
        FLAG_ACTION = 1 << 3,

        FLAG_NONE = 0,
        FLAG_ALL = FLAG_MODE | FLAG_FORCE | FLAG_DISTANCE | FLAG_ACTION
    };

public:
    EditorLODSystem(DAVA::Scene* scene);
    ~EditorLODSystem() override;

    void AddEntity(DAVA::Entity* entity) override;
    void RemoveEntity(DAVA::Entity* entity) override;
    void AddComponent(DAVA::Entity* entity, DAVA::Component* component);
    void RemoveComponent(DAVA::Entity* entity, DAVA::Component* component);

    void Process(DAVA::float32 timeElapsed) override;
    void SceneDidLoaded() override;

    eEditorMode GetMode() const;
    void SetMode(eEditorMode mode);

    //actions
    bool CanDeleteLOD() const;
    bool CanCreateLOD() const;

    void CreatePlaneLOD(DAVA::int32 fromLayer, DAVA::uint32 textureSize, const DAVA::FilePath& texturePath);
    void DeleteFirstLOD();
    void DeleteLastLOD();
    void CopyLastLODToFirst();
    //end of actions

    const ForceValues& GetForceValues() const;
    void SetForceValues(const ForceValues& values);

    const LODComponentHolder* GetActiveLODData() const;

    void SetLODDistances(const DAVA::Vector<DAVA::float32>& distances);

    //scene signals
    void SolidChanged(const DAVA::Entity* entity, bool value);
    void SelectionChanged(const SelectableGroup* selected, const SelectableGroup* deselected);

    void AddDelegate(EditorLODSystemUIDelegate* uiDelegate);
    void RemoveDelegate(EditorLODSystemUIDelegate* uiDelegate);

    DAVA::FilePath GetPathForPlaneEntity() const;

protected:
    void ProcessCommand(const Command2* command, bool redo);

private:
    void RecalculateData();
    //actions
    void CopyLOD(DAVA::int32 fromLayer, DAVA::int32 toLayer);
    void DeleteLOD(DAVA::int32 layer);

    //signals
    void EmitInvalidateUI(DAVA::uint32 flags);
    void DispatchSignals();
    //signals

    void ProcessPlaneLODs();

private:
    LODComponentHolder lodData[eEditorMode::MODE_COUNT];
    LODComponentHolder* activeLodData = nullptr;
    ForceValues forceValues;
    eEditorMode mode = eEditorMode::MODE_DEFAULT;

    DAVA::Vector<CreatePlaneLODCommandHelper::RequestPointer> planeLODRequests;

    bool generateCommands = false;

    DAVA::Vector<EditorLODSystemUIDelegate*> uiDelegates;
    DAVA::uint32 invalidateUIFlag = FLAG_NONE;
};

class EditorLODSystemUIDelegate
{
public:
    virtual ~EditorLODSystemUIDelegate() = default;

    virtual void UpdateModeUI(EditorLODSystem* forSystem, const eEditorMode mode){};
    virtual void UpdateForceUI(EditorLODSystem* forSystem, const ForceValues& forceValues){};
    virtual void UpdateDistanceUI(EditorLODSystem* forSystem, const LODComponentHolder* lodData){};
    virtual void UpdateActionUI(EditorLODSystem* forSystem){};
};


#endif // __SCENE_LOD_SYSTEM_V2_H__
