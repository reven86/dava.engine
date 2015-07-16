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


#include "Scene3D/LodNode.h"
#include "Scene3D/Scene.h"
#include "Scene3D/SceneFileV2.h"
#include "Render/3D/StaticMesh.h"
#include "Render/RenderHelper.h"
#include "Utils/StringFormat.h"
#include "Scene3D/SceneNodeAnimation.h"
#include "Scene3D/SceneNodeAnimationList.h"

namespace DAVA 
{

LodNode::LodDistance::LodDistance()
{
    distance = nearDistance = nearDistanceSq = farDistance = farDistanceSq = (float32) INVALID_DISTANCE;
}
    
void LodNode::LodDistance::SetDistance(float32 newDistance)
{
    distance = newDistance;
}
    
void LodNode::LodDistance::SetNearDistance(float32 newDistance)
{
    nearDistance = newDistance;
    nearDistanceSq = nearDistance * nearDistance;
}

void LodNode::LodDistance::SetFarDistance(float32 newDistance)
{
    farDistance = newDistance;
    farDistanceSq = farDistance * farDistance;
}
    

    
const float32 LodNode::INVALID_DISTANCE = -1.f;
const float32 LodNode::MIN_LOD_DISTANCE = 0;
const float32 LodNode::MAX_LOD_DISTANCE = 500;


LodNode::LodNode()
:	Entity()
,   currentLod(NULL)
,   lastLodUpdateFrame(0)
,   forceLodLayer(INVALID_LOD_LAYER)
,   forceDistance(INVALID_DISTANCE)
,   forceDistanceSq(INVALID_DISTANCE)
    
{
    for(int32 iLayer = 0; iLayer < MAX_LOD_LAYERS; ++iLayer)
    {
        SetLodLayerDistance(iLayer, GetDefaultDistance(iLayer));
    }

//    lodLayersArray[0].SetDistance(0.0f);
    lodLayersArray[0].SetNearDistance(0.0f);
}
	
LodNode::~LodNode()
{
//    const List<LodData>::const_iterator & end = lodLayers.end();
//    for (List<LodData>::iterator it = lodLayers.begin(); it != end; ++it)
//    {
//        LodData & ld = *it;
//        size_t size = ld.materials.size();
//        for (size_t idx = 0; idx < size; ++idx)
//        {
//            SafeRelease(ld.materials[idx]);
//            SafeRelease(ld.meshes[idx]);
//        }
//    }
}

void LodNode::AddNodeInLayer(Entity * node, int32 layer)
{
    AddNode(node);
    RegisterNodeInLayer(node, layer);
}

LodNode::LodData	*LodNode::CreateNewLayer(int32 layer)
{
    LodData *ld = NULL;
    if (lodLayers.empty()) 
    {
        LodData d;
        d.layer = layer;
        lodLayers.push_back(d);
        ld = &(*lodLayers.begin());
        SetCurrentLod(ld);
    }
    else 
    {
        for (List<LodData>::iterator it = lodLayers.begin(); it != lodLayers.end(); it++)
        {
            if (it->layer == layer) 
            {
                ld = &(*it);
                return ld;
            }
            if (layer < it->layer)
            {
                LodData d;
                d.layer = layer;
                List<LodData>::iterator newIt = lodLayers.insert(it, d);
                ld = &(*newIt);
                return ld;
            }
        }

        LodData d;
        d.layer = layer;
        lodLayers.push_back(d);
        ld = &(*lodLayers.rbegin());
    }
    
    return ld;
}

void LodNode::RegisterNodeInLayer(Entity * node, int32 layer)
{
    LodData *ld = CreateNewLayer(layer);
    if (node->GetName().find("dummy") != String::npos) 
    {
        if (ld->nodes.empty()) 
        {
            ld->isDummy = true;
        }
    }
    else 
    {
        ld->isDummy = false;
    }

    ld->nodes.push_back(node);
    node->AddFlagRecursive(Entity::NODE_IS_LOD_PART);
    if (ld != currentLod) 
    {
        node->SetUpdatable(false);
    }
    
}
    
void LodNode::RegisterIndexInLayer(int32 nodeIndex, int32 layer)
{
    LodData *ld = CreateNewLayer(layer);
	if (nodeIndex >= 0)
	{
		ld->indexes.push_back(nodeIndex);
	}
}

void LodNode::RemoveNode(Entity * node)
{
    Entity::RemoveNode(node);
    List<LodData>::iterator ei = lodLayers.end();
    for (List<LodData>::iterator i = lodLayers.begin(); i != ei; i++) 
    {
        Vector<Entity*>::iterator eit = i->nodes.end();
        for (Vector<Entity*>::iterator it = i->nodes.begin(); it != eit; it++) 
        {
            if (*it == node) 
            {
                node->RemoveFlagRecursive(Entity::NODE_IS_LOD_PART);
                i->nodes.erase(it);
                
                return;
            }
        }
    }
}
    
void LodNode::RemoveAllChildren()
{
    Entity::RemoveAllChildren();
    RemoveFlagRecursive(Entity::NODE_IS_LOD_PART);
    lodLayers.clear();
    currentLod = NULL;
}
    
bool LodNode::IsLodMain(Entity *childToCheck)
{
    if (!childToCheck) 
    {
        return true;
    }
    if (childToCheck->GetName().find("dummy") != String::npos) 
    {
        return true;
    }
    
    List<LodData>::iterator ei = lodLayers.end();
    for (List<LodData>::iterator i = lodLayers.begin(); i != ei; i++) 
    {
        if(i->isDummy) 
        {
            continue;
        }
        Vector<Entity*>::iterator eit = i->nodes.end();
        for (Vector<Entity*>::iterator it = i->nodes.begin(); it != eit; it++) 
        {
            if (*it == childToCheck) 
            {
                return true;
            }
        }
        return false;
    }
    return false;
}

    
void LodNode::RecheckLod()
{
//#define LOD_DEBUG
    if (!currentLod)return;

    if (INVALID_LOD_LAYER != forceLodLayer) 
    {
        for (List<LodData>::iterator it = lodLayers.begin(); it != lodLayers.end(); it++)
        {
            if (it->layer >= forceLodLayer)
            {
                currentLod = &(*it);
                return;
            }
        }
            return;
    }
    
#ifdef LOD_DEBUG
    int32 cl = currentLod->layer;
#endif
    {
        float32 dst = 0.f;
        if(INVALID_DISTANCE == forceDistance)
        {
            if(scene->GetCurrentCamera())
            {
                dst = (scene->GetCurrentCamera()->GetPosition() - GetWorldTransform().GetTranslationVector()).SquareLength();
                dst *= scene->GetCurrentCamera()->GetZoomFactor() * scene->GetCurrentCamera()->GetZoomFactor();
            }
        }
        else 
        {
            dst = forceDistanceSq;
        }
        
        if (dst > GetLodLayerFarSquare(currentLod->layer) || dst < GetLodLayerNearSquare(currentLod->layer))
        {
            for (List<LodData>::iterator it = lodLayers.begin(); it != lodLayers.end(); it++)
            {
                if (dst >= GetLodLayerNearSquare(it->layer))
                {
                    currentLod = &(*it);
                }
                else 
                {
#ifdef LOD_DEBUG
                    if (cl != currentLod->layer) 
                    {
                        Logger::FrameworkDebug("Switch lod to %d", currentLod->layer);
                    }
#endif
                    return;
                }
            }
        }
    }
#ifdef LOD_DEBUG
    if (cl != currentLod->layer) 
    {
        Logger::FrameworkDebug("Switch lod to %d", currentLod->layer);
    }
#endif
}

void LodNode::SetCurrentLod(LodData *newLod)
{
    if (newLod != currentLod) 
    {
        if (currentLod) 
        {
            int32 size = static_cast<int32>(currentLod->nodes.size());
            for (int i = 0; i < size; i++) 
            {
                currentLod->nodes[i]->SetUpdatable(false);
            }
        }
        currentLod = newLod;
        int32 size = static_cast<int32>(currentLod->nodes.size());
        for (int i = 0; i < size; i++) 
        {
            currentLod->nodes[i]->SetUpdatable(true);
        }
    }
}


void LodNode::Update(float32 timeElapsed)
{
    
    if (flags&Entity::NODE_VISIBLE)
    {
        lastLodUpdateFrame++;
        if (lastLodUpdateFrame > RECHECK_LOD_EVERY_FRAME)
        {
            lastLodUpdateFrame = 0;
            LodData *oldLod = currentLod;
            RecheckLod();
            if (oldLod != currentLod) 
            {
                if (oldLod) 
                {
                    int32 size = static_cast<int32>(oldLod->nodes.size());
                    for (int i = 0; i < size; i++) 
                    {
                        oldLod->nodes[i]->SetUpdatable(false);
                    }
                }
                int32 size = static_cast<int32>(currentLod->nodes.size());
                for (int i = 0; i < size; i++) 
                {
                    currentLod->nodes[i]->SetUpdatable(true);
                }
            }
        }
    }
	else
	{
		lastLodUpdateFrame = RECHECK_LOD_EVERY_FRAME + 1;
	}
}

	
Entity* LodNode::Clone(Entity *dstNode)
{
    if (!dstNode) 
    {
		DVASSERT_MSG(IsPointerToExactClass<LodNode>(this), "Can clone only LodNode");
        dstNode = new LodNode();
    }
    
    Entity::Clone(dstNode);
    LodNode *nd = (LodNode *)dstNode;

    nd->lodLayers = lodLayers;
    int32 lodIdx = 0;// Don't ask me how it's works
	if(!nd->lodLayers.empty())
	{
		const List<LodData>::const_iterator &end = nd->lodLayers.end();
		nd->currentLod = &(*nd->lodLayers.begin());
		for (List<LodData>::iterator it = nd->lodLayers.begin(); it != end; ++it)
		{
			LodData & ld = *it;
			size_t size = ld.nodes.size();
			for (size_t idx = 0; idx < size; ++idx)
			{
				for (int i = 0; i < (int)children.size(); i++) 
				{
					if(children[i] == ld.nodes[idx])
					{
						ld.nodes[idx] = nd->children[i];
						if (nd->currentLod != &ld) 
						{
							ld.nodes[idx]->SetUpdatable(false);
						}
						else 
						{
							ld.nodes[idx]->SetUpdatable(true);
						}

						break;
					}
				}
			}
			lodIdx++;
		}
	}
    
	//Lod values
	for(int32 iLayer = 0; iLayer < MAX_LOD_LAYERS; ++iLayer)
	{
		nd->lodLayersArray[iLayer].distance = lodLayersArray[iLayer].distance;
		nd->lodLayersArray[iLayer].nearDistance = lodLayersArray[iLayer].nearDistance;
		nd->lodLayersArray[iLayer].nearDistanceSq = lodLayersArray[iLayer].nearDistanceSq;
		nd->lodLayersArray[iLayer].farDistance = lodLayersArray[iLayer].farDistance;
		nd->lodLayersArray[iLayer].farDistanceSq = lodLayersArray[iLayer].farDistanceSq;
	}

	nd->forceDistance = forceDistance;
	nd->forceDistanceSq = forceDistanceSq;
	nd->forceLodLayer = forceLodLayer;
    
	return dstNode;
}
    /**
     \brief virtual function to save node to KeyedArchive
     */
void LodNode::Save(KeyedArchive * archive, SerializationContext * serializationContext)
{
    Entity::Save(archive, serializationContext);
    archive->SetInt32("lodCount", (int32)lodLayers.size());
    
    int32 lodIdx = 0;
    const List<LodData>::const_iterator &end = lodLayers.end();
    for (List<LodData>::iterator it = lodLayers.begin(); it != end; ++it)
    {
        LodData & ld = *it;
        archive->SetInt32(Format("lod%d_layer", lodIdx), (int32)ld.layer);
        size_t size = ld.nodes.size();
        archive->SetInt32(Format("lod%d_cnt", lodIdx), (int32)size);
        for (size_t idx = 0; idx < size; ++idx)
        {
            for (int32 i = 0; i < (int32)children.size(); i++) 
            {
                if(children[i] == ld.nodes[idx])
                {
                    archive->SetInt32(Format("l%d_%d_ni", lodIdx, idx), i);
                    break;
                }
            }
        }
        
        archive->SetFloat(Format("lod%d_dist", lodIdx), GetLodLayerDistance(lodIdx));
        lodIdx++;
    }
}
    
    /**
     \brief virtual function to load node to KeyedArchive
     */
void LodNode::Load(KeyedArchive * archive, SerializationContext * serializationContext)
{
    Entity::Load(archive, serializationContext);

    int32 lodCount = archive->GetInt32("lodCount", 0);
    
    for (int32 lodIdx = 0; lodIdx < lodCount; ++lodIdx)
    {
        int32 layer = archive->GetInt32(Format("lod%d_layer", lodIdx), 0);
        size_t size = archive->GetInt32(Format("lod%d_cnt", lodIdx), 0);    //TODO: why size_t? int32?
		if (size > 0)
		{
			for (size_t idx = 0; idx < size; ++idx)
			{
				int32 index  = archive->GetInt32(Format("l%d_%d_ni", lodIdx, idx), 0);
				RegisterIndexInLayer(index, layer);
			}
		}
		else
		{
			RegisterIndexInLayer(-1, layer);
		}
        
        float32 distance = archive->GetFloat(Format("lod%d_dist", lodIdx), GetDefaultDistance(lodIdx));
        if(INVALID_DISTANCE == distance)
        {   // TemporaryFix. Remove it after all objects would be fixed.
            distance = GetDefaultDistance(lodIdx);
        }
        SetLodLayerDistance(lodIdx, distance);
    }
}
    
void LodNode::SceneDidLoaded()
{
    Entity::SceneDidLoaded();
    const List<LodData>::const_iterator &end = lodLayers.end();
    for (List<LodData>::iterator it = lodLayers.begin(); it != end; ++it)
    {
        LodData & ld = *it;
        size_t size = ld.indexes.size();
        for (size_t idx = 0; idx < size; ++idx)
        {
            ld.nodes.push_back(children[ld.indexes[idx]]);
//            if (&ld != currentLod) 
            {
                children[ld.indexes[idx]]->SetUpdatable(false);
            }
        }
    }

    currentLod = NULL;
    if (lodLayers.size() > 0)
    {
        SetCurrentLod(&(*lodLayers.rbegin()));
        lastLodUpdateFrame = 1000;
    }
    
}

int32 LodNode::GetMaxLodLayer()
{
	int32 ret = -1;
	const List<LodData>::const_iterator &end = lodLayers.end();
	for (List<LodData>::iterator it = lodLayers.begin(); it != end; ++it)
	{
		LodData & ld = *it;
		if(ld.layer > ret)
		{
			ret = ld.layer;
		}
	}

	return ret;
}

void LodNode::SetForceLodLayer(int32 layer)
{
    forceLodLayer = layer;
}
int32 LodNode::GetForceLodLayer()
{
    return forceLodLayer;
}

void LodNode::SetLodLayerDistance(int32 layerNum, float32 distance)
{
    DVASSERT(0 <= layerNum && layerNum < MAX_LOD_LAYERS);
    
    if(INVALID_DISTANCE != distance)
    {
        float32 nearDistance = distance * 0.95f;
        float32 farDistance = distance * 1.05f;
        
        if(GetLodLayersCount() - 1 == layerNum)
        {
            lodLayersArray[layerNum].SetFarDistance(MAX_LOD_DISTANCE * 2);
        }
        if(layerNum)
        {
            lodLayersArray[layerNum-1].SetFarDistance(farDistance);
        }

		lodLayersArray[layerNum].SetDistance(distance);
        lodLayersArray[layerNum].SetNearDistance(nearDistance);
    }
    else 
    {
        lodLayersArray[layerNum].SetDistance(distance);
    }
}
    
float32 LodNode::GetDefaultDistance(int32 layer)
{
    float32 distance = MIN_LOD_DISTANCE + ((float32)(MAX_LOD_DISTANCE - MIN_LOD_DISTANCE) / (MAX_LOD_LAYERS-1)) * layer;
    return distance;
}

void LodNode::SetForceLodLayerDistance(float32 newForceDistance)
{
    forceDistance = newForceDistance;
    forceDistanceSq = forceDistance * forceDistance;
}

float32 LodNode::GetForceLodLayerDistance()
{
    return forceDistance;
}
    
void LodNode::GetLodData(List<LodData*> &retLodLayers)
{
    retLodLayers.clear();
    
    List<LodData>::const_iterator endIt = lodLayers.end();
    for(List<LodData>::iterator it = lodLayers.begin(); it != endIt; ++it)
    {
        LodData *ld = &(*it);
        retLodLayers.push_back(ld);
    }
}

    
};
