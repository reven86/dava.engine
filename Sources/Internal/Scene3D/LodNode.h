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


#ifndef __DAVAENGINE_LOD_NODE_H__
#define __DAVAENGINE_LOD_NODE_H__

#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA 
{
class Scene;
class StaticMesh;
class Texture;
class SceneFileV2;
    
class LodNode : public Entity
{
    static const int32 RECHECK_LOD_EVERY_FRAME = 3;
    
public:

    static const int32 MAX_LOD_LAYERS = 4;
    static const int32 INVALID_LOD_LAYER = -1;
    static const float32 MIN_LOD_DISTANCE;
    static const float32 MAX_LOD_DISTANCE;
	static const float32 INVALID_DISTANCE;

    struct LodDistance
    {
        float32 distance;

        float32 nearDistance;
        float32 farDistance;
        
        float32 nearDistanceSq;
        float32 farDistanceSq;
        
        LodDistance();
        void SetDistance(float32 newDistance);
        void SetNearDistance(float32 newDistance);
        void SetFarDistance(float32 newDistance);
    };
    
    struct LodData
    {
        LodData()
        :layer(INVALID_LOD_LAYER)
        ,isDummy(false)
        {
        }
        Vector<Entity*> nodes;
        Vector<int32> indexes;
        int32 layer;
        bool isDummy;
    };
protected:
	virtual ~LodNode();
public:
	LodNode();
	
    
    virtual void	AddNodeInLayer(Entity * node, int32 layer);//adds new node and registers this node as a LOD layer
    virtual void	RegisterNodeInLayer(Entity * node, int32 layer);//register existing node as a layer
	virtual void	RemoveNode(Entity * node);
	virtual void	RemoveAllChildren();
    
    virtual void Update(float32 timeElapsed);
    void SimpleUpdate(float32 timeElapsed);
	
    virtual Entity* Clone(Entity *dstNode = NULL);
    /**
        \brief virtual function to save node to KeyedArchive
     */
    virtual void Save(KeyedArchive * archive, SerializationContext * serializationContext);
    
    /**
        \brief virtual function to load node to KeyedArchive
     */
	virtual void Load(KeyedArchive * archive, SerializationContext * serializationContext);
    
    void SetCurrentLod(LodData *newLod);

    virtual void SceneDidLoaded();
    
    virtual bool IsLodMain(Entity *childToCheck = NULL);//if childToCheck is NULL checks the caller node

	int32 GetMaxLodLayer();


    /**
     \brief Sets lod layer thet would be forcely used in the whole scene.
     \param[in] layer layer to set on the for the scene. Use -1 to disable forced lod layer.
	 */
    void SetForceLodLayer(int32 layer);
    int32 GetForceLodLayer();

    /**
     \brief Sets distance that will be used for lod instead of layer
     \param[in] newForceDistance distance to set on foк lod node. Use -1 to disable forced lod layer distance.
	 */
    void SetForceLodLayerDistance(float32 newForceDistance);
    float32 GetForceLodLayerDistance();

    
    /**
     \brief Registers LOD layer into the LodNode.
     \param[in] layerNum is the layer index
     \param[in] distance near view distance for the layer
	 */
    void SetLodLayerDistance(int32 layerNum, float32 distance);
    
    
    inline int32 GetLodLayersCount();
    inline float32 GetLodLayerDistance(int32 layerNum);
    inline float32 GetLodLayerNear(int32 layerNum);
    inline float32 GetLodLayerFar(int32 layerNum);
    inline float32 GetLodLayerNearSquare(int32 layerNum);
    inline float32 GetLodLayerFarSquare(int32 layerNum);

    static float32 GetDefaultDistance(int32 layer);

    void GetLodData(List<LodData*> &retLodLayers);
    
protected:
//    virtual Entity* CopyDataTo(Entity *dstNode);

    virtual void	RegisterIndexInLayer(int32 nodeIndex, int32 layer);
    virtual LodData	*CreateNewLayer(int32 layer);

    void RecheckLod();
    
    LodData *currentLod;
    List<LodData> lodLayers;
    
    
    int lastLodUpdateFrame;

    LodDistance lodLayersArray[MAX_LOD_LAYERS];
    int32 forceLodLayer;
    float32 forceDistance;
    float32 forceDistanceSq;
};
    
int32 LodNode::GetLodLayersCount()
{
	return (int32)lodLayers.size();
}

float32 LodNode::GetLodLayerDistance(int32 layerNum)
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    return lodLayersArray[layerNum].distance;
}

float32 LodNode::GetLodLayerNear(int32 layerNum)
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    return lodLayersArray[layerNum].nearDistance;
}

float32 LodNode::GetLodLayerFar(int32 layerNum)
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    return lodLayersArray[layerNum].farDistance;
}

float32 LodNode::GetLodLayerNearSquare(int32 layerNum)
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    return lodLayersArray[layerNum].nearDistanceSq;
}

float32 LodNode::GetLodLayerFarSquare(int32 layerNum)
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    return lodLayersArray[layerNum].farDistanceSq;
}
	
};

#endif // __DAVAENGINE_MESH_INSTANCE_H__
