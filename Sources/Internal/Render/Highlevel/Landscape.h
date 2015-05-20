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
#include "Render/Highlevel/LandscapeCursor.h"
#include "Render/Highlevel/RenderObject.h"

#include "FileSystem/FilePath.h"

#include "Scene3D/SceneFile/SerializationContext.h"

//#define LANDSCAPE_SPECULAR_LIT 1

namespace DAVA
{

class Scene;
class Image;
class Texture;
class RenderDataObject;
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
    
    /**
        \brief Set lod coefficients for dynamic roam landscape
        Default values: (60, 120, 240, 480)
        Every next value should be almost twice higher than previous to avoid gaps between levels
     */
    void SetLods(const Vector4 & lods);
    
    
    const static FastName PARAM_TILE_COLOR0;
	const static FastName PARAM_TILE_COLOR1;
	const static FastName PARAM_TILE_COLOR2;
	const static FastName PARAM_TILE_COLOR3;

    
    /**
        \brief Builds landscape from heightmap image and bounding box of this landscape block
        \param[in] landscapeBox axial-aligned bounding box of the landscape block
     */
    virtual void BuildLandscapeFromHeightmapImage(const FilePath & heightmapPathname, const AABBox3 & landscapeBox);
    
    enum eTextureLevel
    {
        TEXTURE_COLOR = 0,  // in case of BLENDED_SHADER in alpha channel it can be tile mask for TILED_TEXTURES
        TEXTURE_TILE_MASK,
        TEXTURE_TILE0,   
        TEXTURE_TILE1,
        TEXTURE_TILE2,
        TEXTURE_TILE3,
        // TEXTURE_BUMP,
        
        TEXTURE_DETAIL, 
        
        TEXTURE_TILE_FULL, 
        
        TEXTURE_COUNT
    };

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
    
    /**
        \brief Set texture for the specific texture level
        
     To render landscape you need to set textures.  
        For RENDERING_MODE_TEXTURE you need to set only TEXTURE_TEXTURE0.
        For RENDERING_MODE_DETAIL_SHADER you have to set TEXTURE_TEXTURE0 and TEXTURE_DETAIL
        For RENDERING_MODE_BLENDED_SHADER you have to set TEXTURE_TEXTURE0, TEXTURE_TEXTURE1, TEXTURE_TEXTUREMASK
          
        \param[in] level level of texture you want to set
        \param[in] textureName name of texture you want to open and set to specific level
     */
    void SetTexture(eTextureLevel level, const FilePath & textureName);

    
    /**
     \brief Set texture for the specific texture level
     
     To render landscape you need to set textures.  
     For RENDERING_MODE_TEXTURE you need to set only TEXTURE_TEXTURE0.
     For RENDERING_MODE_DETAIL_SHADER you have to set TEXTURE_TEXTURE0 and TEXTURE_DETAIL
     For RENDERING_MODE_BLENDED_SHADER you have to set TEXTURE_TEXTURE0, TEXTURE_TEXTURE1, TEXTURE_TEXTUREMASK
     
     \param[in] level level of texture you want to set
     \param[in] texture you want to set to specific level
     */
    void SetTexture(eTextureLevel level, Texture * texture);

	/**
        \brief Get texture that was previously set in SetTexture.
        \param[in] level 
        \returns current texture
	 */
	virtual Texture * GetTexture(eTextureLevel level);
    
	/**
        \brief Get texture name that was previously set in SetTexture.
        \param[in] level level of texture you want to get name
        \returns current texture name
	 */
    const FilePath & GetTextureName(eTextureLevel level);

	/**
        \brief Set texture name for export.
        \param[in] level level of texture you want to set name
        \param[in] newTextureName new texture name
	 */
    void SetTextureName(eTextureLevel level, const FilePath &newTextureName);
    
    
	/**
        \brief Set tiling for specific texture level.
        This function gives you can control of tiling for specific landscape level.
     */    
    void SetTextureTiling(eTextureLevel level, const Vector2 & tiling);

    /**
        \brief Get tiling for specified texture level.
        \returns Tiling for specified texture level.
     */
    Vector2 GetTextureTiling(eTextureLevel level); 

    void SetTileColor(eTextureLevel level, const Color & color);
    Color GetTileColor(eTextureLevel level);

    /**
        \brief Overloaded draw function to draw landscape
     */
	virtual void Draw(Camera * camera);

	/**
        \brief Get landscape mesh geometry.
        Unoptimized lod0 mesh is returned.
        \param[out] vertices landscape vertices
        \param[out] indices landscape indices
	 */
	void GetGeometry(Vector<LandscapeVertex> & vertices, Vector<int32> & indices);
    
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
    
    void Create(NMaterial *fromMaterial = NULL);
    void Save(KeyedArchive * archive, SerializationContext * serializationContext);
    void Load(KeyedArchive * archive, SerializationContext * serializationContext);

    CC_DEPRECATED(void LoadFog(KeyedArchive * archive, SerializationContext * serializationContext));
    CC_DEPRECATED(void LoadMaterialProps(KeyedArchive * archive, SerializationContext * serializationContext));
    
    // TODO: Need comment here
	bool PlacePoint(const Vector3 & point, Vector3 & result, Vector3 * normal = 0) const;
	Vector3 GetPoint(int16 x, int16 y, uint16 height);

	void CursorEnable();
	void CursorDisable();

    Heightmap *GetHeightmap();
    virtual void SetHeightmap(Heightmap *height);
    
//    virtual void UpdateFullTiledTexture();
//    FilePath SaveFullTiledTexture();
    Texture *CreateLandscapeTexture();
    LandscapeCursor *GetCursor();
    
	virtual RenderObject * Clone(RenderObject *newObject);

	int32 GetDrawIndices() const;
	
    void SetFoliageSystem(FoliageSystem* _foliageSystem);

protected:
	
	const static FastName PARAM_CAMERA_POSITION;
	const static FastName PARAM_TEXTURE0_TILING;
	const static FastName PARAM_TEXTURE1_TILING;
	const static FastName PARAM_TEXTURE2_TILING;
	const static FastName PARAM_TEXTURE3_TILING;
	const static FastName PARAM_PROP_SPECULAR_COLOR;
	const static FastName PARAM_SPECULAR_SHININESS;
	const static FastName TEXTURE_SPECULAR_MAP;
	const static FastName TECHNIQUE_TILEMASK_NAME;
    
    class LandscapeQuad
    {
    public:
        LandscapeQuad()
        {
            x = y = size = lod = 0;
            rdoQuad = -1;
            frame = 0;
			startClipPlane = 0;
        }
        
        int16   x, y;
        //int16   xbuf, ybuf;
        int16   size;
        int8    lod;
        int16   rdoQuad;
        AABBox3 bbox;
		uint8 startClipPlane;
        uint32  frame;
    };
   
    static const int32 RENDER_QUAD_WIDTH = 129;
    static const int32 RENDER_QUAD_AND = RENDER_QUAD_WIDTH - 2;
    static const int32 INDEX_ARRAY_COUNT = RENDER_QUAD_WIDTH * RENDER_QUAD_WIDTH * 6;
    

    void RecursiveBuild(LandQuadTreeNode<LandscapeQuad> * currentNode, int32 level, int32 maxLevels);
    LandQuadTreeNode<LandscapeQuad> * FindNodeWithXY(LandQuadTreeNode<LandscapeQuad> * currentNode, int16 quadX, int16 quadY, int16 quadSize);
    void FindNeighbours(LandQuadTreeNode<LandscapeQuad> * currentNode);
    void MarkFrames(LandQuadTreeNode<LandscapeQuad> * currentNode, int32 & depth);

    void BindMaterial(int32 lodLayer, Camera* camera);
    void UnbindMaterial();
    
    void DrawQuad(LandQuadTreeNode<LandscapeQuad> * currentNode, int8 lod);
    void Draw(LandQuadTreeNode<LandscapeQuad> * currentNode, uint8 clippingFlags);
    void DrawFans();

    Texture * CreateTexture(eTextureLevel level, const FilePath & textureName);
    
    int16 AllocateRDOQuad(LandscapeQuad * quad);
    void ReleaseAllRDOQuads();

	int GetMaxLod(float32 quadDistance);
	float32 GetQuadToCameraDistance(const Vector3& camPos, const LandscapeQuad& quad);
	
	void SetupMaterialProperties();
	
	void SetSpecularColor(const Color& color);
	Color GetSpecularColor();
	void SetSpecularShininess(const float32& shininess);
	float32 GetSpecularShininess();
	void SetSpecularMapPath(const FilePath& path);
	FilePath GetSpecularMapPath();
    
    void SetLandscapeSize(const Vector3 & newSize);
	
    Vector<LandscapeVertex *> landscapeVerticesArray;
    Vector<RenderDataObject *> landscapeRDOArray;
    
    uint16 * indices;
    //Texture * textures[TEXTURE_COUNT];
    //Vector<FilePath> textureNames;
    
    int32 lodLevelsCount;
    float32 lodDistance[8]; //
    float32 lodSqDistance[8];
    
    LandQuadTreeNode<LandscapeQuad> quadTreeHead;

    Vector<LandQuadTreeNode<LandscapeQuad>*> fans;
    
    int32 allocatedMemoryForQuads;
    
    Vector3 cameraPos;
    Frustum *frustum;
    
    ePrimitiveType primitypeType;
    
    //Vector2 textureTiling[TEXTURE_COUNT];
    //Color tileColor[TEXTURE_COUNT];
    
	LandscapeCursor * cursor;
        
    int16 queueRdoQuad;
    int32 queueRenderCount;
    uint16 * queueDrawIndices;
    
    void FlushQueue();
    void ClearQueue();
    
    bool BuildHeightmap();
    void BuildLandscape();
    Heightmap *heightmap;
    FilePath heightmapPath;
    
	static const uint32 TEXTURE_TILE_FULL_SIZE = 2048;
    
    Vector<LandQuadTreeNode<LandscapeQuad> *>lod0quads;
    Vector<LandQuadTreeNode<LandscapeQuad> *>lodNot0quads;

    int32 prevLodLayer;
    
    int32 flashQueueCounter;
    
    int32 nearLodIndex;
    int32 farLodIndex;
    
	NMaterial* tileMaskMaterial;
	//NMaterial* fullTiledMaterial;
	//NMaterial* currentMaterial;
	
	uint32 drawIndices;
	
	void SetDefaultValues();

    FoliageSystem* foliageSystem;

public:
   
    INTROSPECTION_EXTEND(Landscape, RenderObject,
        PROPERTY("heightmapPath", "Height Map Path", GetHeightmapPathname, SetHeightmapPathname, I_VIEW | I_EDIT)
        PROPERTY("size", "Size", GetLandscapeSize, SetLandscapeSize, I_VIEW | I_EDIT)
        PROPERTY("height", "Height", GetLandscapeHeight, SetLandscapeHeight, I_VIEW | I_EDIT)
		);
};

};

#endif // __DAVAENGINE_LANDSCAPE_NODE_H__





