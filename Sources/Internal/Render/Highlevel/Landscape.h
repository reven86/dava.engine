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


#ifndef __DAVAENGINE_LANDSCAPE_NODE_H__
#define __DAVAENGINE_LANDSCAPE_NODE_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Render/RenderBase.h"
#include "Scene3D/Entity.h"
#include "Render/Highlevel/Frustum.h"
#include "Render/Highlevel/RenderObject.h"

#include "FileSystem/FilePath.h"

#include "Scene3D/SceneFile/SerializationContext.h"

#include "MemoryManager/MemoryProfiler.h"

//#define LANDSCAPE_SPECULAR_LIT 1

namespace DAVA
{

class Scene;
class Image;
class Texture;
class Shader;
class SceneFileV2;
class Heightmap;
class NMaterial;
    
template<class T>
class LandQuadTreeNode
{
public:
    LandQuadTreeNode()
    {
        children = 0;
        parent = 0;
        for (int32 k = 0; k < 4; ++k)
            neighbours[k] = 0;
    }
    ~LandQuadTreeNode()
    {
        ReleaseChildren();
    }
    
    void AllocChildren()
    {
        ReleaseChildren();
        children = new LandQuadTreeNode[4];
    }
    
    void ReleaseChildren()
    {
        SafeDeleteArray(children);
    }
    
    LandQuadTreeNode * children;  // It's array of 4 child nodes
    LandQuadTreeNode * parent;
    LandQuadTreeNode * neighbours[4]; 
    T data;
};
    
template <class T>
class LinearQuadTree
{
public:
    
};

using CircularIndexBufferArray = CircularArray<rhi::HIndexBuffer, 3>;

/**    
    \brief Implementation of cdlod algorithm to render landscapes
    This class is base of the landscape code on all platforms
    Landscape node is always axial aligned for simplicity of frustum culling calculations
    Keep in mind that landscape orientation cannot be changed using localTransform and worldTransform matrices. 
 */ 

class FoliageSystem;
class NMaterial;
class Landscape : public RenderObject
{
    DAVA_ENABLE_CLASS_ALLOCATION_TRACKING(ALLOC_POOL_LANDSCAPE)

public:
    enum 
    {
        LEFT = 0,
        RIGHT = 1,
        TOP = 2,
        BOTTOM = 3,
    };
	Landscape();
	virtual ~Landscape();

    const static FastName PARAM_TEXTURE_TILING;
    const static FastName PARAM_TILE_COLOR0;
    const static FastName PARAM_TILE_COLOR1;
    const static FastName PARAM_TILE_COLOR2;
    const static FastName PARAM_TILE_COLOR3;

    const static FastName TEXTURE_COLOR;
    const static FastName TEXTURE_TILE;
    const static FastName TEXTURE_TILEMASK;
    const static FastName TEXTURE_SPECULAR;

    /**
        \brief Set lod coefficients for dynamic roam landscape
        Default values: (60, 120, 240, 480)
        Every next value should be almost twice higher than previous to avoid gaps between levels
     */
    void SetLods(const Vector4 & lods);
    
    /**
        \brief Builds landscape from heightmap image and bounding box of this landscape block
        \param[in] landscapeBox axial-aligned bounding box of the landscape block
     */
    virtual void BuildLandscapeFromHeightmapImage(const FilePath & heightmapPathname, const AABBox3 & landscapeBox);

	//TODO: think about how to switch normal generation for landscape on/off
	//ideally it should be runtime option and normal generaiton should happen when material that requires landscape has been set
	class LandscapeVertex
	{
	public:
		Vector3 position;
		Vector2 texCoord;
#ifdef LANDSCAPE_SPECULAR_LIT
		Vector3 normal;
        Vector3 tangent;
#endif
	};

    struct LanscapeBufferData
    {
        rhi::HVertexBuffer buffer;
        LandscapeVertex *data;
        uint32 dataSize;

    };

    void PrepareToRender(Camera * camera) override;

	/**
        \brief Get landscape mesh geometry.
        Unoptimized lod0 mesh is returned.
        \param[out] vertices landscape vertices
        \param[out] indices landscape indices
	 */
    bool GetGeometry(Vector<LandscapeVertex> & vertices, Vector<int32> & indices) const;
    
    /**
        \brief Function to receive pathname of heightmap object
        \returns pathname of heightmap
     */
    const FilePath & GetHeightmapPathname();
	void SetHeightmapPathname(const FilePath & newPath);
	
	float32 GetLandscapeSize() const;
	void SetLandscapeSize(float32 newSize);

	float32 GetLandscapeHeight() const;
	void SetLandscapeHeight(float32 newHeight);
    
    void GetDataNodes(Set<DataNode*> & dataNodes) override;

    void Save(KeyedArchive * archive, SerializationContext * serializationContext);
    void Load(KeyedArchive * archive, SerializationContext * serializationContext);
    
    // TODO: Need comment here
	bool PlacePoint(const Vector3 & point, Vector3 & result, Vector3 * normal = 0) const;
	Vector3 GetPoint(int16 x, int16 y, uint16 height) const;

    Heightmap *GetHeightmap();
    virtual void SetHeightmap(Heightmap *height);
    
    NMaterial * GetMaterial();
    void SetMaterial(NMaterial * material);

	virtual RenderObject * Clone(RenderObject *newObject);
    virtual void RecalcBoundingBox();

	int32 GetDrawIndices() const;
	
    void SetFoliageSystem(FoliageSystem* _foliageSystem);

	// RHI_COMPLETE need remove this
    void UpdatePart(Heightmap* fromHeightmap, const Rect2i & rect);
    
	using LandscapeThumbnailCallback = DAVA::Function<void(Landscape*, Texture*)>;
    void CreateLandscapeTexture(LandscapeThumbnailCallback callback);

protected:
	static const int32 TEXTURE_SIZE_FULL_TILED = 2048;
	static const int32 RENDER_QUAD_WIDTH = 129;
	static const int32 RENDER_QUAD_AND = RENDER_QUAD_WIDTH - 2;
	static const int32 INDEX_ARRAY_COUNT = 10000 * 6; // 10k triangles max

    struct LandscapeQuad
    {
        AABBox3 bbox;
        int16 x = 0;
		int16 y = 0;
        int16 size = 0;
        int16 rdoQuad = -1;
        uint32 frame = 0;
		uint8 startClipPlane = 0;
        int8 lod = 0;
    };
    
    //RHI_COMPLETE need remove this
    void CollectNodesRecursive(LandQuadTreeNode<LandscapeQuad> * currentNode, int16 nodeSize,
                               Vector< LandQuadTreeNode<LandscapeQuad> * > & nodes);

    void RecursiveBuild(LandQuadTreeNode<LandscapeQuad> * currentNode, int32 level, int32 maxLevels);
    LandQuadTreeNode<LandscapeQuad> * FindNodeWithXY(LandQuadTreeNode<LandscapeQuad> * currentNode, int16 quadX, int16 quadY, int16 quadSize);
    void FindNeighbours(LandQuadTreeNode<LandscapeQuad> * currentNode);
    void MarkFrames(LandQuadTreeNode<LandscapeQuad> * currentNode, int32 & depth);

    void GenLods(LandQuadTreeNode<LandscapeQuad> * currentNode, uint8 clippingFlags, Camera * camera);
    void GenQuad(LandQuadTreeNode<LandscapeQuad> * currentNode, int8 lod);
    void GenFans();
    
    int16 AllocateQuadVertexBuffer(LandscapeQuad * quad);
    void AllocateGeometryData();
    void ReleaseGeometryData();

    void RestoreGeometry();
    
    void SetLandscapeSize(const Vector3 & newSize);

    void FlushQueue();
    void ClearQueue();
    bool BuildHeightmap();
    void BuildLandscape();

	void OnCreateLandscapeTextureCompleted(rhi::HSyncObject);
	void UnregisterCreateTextureCallback();

	void UpdateNodeChildrenBoundingBoxesRecursive(LandQuadTreeNode<LandscapeQuad>& root, Heightmap* fromHeightmap);

private:
    LandQuadTreeNode<LandscapeQuad> quadTreeHead;
    Vector<LandQuadTreeNode<LandscapeQuad>*> fans;
    Vector<LandQuadTreeNode<LandscapeQuad>*> lod0quads;
    Vector<LandQuadTreeNode<LandscapeQuad>*> lodNot0quads;
    Vector<rhi::HVertexBuffer> vertexBuffers;
    Vector<LanscapeBufferData> bufferRestoreData;
    rhi::HIndexBuffer currentIndexBuffer;
    CircularIndexBufferArray indexBuffers;
    FilePath heightmapPath;

    Frustum *frustum = nullptr;
    Heightmap *heightmap = nullptr;
	NMaterial* landscapeMaterial = nullptr;
    FoliageSystem* foliageSystem = nullptr;

	Texture* thumbnailRenderTarget = nullptr;
	LandscapeThumbnailCallback createdLandscapeTextureCallback;
    
    uint16* indices = nullptr;
    uint16* queueDrawIndices = nullptr;
    float32 lodDistance[8];
    float32 lodSqDistance[8];

    uint32 vertexLayoutUID = 0;
    int32 lodLevelsCount = 0;
    int32 allocatedMemoryForQuads = 0;
    int32 queueIndexCount = 0;
    int32 queueIndexOffset = 0;
    int32 flushQueueCounter = 0;
    int32 nearLodIndex = 0;
    int32 farLodIndex = 0;
	uint32 drawIndices = 0;
    int16 queueRdoQuad = 0;

public:
   
    INTROSPECTION_EXTEND(Landscape, RenderObject,
        PROPERTY("heightmapPath", "Height Map Path", GetHeightmapPathname, SetHeightmapPathname, I_VIEW | I_EDIT)
        PROPERTY("size", "Size", GetLandscapeSize, SetLandscapeSize, I_VIEW | I_EDIT)
        PROPERTY("height", "Height", GetLandscapeHeight, SetLandscapeHeight, I_VIEW | I_EDIT)
		);
};

};

#endif // __DAVAENGINE_LANDSCAPE_NODE_H__
