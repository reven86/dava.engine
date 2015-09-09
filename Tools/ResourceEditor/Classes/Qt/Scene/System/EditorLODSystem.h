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


#ifndef __SCENE_LOD_SYSTEM_H__
#define __SCENE_LOD_SYSTEM_H__

#include "Commands2/CreatePlaneLODCommandHelper.h"

class Command2;
class SceneEditor2;
class EntityGroup;

class EditorLODSystem : public DAVA::SceneSystem
{
    struct ForceData
    {
        DAVA::int32 forceLayer;
        DAVA::float32 forceDistance;

        ForceData(DAVA::int32 newForceLayer = -1, DAVA::float32 newDistance = -1);
    };

    friend class SceneEditor2;
    friend class EntityModificationSystem;

public:
    EditorLODSystem(DAVA::Scene * scene);
    virtual ~EditorLODSystem();

    void AddEntity(DAVA::Entity * entity) override;
    void RemoveEntity(DAVA::Entity * entity) override;

    void UpdateDistances(const DAVA::Map<DAVA::uint32, DAVA::float32> & lodDistances);

    DAVA::FilePath GetDefaultTexturePathForPlaneEntity() const;

    static void AddTrianglesInfo(std::array<DAVA::uint32, DAVA::LodComponent::MAX_LOD_LAYERS> &triangles, DAVA::LodComponent *lod, bool onlyVisibleBatches);

    void SolidChanged(const DAVA::Entity *entity, bool value);

    bool CreatePlaneLOD(DAVA::int32 fromLayer, DAVA::uint32 textureSize, const DAVA::FilePath & texturePath);
    bool CanCreatePlaneLOD() const;
    //TODO: remove after lod editing implementation
    DAVA_DEPRECATED(bool CopyLastLodToLod0());

    bool CanDeleteLod() const;

    bool DeleteFirstLOD();
    bool DeleteLastLOD();

    void SceneSelectionChanged(const EntityGroup *selected, const EntityGroup *deselected);

    inline DAVA::uint32 GetLayerTriangles(DAVA::uint32 layerNum) const;

    inline DAVA::float32 GetLayerDistance(DAVA::uint32 layerNum) const;
    void SetLayerDistance(DAVA::int32 layerNum, DAVA::float32 distance);

    inline DAVA::uint32 GetCurrentLodsLayersCount() const;

    inline bool GetForceDistanceEnabled() const;
    void SetForceDistanceEnabled(bool enable);

    inline DAVA::float32 GetForceDistance() const;
    void SetForceDistance(DAVA::float32 distance);
    DAVA::float32 GetCurrentDistance() const;

    inline DAVA::int32 GetForceLayer() const;
    void SetForceLayer(DAVA::int32 layer);
    DAVA::int32 GetCurrentForceLayer() const;

    void CollectLODDataFromScene();

    inline bool GetAllSceneModeEnabled() const;
    void SetAllSceneModeEnabled(bool enabled);

protected:
    bool CheckSelectedContainsEntity(const DAVA::Entity *arg) const;

    void AddSelectedLODsRecursive(DAVA::Entity *entity);
    void RemoveSelectedLODsRecursive(DAVA::Entity *entity);

    void UpdateForceLayer();
    void UpdateForceDistance();
    void UpdateAllSceneModeEnabled();
    void UpdateForceData();
    void ResetForceState(DAVA::Entity *entity);
    void ResetForceState(DAVA::LodComponent *lodComponent);

	void Process(DAVA::float32 elapsedTime) override;

    inline const DAVA::List<DAVA::LodComponent *> GetCurrentLODs() const;

    DAVA::int32 CalculateForceLayer() const;
    DAVA::float32 CalculateForceDistance() const;

private:
    DAVA::int32 currentLodsLayersCount = 0;

    DAVA::int32 forceLayer = DAVA::LodComponent::INVALID_LOD_LAYER;
    DAVA::int32 allSceneForceLayer = DAVA::LodComponent::MAX_LOD_LAYERS;
    DAVA::float32 forceDistance = DAVA::LodComponent::MIN_LOD_DISTANCE;
    DAVA::float32 allSceneForceDistance = DAVA::LodComponent::INVALID_DISTANCE;

    std::array<DAVA::uint32, DAVA::LodComponent::MAX_LOD_LAYERS> lodTrianglesCount;
    std::array<DAVA::float32, DAVA::LodComponent::MAX_LOD_LAYERS> lodDistances;

    DAVA::UnorderedMap<DAVA::LodComponent*, ForceData> sceneLODs;
    DAVA::List<DAVA::LodComponent*> selectedLODs;

	std::vector<CreatePlaneLODCommandHelper::RequestPointer> planeLODRequests;

    bool forceDistanceEnabled = false;
    bool allSceneModeEnabled = false;
};

inline DAVA::uint32 EditorLODSystem::GetLayerTriangles(DAVA::uint32 layerNum) const
{
    return lodTrianglesCount[layerNum];
}

inline DAVA::float32 EditorLODSystem::GetLayerDistance(DAVA::uint32 layerNum) const
{
    return lodDistances[layerNum];
}

inline DAVA::uint32 EditorLODSystem::GetCurrentLodsLayersCount() const
{
    return currentLodsLayersCount;
}

inline bool EditorLODSystem::GetForceDistanceEnabled() const
{
    return forceDistanceEnabled;
}

inline DAVA::float32 EditorLODSystem::GetForceDistance() const
{
    return forceDistance;
}

inline DAVA::int32 EditorLODSystem::GetForceLayer() const
{
    return forceLayer;
}

inline bool EditorLODSystem::GetAllSceneModeEnabled() const
{
    return allSceneModeEnabled;
}

inline const DAVA::List<DAVA::LodComponent *> EditorLODSystem::GetCurrentLODs() const
{
    if (allSceneModeEnabled)
    {
        DAVA::List<DAVA::LodComponent *> lods;
        for (auto it = sceneLODs.begin(); it != sceneLODs.end(); ++it)
        {
            lods.push_back(it->first);
        }
        return lods;
    }

    return selectedLODs;
}

#endif // __SCENE_LOD_SYSTEM_H__
