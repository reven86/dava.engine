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


#ifndef __DAVAENGINE_MESH_INSTANCE_H__
#define __DAVAENGINE_MESH_INSTANCE_H__

#include "Scene3D/Entity.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/RenderBatch.h"
#include "FileSystem/FilePath.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA 
{
class Scene;
class StaticMesh;
class Texture;
class SceneFileV2;
class PolygonGroup;
class MeshInstanceNode;
class Light;
class NMaterial;

class PolygonGroupWithMaterial : public RenderBatch
{
protected:
    virtual ~PolygonGroupWithMaterial();

public:
    void Setup(StaticMesh* mesh, int32 polygroupIndex, NMaterial* material, TransformComponent* transform);

    virtual uint64 GetSortID();

    StaticMesh * GetMesh();
    int32 GetPolygroupIndex();
    PolygonGroup * GetPolygonGroup();

private:
    friend class MeshInstanceNode;

    StaticMesh* mesh = nullptr;
    TransformComponent* transform = nullptr;
    int32 polygroupIndex = 0;
};

    
class MeshInstanceNode : public Entity
{
public:	
	struct LightmapData
	{
		Texture * lightmap;
		FilePath lightmapName;
		Vector2 uvOffset;
		Vector2 uvScale;
	};
protected:
	~MeshInstanceNode();
public:
	MeshInstanceNode();

    void AddPolygonGroup(StaticMesh* mesh, int32 polygonGroupIndex, NMaterial* material);

    virtual void Update(float32 timeElapsed);
    virtual void Draw();
    virtual uint64 GetSortID();
    
    uint32 GetRenderBatchCount();
    RenderBatch * GetRenderBatch(uint32 batchIndex);

	inline const AABBox3 & GetBoundingBox() const;
    inline const AABBox3 & GetWorldTransformedBox() const; 
	
    Vector<PolygonGroupWithMaterial*> & GetPolygonGroups();
    	
    virtual Entity* Clone(Entity *dstNode = NULL);
    
    //Returns maximum Bounding Box as WorlTransformedBox
    virtual AABBox3 GetWTMaximumBoundingBoxSlow();

	
    /**
        \brief virtual function to save node to KeyedArchive
     */
    virtual void Save(KeyedArchive * archive, SerializationContext * serializationContext);
    
    /**
        \brief virtual function to load node to KeyedArchive
     */
	virtual void Load(KeyedArchive * archive, SerializationContext * serializationContext);

	/**
        \brief Add lightmap texture.
        Consequent calls of this function will add lightmaps to consequent PolygonGroups of this MeshInstance.
            
        Normal usage (pseudocode):
        \code
        ClearLightmaps();
        for(each polygon group)
        {
            AddLightmap(...);
        }
        \endcode

        \param[in] lightmapName path to texture
	 */
	void AddLightmap(int32 polygonGroupIndex, const LightmapData & lightmapData);

	/**
		\brief Same as previous function, but polygonGroupIndex is calculated as polygonGroupIndex=currentPolygonGroupCount.
	*/
	void AddLightmap(const LightmapData & lightmapData);

	/**
        \brief Delete all lightmaps for this MeshInstance. 
	 */
	void ClearLightmaps();

	bool HasLightmaps();

	void CreateDynamicShadowNode();
	void DeleteDynamicShadowNode();
	void ConvertToShadowVolume();
    
    virtual void GetDataNodes(Set<DataNode*> & dataNodes);


	LightmapData * GetLightmapDataForIndex(int32 index);
    int32 GetLightmapCount();

    /**
        \brief Bake transformations into polygon groups if polygon groups single instanced.
     */
    virtual void BakeTransforms();
    
    /**
        \brief Register nearest node to this MeshInstanceNode.
        MeshInstance can have own criteria of detection on which light nodes are interesting for this particular mesh and which are not.
     */
    virtual void RegisterNearestLight(Light * node);

	Vector<PolygonGroupWithMaterial*> polygroups;

protected:
    virtual void UpdateLights();
    
//    virtual Entity* CopyDataTo(Entity *dstNode);
    
    
    Vector<Light*> nearestLights;
    
    
	AABBox3 bbox;
    AABBox3 transformedBox;
    
	Vector<LightmapData> lightmaps;

};
	
inline const AABBox3 & MeshInstanceNode::GetBoundingBox() const
{
	return bbox;
}
inline const AABBox3 & MeshInstanceNode::GetWorldTransformedBox() const
{
    return transformedBox;
}

};

#endif // __DAVAENGINE_MESH_INSTANCE_H__
