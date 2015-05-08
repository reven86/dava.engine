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

#define LANDSCAPE_SPECULAR_LIT 1

#define PATCH_VERTEX_COUNT 17
#define PATCH_QUAD_COUNT (PATCH_VERTEX_COUNT - 1)
#define MAX_LANDSCAPE_SUBDIV_LEVELS 9
#define MAX_QUAD_COUNT_IN_VBO 128


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
    
    // TODO: Remove functions to work with texture through landscape
    
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

    DAVA_DEPRECATED(void LoadFog(KeyedArchive * archive, SerializationContext * serializationContext));
    DAVA_DEPRECATED(void LoadMaterialProps(KeyedArchive * archive, SerializationContext * serializationContext));
    
    // TODO: Need comment here
	bool PlacePoint(const Vector3 & point, Vector3 & result, Vector3 * normal = 0) const;
	Vector3 GetPoint(int16 x, int16 y, uint16 height);

	void CursorEnable();
	void CursorDisable();

    Heightmap *GetHeightmap();
    virtual void SetHeightmap(Heightmap *height);
    

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
    

    struct PatchQuadInfo
    {
        uint32 rdoQuad;
        AABBox3 bbox;
        Vector3 positionOfMaxError;
        float32 maxError;
    };
    
    struct SubdivisionPatchInfo
    {
        enum
        {
            CLIPPED = 1,
            DRAW = 2,
            SUBDIVIDED = 3,
            TERMINATED = 4,
        };
        
        SubdivisionPatchInfo()
        {
            subdivisionState = CLIPPED;
            startClipPlane = 0;
        }
        
        uint32 lastSubdividedSize;
        uint32 subdivisionState;
        uint8 startClipPlane;
    };
    
    struct SubdivisionLevelInfo
    {
        uint32 offset;
        uint32 size;
    };
    
    uint32 subdivLevelCount;
    uint32 subdivPatchCount;
    uint32 rdoQuadWidth;
    
    SubdivisionLevelInfo subdivLevelInfoArray[MAX_LANDSCAPE_SUBDIV_LEVELS];
    PatchQuadInfo * patchQuadArray;
    SubdivisionPatchInfo * subdivPatchArray;
    Vector<SubdivisionPatchInfo*> drawPatchArray;
    
    uint32 GetQuadPosition(uint32 level, uint32 x, uint32 y);
    PatchQuadInfo * GetQuad(uint32 level, uint32 x, uint32 y);
    SubdivisionPatchInfo * GetSubdivPatch(uint32 level, uint32 x, uint32 y);
    
    void UpdatePatchInfo(uint32 level, uint32 x, uint32 y);
    void SubdividePatch(uint32 level, uint32 x, uint32 y, uint8 clippingFlags);
    void TerminateSubdivision(uint32 level, uint32 x, uint32 y, uint32 lastSubdividedSize);
    void DrawPatch(uint32 level, uint32 x, uint32 y,
                   uint32 xNegSize, uint32 xPosSize, uint32 yNegSize, uint32 yPosSize);
    void AddPatchToRenderNoInstancing(uint32 level, uint32 x, uint32 y);
    void DrawNoInstancing();
    void DrawLandscape();
    uint16 GetXYIndex(uint16 x, uint16 y);

    
    
    uint8 testMatrix[32][32];

    
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
    
    LandscapeQuad * rdoArray;
    
    static const int32 RENDER_QUAD_WIDTH = 129;
    static const int32 RENDER_QUAD_AND = RENDER_QUAD_WIDTH - 2;
    static const int32 INDEX_ARRAY_COUNT = RENDER_QUAD_WIDTH * RENDER_QUAD_WIDTH * 6 * 2;
    
    void BindMaterial(Camera* camera);
    //Texture * CreateTexture(eTextureLevel level, const FilePath & textureName);
    
    int16 AllocateRDOQuad(LandscapeQuad * quad);
    void ReleaseLandscape();

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

    
    Camera * camera;
    Vector3 cameraPos;
    float32 fovCorrection;
    Frustum *frustum;
    
	LandscapeCursor * cursor;
    
    // Render Queue
    int16 queueRdoQuad;
    int32 queueRenderCount;
    uint16 * queueDrawIndices;
    uint32 drawIndices;
    int32 flushQueueCounter;
    
    void FlushQueue();
    void ClearQueue();
    
    // Heightmap
    bool BuildHeightmap();
    void ReallocateLandscape();
    Heightmap *heightmap;
    FilePath heightmapPath;
    
    // Material
	static const uint32 TEXTURE_TILE_FULL_SIZE = 2048;
    
	NMaterial* tileMaskMaterial;
	void SetDefaultValues();

    FoliageSystem* foliageSystem;
    
    float32 defaultFov;
    
    float32 solidAngleError;
    float32 geometryAngleError;
    float32 absHeightError;
    
    float32 zoomSolidAngleError;
    float32 zoomGeometryAngleError;
    float32 zoomAbsHeightError;
    
    float32 fovSolidAngleError;
    float32 fovGeometryAngleError;
    float32 fovAbsHeightError;
    
    bool    isDebugDraw;
public:
   
    INTROSPECTION_EXTEND(Landscape, RenderObject,
        PROPERTY("heightmapPath", "Height Map Path", GetHeightmapPathname, SetHeightmapPathname, I_VIEW | I_EDIT)
        PROPERTY("size", "Size", GetLandscapeSize, SetLandscapeSize, I_VIEW | I_EDIT)
        PROPERTY("height", "Height", GetLandscapeHeight, SetLandscapeHeight, I_VIEW | I_EDIT)
        MEMBER(solidAngleError, "solidAngleError", I_VIEW | I_EDIT)
        MEMBER(geometryAngleError, "geometryAngleError", I_VIEW | I_EDIT)
        MEMBER(absHeightError, "absHeightError", I_VIEW | I_EDIT)

        MEMBER(zoomSolidAngleError, "solidAngleError", I_VIEW | I_EDIT)
        MEMBER(zoomGeometryAngleError, "geometryAngleError", I_VIEW | I_EDIT)
        MEMBER(zoomAbsHeightError, "absHeightError", I_VIEW | I_EDIT)
        MEMBER(isDebugDraw, "isDebugDraw", I_VIEW | I_EDIT)
		);
};

};

#endif // __DAVAENGINE_LANDSCAPE_NODE_H__





