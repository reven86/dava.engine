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


#ifndef __DAVAENGINE_MATERIAL_H__
#define __DAVAENGINE_MATERIAL_H__


#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Scene3D/DataNode.h"

#include "FileSystem/FilePath.h"
#include "Base/FastName.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{


class UberShader;
class Shader;
class Texture;    
class SceneFileV2;
class Light;
class PolygonGroup;
class RenderDataObject;
    
    
// TODO: move Material to Scene3D

struct StaticLightingParams
{
	Color transparencyColor;

	StaticLightingParams() : transparencyColor(0, 0, 0, 0) {}
	virtual ~StaticLightingParams() {}

	INTROSPECTION(StaticLightingParams,
	MEMBER(transparencyColor, "Transparency Color", I_SAVE | I_VIEW | I_EDIT))
};

class InstanceMaterialState : public BaseObject
{
    static const int32 LIGHT_NODE_MAX_COUNT = 4;
	static const int32 LIGHTMAP_SIZE_DEFAULT = 128; 
protected:
    virtual ~InstanceMaterialState();
public:
    InstanceMaterialState();

	virtual void Save(KeyedArchive * archive, SerializationContext *serializationContext);
	virtual void Load(KeyedArchive * archive, SerializationContext *serializationContext);
    
    void SetLight(int32 lightIndex, Light * lightNode);
    Light * GetLight(int32 lightIndex);
    
    void SetLightmap(Texture * texture, const FilePath & lightmapName);
    void SetUVOffsetScale(const Vector2 & uvOffset, const Vector2 uvScale);
    const Vector2 & GetUVScale() const { return uvScale; };
    const Vector2 & GetUVOffset() const { return uvOffset; };

	int32 GetLightmapSize();
	void SetLightmapSize(int32 size);

    inline Texture * GetLightmap() const;
	inline const FilePath & GetLightmapName() const;
    
    void SetFlatColor(const Color & color);
    const Color & GetFlatColor();
    
    void SetTextureShift(const Vector2 & speed);
    const Vector2 & GetTextureShift();

	void ClearLightmap();
	
	InstanceMaterialState * Clone();
    void InitFromState(const InstanceMaterialState * state);

private:
    Texture * lightmapTexture;
    FilePath lightmapName;
	int32 lightmapSize;
    Vector2 uvOffset;
    Vector2 uvScale;
    Color flatColor;
    Vector2 texture0Shift;

    
    Light * lightNodes[LIGHT_NODE_MAX_COUNT];
    
    friend class Material;
public:
    INTROSPECTION_EXTEND(InstanceMaterialState, BaseObject,
                         //MEMBER(lightmapTexture, "Texture:", INTROSPECTION_EDITOR)
//                         MEMBER(lightmapName, "Lightmap Name:", INTROSPECTION_EDITOR)
                         MEMBER(uvOffset, "UV Offset", I_VIEW | I_EDIT)
                         MEMBER(uvScale, "UV Scale", I_VIEW | I_EDIT)
                         MEMBER(lightmapSize, "Lightmap Size", I_VIEW | I_EDIT)
                         
                         PROPERTY("flatColor", "Flat Color (works only if flat color enabled)", GetFlatColor, SetFlatColor, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("texture0Shift", "Texture Shift", GetTextureShift, SetTextureShift, I_SAVE | I_VIEW | I_EDIT)
                         
                         //MEMBER(aabbox, "AABBox", INTROSPECTION_EDITOR)
                         );
};
    
    
class Material : public DataNode
{
	friend class SerializationContext;
	
public:
    enum eType
    {
        // Normal Materials
        MATERIAL_UNLIT_TEXTURE = 0,                 // texture
        MATERIAL_UNLIT_TEXTURE_DETAIL,              // texture * detail texture * 2.0
        MATERIAL_UNLIT_TEXTURE_DECAL,               // texture * decal 
        MATERIAL_UNLIT_TEXTURE_LIGHTMAP,            // texture * lightmap
        
        MATERIAL_VERTEX_LIT_TEXTURE,                // single texture with vertex lighting
        MATERIAL_VERTEX_LIT_DETAIL,                 // single texture * detail texture * 2.0 with vertex lighting
        MATERIAL_VERTEX_LIT_DECAL,
        MATERIAL_VERTEX_LIT_LIGHTMAP,               // vertex lit lighting + lightmaps
        
        MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE,
        MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR,     // single texture + diffuse light normal mapping
        MATERIAL_PIXEL_LIT_NORMAL_DIFFUSE_SPECULAR_MAP, // single texture + diffuse light normal mapping

		MATERIAL_VERTEX_COLOR_ALPHABLENDED,
        MATERIAL_FLAT_COLOR,
		
		MATERIAL_SKYBOX,

		MATERIAL_VERTEX_COLOR_ALPHABLENDED_FRAME_BLEND,
        
        MATERIAL_SPEED_TREE_LEAF,

        // MATERIAL_TEXTURE, 
        // MATERIAL_LIGHTMAPPED_TEXTURE,   
        // MATERIAL_VERTEX_LIGHTING,       // flag
        // MATERIAL_NORMAL_MAPPED,         // flag
        MATERIAL_TYPES_COUNT
    };
    
	enum eViewOptions
	{
		MATERIAL_VIEW_TEXTURE_LIGHTMAP = 0,
		MATERIAL_VIEW_LIGHTMAP_ONLY,
		MATERIAL_VIEW_TEXTURE_ONLY,

		MATERIAL_VIEW_COUNT
	};

    
    /*
        Plan of supported materials:
            Default per vertex lighting material
            Default per pixel lighting material
            Default deferred lighting material
     */
    
    enum eMaterialGeneratorInput
    {
        MATERIAL_INPUT_DIFFUSE = (1 << 1),
        MATERIAL_INPUT_SPECULAR = (1 << 2),
        MATERIAL_INPUT_SPECULAR_POWER = (1 << 3),
        MATERIAL_INPUT_NORMAL = (1 << 4),
        MATERIAL_INPUT_OPACITY = (1 << 5),
        MATERIAL_INPUT_OPACITY_MASK = (1 << 6),
        MATERIAL_INPUT_EMISSIVE = (1 << 7),
    };
    
    static const char8 * GetTypeName(eType type);

protected:
    virtual ~Material();
public:
    Material();
    
    Material * Clone(Material *newMaterial = NULL);
	void CopySettings(Material *fromMaterial);
    
    enum eValidationResult
    {
        VALIDATE_COMPATIBLE = 1,
        VALIDATE_INCOMPATIBLE,
    };
    
    eValidationResult Validate(PolygonGroup * polygonGroup);
    
    virtual void SetScene(Scene * _scene);
   
	virtual int32 Release();
    
    void SetType(eType _type);
    
    void SetAlphatest(bool alphatest);
    bool GetAlphatest();

	void SetAlphablend(bool isAlphablend);
	bool GetAlphablend();
    
    // TODO: remove const reference
    void EnableFlatColor(const bool & isEnabled);
    const bool & IsFlatColorEnabled();
    
    void SetWireframe(bool isWireframe);
    bool GetWireframe();
    
    void SetFog(const bool & _fogEnabled);
    const bool & IsFogEnabled() const;
    void SetFogDensity(float32 _fogDensity);
    float32 GetFogDensity() const;
    void SetFogColor(const Color & _fogColor);
    const Color & GetFogColor() const;
    
	void SetViewOption(eViewOptions option);
	eViewOptions GetViewOption();
    
    void SetTwoSided(bool _isTwoSided);
    bool GetTwoSided();
    
    void SetAmbientColor(const Color & color);
    void SetDiffuseColor(const Color & color);
    void SetSpecularColor(const Color & color);
    void SetEmissiveColor(const Color & color);

    const Color & GetAmbientColor() const;
    const Color & GetDiffuseColor() const;
    const Color & GetSpecularColor() const;
    const Color & GetEmissiveColor() const;
    
    void SetShininess(float32 shininess);
    float32 GetShininess() const;

	void SetSetupLightmap(bool isSetupLightmap);
    bool GetSetupLightmap() const;
    
    
    void EnableTextureShift(const bool & isEnabled);
    const bool & IsTextureShiftEnabled();


    /**
        \brief Bind material to render system.
        Function should be used if you want to render something with this material.
     */
    //void BindMaterial();
	void PrepareRenderState(InstanceMaterialState * instanceMaterialState = 0, Matrix4 * worldMxPtr = 0);
    void Draw(PolygonGroup * group, InstanceMaterialState * state, Matrix4 * worldMxPtr);
    
    // TODO: remove const &
    const bool & IsExportOwnerLayerEnabled();
    void SetExportOwnerLayer(const bool & isEnabled);
    const FastName & GetOwnerLayerName();
    void SetOwnerLayerName(const FastName & fastname);

    
    
//    eType   type; //TODO: waiting for enums at introspection
    uint32 type;
	eViewOptions viewOptions;

	Vector4 reflective;
	float32	reflectivity;

	Vector4 transparent;
	float	transparency; 
	float	indexOfRefraction;


    enum eTextureLevel
    {
        TEXTURE_DIFFUSE = 0,
        TEXTURE_DETAIL = 1,
        TEXTURE_DECAL = 1,
		TEXTURE_LIGHTMAP = 1,
		TEXTURE_NORMALMAP = 2,
        
        TEXTURE_COUNT, 
    };    

    void Save(KeyedArchive * keyedArchive, SerializationContext * serializationContext);
    void Load(KeyedArchive * keyedArchive, SerializationContext * serializationContext);
    
    
    //void SetTextureSlotName(uint32 index, const String & string);

    uint32 GetTextureSlotCount() const;
    const String & GetTextureSlotName(uint32 index) const;
    uint32 GetTextureSlotIndexByName(const String & string) const;
    
    
    void SetTexture(eTextureLevel level, Texture * texture);
    void SetTexture(eTextureLevel level, const FilePath & textureName);
	inline Texture * GetTexture(eTextureLevel level) const;
	inline const FilePath & GetTextureName(eTextureLevel level) const;

	//RenderState * GetRenderState();
    
    inline void SetBlending(eBlending blending);
	
	inline void SetStaticLightingParams(StaticLightingParams * params);    
	inline StaticLightingParams * GetStaticLightingParams() const;
    
    const String& GetName() const;
    void SetName(const String& materialName);
    
private:
    void RetrieveTextureSlotNames();
    
    
    Texture * textures[TEXTURE_COUNT];
    Vector<FilePath> names;
    
    String textureSlotNames[TEXTURE_COUNT];
    uint32 textureSlotCount;


	eBlending blending;	

    void RebuildShader();
    
    bool    isTranslucent;
    bool    isTwoSided;

	bool	isSetupLightmap;
    
    float32	shininess;
    
    Color ambientColor;
	Color diffuseColor;
	Color specularColor;
	Color emissiveColor;
    Color treeLeafColor;

    float32 treeLeafOcclusionOffset;
    float32 treeLeafOcclusionMul;

    bool    isFogEnabled;
    float32 fogDensity;
    Color   fogColor;
    
	StaticLightingParams * lightingParams;

	bool isAlphablend;
    bool isFlatColorEnabled;
    
    bool isTexture0ShiftEnabled;
    
    bool isWireframe;
    
    Shader  * shader;
    
    int32 uniformTexture0;
    int32 uniformTexture1;
    int32 uniformLightPosition0;
    int32 uniformMaterialLightAmbientColor;
    int32 uniformMaterialLightDiffuseColor;
    int32 uniformMaterialLightSpecularColor;
    int32 uniformMaterialSpecularShininess;
    int32 uniformLightIntensity0;
    int32 uniformLightAttenuationQ;
	int32 uniformUvOffset;
	int32 uniformUvScale;
    int32 uniformFogDensity;
    int32 uniformFogColor;
    int32 uniformFlatColor;
    int32 uniformTexture0Shift;
    int32 uniformWorldTranslate;
    int32 uniformWorldScale;
    int32 uniformTreeLeafColorMul;
    int32 uniformTreeLeafOcclusionOffset;

	//RenderState renderStateBlock;
    
    String name;
    
    /*
        TODO: Uniform array, with set of all uniforms, with one set.
     */
    bool isExportOwnerLayerEnabled;
    FastName ownerLayerName;
    
    static UberShader * uberShader;
    
public:
    
    INTROSPECTION_EXTEND(Material, DataNode,
		MEMBER(lightingParams, "Static Lighting Params", I_SAVE | I_VIEW | I_EDIT)

        MEMBER(isTranslucent, "Is Translucent", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(isTwoSided, "Is Two Sided", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(isSetupLightmap, "Is Setup Lightmap", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(shininess, "Shininess", I_SAVE | I_VIEW | I_EDIT)

        MEMBER(ambientColor, "Ambient Color", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(diffuseColor, "Diffuse Color", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(specularColor, "Specular Color", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(emissiveColor, "Emissive Color", I_SAVE | I_VIEW | I_EDIT)

        MEMBER(treeLeafColor, "Tree Leaf Color", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(treeLeafOcclusionOffset, "Tree Leaf Occlusion Offset", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(treeLeafOcclusionMul, "Tree Leaf Occlusion Multiply", I_SAVE | I_VIEW | I_EDIT)

        PROPERTY("isFogEnabled", "Is Fog Enabled", IsFogEnabled, SetFog, I_SAVE | I_VIEW | I_EDIT)
        MEMBER(fogDensity, "Fog Density", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(fogColor, "Fog Color", I_SAVE | I_VIEW | I_EDIT)
                         
        MEMBER(isAlphablend, "Is Alphablended", I_SAVE | I_VIEW | I_EDIT)
        
        PROPERTY("isFlatColorEnabled", "Is flat color enabled", IsFlatColorEnabled, EnableFlatColor, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("isTexture0ShiftEnabled", "Is texture shift enabled", IsTextureShiftEnabled, EnableTextureShift, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("isExportOwnerLayerEnabled", "Is export owner layer enabled. (Export layer settings to render batch on set)", IsExportOwnerLayerEnabled, SetExportOwnerLayer, I_SAVE)
        PROPERTY("ownerLayerName", "Owner layer name", GetOwnerLayerName, SetOwnerLayerName, I_SAVE | I_VIEW)
                                         
        MEMBER(isWireframe, "Is Wire Frame", I_SAVE | I_VIEW | I_EDIT)
		MEMBER(type, "Type", I_SAVE | I_VIEW | I_EDIT)
                         
        COLLECTION(names, "Names", I_SAVE | I_VIEW | I_EDIT)

		//MEMBER(renderStateBlock, "Render State", I_VIEW | I_EDIT)
    );
};

Texture * Material::GetTexture(eTextureLevel level) const
{
	DVASSERT(level < TEXTURE_COUNT);
	return textures[level];
}

inline const FilePath & Material::GetTextureName(eTextureLevel level) const
{
	DVASSERT(level < TEXTURE_COUNT);
	return names[level];
}

inline void Material::SetBlending(eBlending _blending)
{
    blending = _blending;
}

inline void Material::SetStaticLightingParams(StaticLightingParams * params)
{
	SafeDelete(lightingParams);
	lightingParams = params;
}

inline StaticLightingParams * Material::GetStaticLightingParams() const
{
	return lightingParams;
}

inline Texture * InstanceMaterialState::GetLightmap() const
{
    return lightmapTexture;
}

inline const FilePath & InstanceMaterialState::GetLightmapName() const
{
    return lightmapName;
}

    
};


#endif // __DAVAENGINE_MATERIAL_H__

