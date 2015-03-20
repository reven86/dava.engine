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

class Command2;
class SceneEditor2;
class EntityGroup;

class EditorLODSystem : public DAVA::SceneSystem
{
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

    inline DAVA::int32 GetForceLayer() const;
    void SetForceLayer(DAVA::int32 layer);

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

    DAVA::int32 currentLodsLayersCount;
    bool forceDistanceEnabled;
    DAVA::float32 forceDistance;
    DAVA::int32 forceLayer;    
    bool allSceneModeEnabled;

    std::array<DAVA::uint32, DAVA::LodComponent::MAX_LOD_LAYERS> lodTrianglesCount;
    std::array<DAVA::float32, DAVA::LodComponent::MAX_LOD_LAYERS > lodDistances;

    DAVA::Deque<DAVA::LodComponent *> sceneLODs;
    DAVA::Deque<DAVA::LodComponent *> selectedLODs;
    inline const DAVA::Deque<DAVA::LodComponent *> &GetCurrentLODs() const;
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

inline const DAVA::Deque<DAVA::LodComponent *> &EditorLODSystem::GetCurrentLODs() const
{
    return allSceneModeEnabled ? sceneLODs : selectedLODs;
}

#endif // __SCENE_LOD_SYSTEM_H__
