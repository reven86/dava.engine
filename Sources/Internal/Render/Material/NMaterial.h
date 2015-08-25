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


#ifndef __DAVAENGINE_NMATERIAL_H__
#define __DAVAENGINE_NMATERIAL_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/HashMap.h"
#include "Base/FastNameMap.h"
#include "Scene3D/DataNode.h"
#include "Render/RenderManager.h"
#include "Render/RenderState.h"
#include "Render/Material/NMaterialConsts.h"
#include "Render/Material/NMaterialTemplate.h"
#include "Render/Shader.h"
#include "Base/Introspection.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Render/Material/RenderTechnique.h"
#include "Render/Highlevel/RenderFastNames.h"

#include "Render/Material/NMaterialStateDynamicTexturesInsp.h"
#include "Render/Material/NMaterialStateDynamicFlagsInsp.h"
#include "Render/Material/NMaterialStateDynamicPropertiesInsp.h"
#include "Render/Material/NMaterialHelper.h"

namespace DAVA
{

class UberShader;
class Texture;    
class SceneFileV2;
class Light;
class PolygonGroup;
class RenderDataObject;
class Light;
class MaterialCompiler;
class MaterialGraph;
class NMaterial;
struct NMaterialTemplate;
	
struct IlluminationParams : public InspBase
{
    static const int32 LIGHTMAP_SIZE_DEFAULT = 128;
    
    bool isUsed;
    bool castShadow;
    bool receiveShadow;
    int32 lightmapSize;

    //this is a weak property since IlluminationParams exists only as a part of parent material
    NMaterial* parent;

    inline IlluminationParams(NMaterial* parentMaterial = NULL);
 
    inline IlluminationParams(const IlluminationParams & params);
    
    inline int32 GetLightmapSize() const;
    void SetLightmapSize(const int32 &size);
    
    void SetParent(NMaterial* parentMaterial);
    inline NMaterial* GetParent() const;

    inline INTROSPECTION(IlluminationParams,
        MEMBER(isUsed, "Use Illumination", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(castShadow, "Cast Shadow", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(receiveShadow, "Receive Shadow", I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("lightmapSize", "Lightmap Size", GetLightmapSize, SetLightmapSize, I_SAVE | I_VIEW | I_EDIT)
        );
};

class NMaterialProperty
{
public:
    Shader::eUniformType type;
    uint8 size;
    uint8* data;
	
	inline NMaterialProperty();
	
	inline ~NMaterialProperty();
	
	inline NMaterialProperty* Clone();
};

class Camera;
class SerializationContext;

/**
 \ingroup materials
 \brief This class represents render material. Relation between render batches
 and materials is 1-to-1.
 
 Each material consists of:
 
 -  render technique. Render technique contains of set of passes. Each pass
 represents shader used for rendering, render state for the shader,
 set of render layers where render batch with the given material will be rendered
 when the pass is active. Techniques are loaded from a persistent
 description stored on the file system.
 Material render state loaded with the technique may be modified at runtime.
 This process is called subclassing.
 
 -  set of flags. Flags alter shader of the technique and allow to add or
 remove shader functionality at runtime (for example, turn fog on/off).
 
 -  set of properties. Each property corresponds to the uniform required by the shader.
 Material may contain properties that has no direct mapping to uniforms.
 
 -  set of textures. Each texture corresponds to sampler required by the shader.
 Material may contain textures that has no direct mapping to samplers.
 
 Each material obeys to settings of quality system and uses technique depending
 of the rules described for different levels of the quality. Material loads textures
 for the given quality level but doesn't unload them when quality level were downgraded.
 Set of flags and properties is shared between all quality levels.
 
 Materials may be organized in hierarchical tree structure. Each node of hierarchy
 may contain its own set of flags, properties, textures and have its own technique.
 All these parameters are inherited by subnodes so material at the tree leaf
 will inherit and provide to its shader all properties, textures and flags
 from its predecessors even if they were not specified in the subnode directly.
 Subnode at any level of tree hierarchy may override parent value.
 Material searches values for textures, properties, flags in the following order:
 self -> parent -> parent of parent -> etc...
 
 Currently there are 3 material types:
 -   GLOBAL material type. This is virtual root for all materials in the scene.
 All default values and most commont properties are set here.
 -   MATERIAL material type. This is middle level. All common flags, textures and properties are set here.
 -   INSTANCE material type. This is material contained by render batches.
 Properties and textures specific per render batch are set here.
 
 */
    
class NMaterial : public DataNode
{
	friend class MaterialSystem;
	friend class MaterialCompiler;
	friend class NMaterialHelper;
    
    friend class NMaterialStateDynamicTexturesInsp;
    friend class NMaterialStateDynamicFlagsInsp;
    friend class NMaterialStateDynamicPropertiesInsp;
	
public:
    static const FastName TEXTURE_ALBEDO;
    static const FastName TEXTURE_NORMAL;
    static const FastName TEXTURE_DETAIL;
    static const FastName TEXTURE_LIGHTMAP;
	static const FastName TEXTURE_DECAL;
	static const FastName TEXTURE_CUBEMAP;
    static const FastName TEXTURE_HEIGHTMAP;
    static const FastName TEXTURE_DECALMASK;
    static const FastName TEXTURE_DECALTEXTURE;
    
    static const FastName TEXTURE_DYNAMIC_REFLECTION;
    static const FastName TEXTURE_DYNAMIC_REFRACTION;
	
	static const FastName PARAM_LIGHT_POSITION0;
	static const FastName PARAM_PROP_AMBIENT_COLOR;
	static const FastName PARAM_PROP_DIFFUSE_COLOR;
	static const FastName PARAM_PROP_SPECULAR_COLOR;
	static const FastName PARAM_LIGHT_AMBIENT_COLOR;
	static const FastName PARAM_LIGHT_DIFFUSE_COLOR;
	static const FastName PARAM_LIGHT_SPECULAR_COLOR;
	static const FastName PARAM_LIGHT_INTENSITY0;
	static const FastName PARAM_MATERIAL_SPECULAR_SHININESS;
    static const FastName PARAM_FOG_LIMIT;
	static const FastName PARAM_FOG_COLOR;
	static const FastName PARAM_FOG_DENSITY;
    static const FastName PARAM_FOG_START;
    static const FastName PARAM_FOG_END;
    static const FastName PARAM_FOG_ATMOSPHERE_COLOR_SUN;
    static const FastName PARAM_FOG_ATMOSPHERE_COLOR_SKY;
    static const FastName PARAM_FOG_ATMOSPHERE_SCATTERING;
    static const FastName PARAM_FOG_ATMOSPHERE_DISTANCE;
    static const FastName PARAM_FOG_HALFSPACE_HEIGHT;
    static const FastName PARAM_FOG_HALFSPACE_DENSITY;
    static const FastName PARAM_FOG_HALFSPACE_FALLOFF;
    static const FastName PARAM_FOG_HALFSPACE_LIMIT;
	static const FastName PARAM_FLAT_COLOR;
	static const FastName PARAM_TEXTURE0_SHIFT;
	static const FastName PARAM_UV_OFFSET;
	static const FastName PARAM_UV_SCALE;
    static const FastName PARAM_LIGHTMAP_SIZE;
    static const FastName PARAM_SHADOW_COLOR;
    static const FastName PARAM_DECAL_TILE_SCALE;
    static const FastName PARAM_DECAL_TILE_COLOR;
    static const FastName PARAM_DETAIL_TILE_SCALE;
    static const FastName PARAM_RCP_SCREEN_SIZE;
    static const FastName PARAM_SCREEN_OFFSET;
    static const FastName PARAM_NORMAL_SCALE;
    static const FastName PARAM_ALPHATEST_THRESHOLD;
    
	static const FastName FLAG_VERTEXFOG;
	static const FastName FLAG_FOG_LINEAR;
    static const FastName FLAG_FOG_HALFSPACE;
    static const FastName FLAG_FOG_HALFSPACE_LINEAR;
	static const FastName FLAG_FOG_ATMOSPHERE;
	static const FastName FLAG_TEXTURESHIFT;
	static const FastName FLAG_TEXTURE0_ANIMATION_SHIFT;
	static const FastName FLAG_WAVE_ANIMATION;
	static const FastName FLAG_FAST_NORMALIZATION;    
    static const FastName FLAG_TILED_DECAL_MASK;
	static const FastName FLAG_FLATCOLOR;
    static const FastName FLAG_DISTANCEATTENUATION;
    static const FastName FLAG_SPECULAR;
    static const FastName FLAG_SEPARATE_NORMALMAPS;

    static const FastName FLAG_SPHERICAL_LIT;

    static const FastName FLAG_TANGENT_SPACE_WATER_REFLECTIONS;
    
    static const FastName FLAG_DEBUG_UNITY_Z_NORMAL;
    static const FastName FLAG_DEBUG_Z_NORMAL_SCALE;
    static const FastName FLAG_DEBUG_NORMAL_ROTATION;

    static const FastName FLAG_SKINNING;
    
	static const FastName FLAG_LIGHTMAPONLY;
	static const FastName FLAG_TEXTUREONLY; //VI: this flag is for backward compatibility with old materials. See FLAG_ALBEDOONLY
	static const FastName FLAG_SETUPLIGHTMAP;
    static const FastName FLAG_VIEWALBEDO;
    static const FastName FLAG_VIEWAMBIENT;
    static const FastName FLAG_VIEWDIFFUSE;
    static const FastName FLAG_VIEWSPECULAR;

    static const FastName FLAG_ALPHATESTVALUE;
	
	static const FastName DEFAULT_QUALITY_NAME;
	
	enum eMaterialType
	{
		MATERIALTYPE_NONE = 0,
		MATERIALTYPE_MATERIAL = 1,
		MATERIALTYPE_INSTANCE = 2,
        MATERIALTYPE_GLOBAL = 3
	};
	
	enum eFlagValue
	{
		FlagOff = 0,
		FlagOn = 1,
		
        FlagInherited = 2 //VI: this bit is set for flags inherited from the parent
	};
	
    enum eDynamicBindFlags
    {
        DYNAMIC_BIND_LIGHT = 1 << 0,
        DYNAMIC_BIND_OBJECT_CENTER = 1 << 1
    };

public:
	
	NMaterial();
	
    /**
	 \brief Returns parent material node in the hierarchy or NULL.
	 \returns parent material.
	 */
	inline NMaterial* GetParent() const;
	
	/**
	 \brief Sets parent material in the hierarchy.
     Parent may not be the same as the target.
     Parent may be NULL.
	 \param[in] newParent pointer to the parent material
	 \param[in] inheritTemplate use true if you want target material to inherit template from the parent.
	 */
	void SetParent(NMaterial* newParent, bool inheritTemplate = true);
    
    /**
	 \brief Returns number of child materials in the hierarchy.
	 \returns number of child materials.
	 */
	inline uint32 GetChildrenCount() const;
	
    /**
	 \brief Returns material child by its index.
     Index must be in a range between 0 and number of child materials.
     \param[in] index child index
	 \returns child material.
	 */
	inline NMaterial* GetChild(uint32 index) const;
			
	//{TODO: these should be removed and changed to a generic system
	//setting properties via special setters
	inline uint8 GetDynamicBindFlags() const;
	//}END TODO

    /**
    \brief Returns using material flags.
    \param[in] reference vector
    */
    inline void GetFlags(Vector<FastName> &flagsCollection) const;

    /**
	 \brief Renders given polygon group with the current material.
     \param[in] polygonGroup polygon group to render.
	 */
    void Draw(PolygonGroup * polygonGroup);
    
    /**
	 \brief Renders given render data object and index array with the current material.
     \param[in] renderData render data object with set of geometry attribute streams.
     \param[in] indices array of indices to render.
     \param[in] indexCount number of indices to render.
	 */
	void Draw(RenderDataObject* renderData, uint16* indices = NULL, uint16 indexCount = 0);
	
    /**
	 \brief Sets flag to the material affectively altering its shaders by define
     corresponding to the flag.
     \param[in] flag name of the flag. This value will be passed to the shader during compilation.
     \param[in] flagValue value of the flag (on/off). Determines if the given flag name will be included to the set of defines.
	 */
	void SetFlag(const FastName& flag, eFlagValue flagValue);
    
    /**
	 \brief Removes flag from the current material flag collection so flag value will be inherited from the parent.
     \param[in] flag name of the flag. This value will be passed to the shader during compilation.
	 */
    void ResetFlag(const FastName& flag);
    
    /**
	 \brief Returns effective value of the given flag in relation to the given material.
     Takes into account flag inheritance.
     \param[in] flag name of the flag. This value will be passed to the shader during compilation.
	 \returns flag value. FlagInherited bit will be set for values inherited from the parent.
	 */
	int32 GetFlagValue(const FastName& flag) const;
    
    /**
	 \brief Allows to check if the flag affects given material.
     Takes into account flag inheritance.
     \param[in] flag name of the flag. This value will be passed to the shader during compilation.
	 \returns true if the flag is effective for the material.
	 */
    bool IsFlagEffective(const FastName& flag) const;
    
    /**
	 \brief Binds render technique using pass name and camera.
     Binding means:
     - shader uniforms will be set
     - textures will be set
     - render state will be set
     \param[in] passName name of the render pass.
     \param[in] camera active camera.
	 */
	void BindMaterialTechnique(const FastName & passName, Camera* camera);
    
    /**
	 \brief Returns set of flags representing vertex format required by the material.
     That set of flags corresponds to shader attributes.
	 \returns set of flags representing vertex format required by the material.
	 */
	inline uint32 GetRequiredVertexFormat() const;
	
    /**
	 \brief Saves material to the keyed archive.
     \param[in] archive object to save material to.
     \param[in] serializationContext serialization context.
	 */
	virtual void Save(KeyedArchive * archive, SerializationContext * serializationContext);
    
    /**
	 \brief Loads material from the keyed archive.
     \param[in] archive object to load material from.
     \param[in] serializationContext serialization context.
	 */
	virtual void Load(KeyedArchive * archive, SerializationContext * serializationContext);
	
	/**
	 \brief Sets desired quality level for the material.
     This method just updates variable value and does nothing more.
     \param[in] stateName name of quality level.
	 */
	void SetQuality(const FastName& stateName);
	
	/**
	 \brief Applies desired quality level for the material.
     This method updates the material using quality
     level set by SetQuality.
     \param[in] force use true to reload material even if the wuality level remained the same.
	 */
	bool ReloadQuality(bool force = false);
	
    /**
	 \brief Creates copy of the material.
     */
	NMaterial* Clone();
    
    /**
	 \brief Creates copy of the material with new name.
     \param[in] newName name of new material.
	 */
	NMaterial* Clone(const String& newName);
	
    /**
	 \brief Returns data container used by art pipeline tools.
     Creates new object if it was not inialized yet.
	 \returns data container used by art pipeline tools.
	 */
    IlluminationParams * GetIlluminationParams(bool createIfNeeded = true);
    
    /**
	 \brief Deletes data container used by art pipeline tools.
     It's safe to call this method if there is no such object created.
     */
    void ReleaseIlluminationParams();
	
	/**
	 \brief Removes texture from the material by sampler name.
     If texture with the sampler name exists in the parent material
     it will become effective for the material.
     \param[in] textureFastName sampler name.
	 */
    void RemoveTexture(const FastName& textureFastName);

    /**
	 \brief Set texture by sampler name.
     When you set texture by path it will be loaded only after it became active in the current material quality.
     SetTexture("cubemap", "~res:/cubemap.pvr") will not result in GetTexture("cubemap") returning a valid texture object.
     A valid texture object will be returned only if there's actually uniform named "cubemap" in the material shader.
     \param[in] textureFastName sampler name.
     \param[in] texturePath path to texture file.
	 */
    void SetTexture(const FastName& textureFastName, const FilePath& texturePath);
    
    /**
	 \brief Set texture path by sampler name.
     This method leaves texture intact.
     Allows to manipulate FBO that has to be saved to some path.
     \param[in] textureFastName sampler name.
     \param[in] texturePath path to texture file.
	 */
    void SetTexturePath(const FastName& textureFastName, const FilePath& texturePath);
    
    /**
	 \brief Set texture by sampler name.
     This method doesn't check for uniform in the material and always uses texture provided.
     \param[in] textureFastName sampler name.
     \param[in] texture texture to use.
	 */
	void SetTexture(const FastName& textureFastName, Texture* texture);
    
    /**
	 \brief Returns texture by sampler name.
     Return NULL if the texture was not loaded.
     Returns texture from the current material only (doesn't take inheritance to account).
     \param[in] textureFastName sampler name.
	 \returns texture object for the given sampler name.
	 */
    Texture * GetTexture(const FastName& textureFastName) const;
    
    /**
	 \brief Returns texture path by sampler name.
     Returns texture path from the current material only (doesn't take inheritance to account).
     \param[in] textureFastName sampler name.
	 \returns texture object for the given sampler name.
	 */
	const FilePath& GetTexturePath(const FastName& textureFastName) const;
    
    /**
	 \brief Returns texture by sampler name.
     Return NULL if the texture was not loaded.
     Takes inheritance to account.
     \param[in] textureFastName sampler name.
	 \returns texture object for the given sampler name.
	 */
    Texture * GetEffectiveTexture(const FastName& textureFastName) const;
    
    /**
	 \brief Returns texture path by sampler name.
     Takes inheritance to account.
     \param[in] textureFastName sampler name.
	 \returns texture object for the given sampler name.
	 */
	const FilePath& GetEffectiveTexturePath(const FastName& textureFastName) const;
    
    /**
	 \brief Returns texture by index. Index doesn't correspond to sampler index.
     Use this method for texture enumeration.
     Return NULL if the texture was not loaded.
     Returns texture from the current material only (doesn't take inheritance to account).
     \param[in] index texture index.
	 \returns texture object for the given texture index.
	 */
    Texture * GetTexture(uint32 index) const;
    
    /**
	 \brief Returns texture path by index. Index doesn't correspond to sampler index.
     Use this method for texture enumeration.
     Returns texture from the current material only (doesn't take inheritance to account).
     \param[in] index texture index.
	 \returns texture object for the given texture index.
	 */
	const FilePath& GetTexturePath(uint32 index) const;
    
    /**
	 \brief Returns sampler name by index. Index doesn't correspond to sampler index.
     Use this method for texture enumeration.
     Returns texture from the current material only (doesn't take inheritance to account).
     \param[in] index texture index.
	 \returns sampler name for the given texture index.
	 */
	const FastName& GetTextureName(uint32 index) const;
    
    /**
	 \brief Returns number of textures set to the material.
     Use this method for texture enumeration.
     Returns number of textures from the current material only (doesn't take inheritance to account).
	 \returns number of textures.
	 */
    uint32 GetTextureCount() const;
    
    /**
	 \brief Set property value by name.
     Name usually corresponds to shader uniform name.
     \param[in] keyName property name.
     \param[in] type property type.
     \param[in] size property size.
     \param[in] data property value.
	 */
    void SetPropertyValue(const FastName & keyName,
						  Shader::eUniformType type,
						  uint32 size,
						  const void * data);
    
    /**
	 \brief Returns property value by name.
     Takes inheritance to account.
     \param[in] keyName property name.
	 \returns property value by name.
	 */
	NMaterialProperty* GetPropertyValue(const FastName & keyName) const;
    
    /**
	 \brief Returns property value by name.
     Returns property value from the current material only (doesn't take inheritance to account).
     \param[in] keyName property name.
	 \returns property value by name.
	 */
	NMaterialProperty* GetMaterialProperty(const FastName & keyName) const;
    
    /**
	 \brief Removes property from the material by name.
     Name usually corresponds to shader uniform name.
     \param[in] keyName property name.
	 */
	void RemoveMaterialProperty(const FastName & keyName);
	
    /**
	 \brief Set material name
     \param[in] name material name.
	 */
	void SetMaterialName(const FastName& name);
    
    /**
	 \brief Returns material name.
	 \returns material name.
	 */
	inline const FastName& GetMaterialName() const;
	
    /**
	 \brief Returns material type.
	 \returns material type.
	 */
	inline eMaterialType GetMaterialType() const;
	
    /**
	 \brief Returns material sorting key.
     Used by render layer to sort render batches.
	 \returns material sorting key.
	 */
	inline uint16 GetSortingKey() const;
    
    /**
	 \brief Returns mask of render layers where render batch with the giben material should be rendered.
	 \returns material render layer mask.
	 */
    inline uint32 GetRenderLayerIDsBitmask() const;
    
    /**
	 \brief Returns mask of render layers where render batch with the giben material should be rendered.
	 \returns material render layer mask.
	 */
    inline uint32 GetRenderLayers() const;
    
    /**
	 \brief Set mask of render layers where render batch with the giben material should be rendered.
     \param[in] mask of render layers.
	 */
    inline void SetRenderLayers(uint32 bitmask);
    
    /**
	 \brief Returns render state for the render pass.
     This method is fast but not thread-safe.
     \param[in] passName pass name.
	 \returns render state for the render pass.
	 */
	const RenderStateData& GetRenderState(const FastName& passName) const;
    
    /**
	 \brief Returns render state for the render pass.
     This method is slower but is thread-safe.
     \param[in] passName pass name.
	 \param[out] target output render state.
	 */
    void GetRenderState(const FastName& passName, RenderStateData& target) const;
    
    /**
	 \brief Allows to replace render state for the pass.
     \param[in] passName pass name.
	 \param[in] newState new render state.
	 */
	void SubclassRenderState(const FastName& passName, RenderStateData& newState);
    
    /**
	 \brief Allows to replace render state for the PASS_FORWARD pass.
	 \param[in] newState new render state.
	 */
	void SubclassRenderState(RenderStateData& newState);
    
    /**
	 \brief Creates and returns material object of INSTANCE type.
	 \returns material object of INSTANCE type.
	 */
	static NMaterial* CreateMaterialInstance();
	
    /**
	 \brief Creates and returns material object of INSTANCE type.
     Also creates and initializes parent material of MATERIAL type.
     Adds INSTANCE to MATERIAL as child.
     \param[in] parentName parent object name.
     \param[in] templateName material template name.
     \param[in] defaultQuality quality name to set.
	 \returns material object of INSTANCE type.
	 */
	static NMaterial* CreateMaterialInstance(const FastName& parentName,
											 const FastName& templateName,
											 const FastName& defaultQuality);
	
    /**
	 \brief Creates and returns material object of MATERIAL type.
     \param[in] parentName parent object name.
     \param[in] templateName material template name.
     \param[in] defaultQuality quality name to set.
	 \returns material object of INSTANCE type.
	 */
	static NMaterial* CreateMaterial(const FastName& materialName,
									 const FastName& templateName,
									 const FastName& defaultQuality);
    
    /**
	 \brief Creates and returns material object of GLOBAL type.
     \param[in] materialName object name.
	 \returns material object of INSTANCE type.
	 */
    static NMaterial* CreateGlobalMaterial(const FastName& materialName);

    /**
	 \brief Returns material template description.
	 \returns material template description.
	 */
	inline const NMaterialTemplate* GetMaterialTemplate() const;
    
    /**
	 \brief Sets material template.
     Calling this method will totally change material render technique set
     to correspond to target template: shader recompilation, texture loading etc will occur.
     \param[in] templateName template name.
	 */
    void SetMaterialTemplateName(const FastName& templateName);
    
    /**
	 \brief Returns material template name.
	 \returns material template name.
	 */
    FastName GetMaterialTemplateName() const;

    /**
	 \brief Returns material quality group name. Used by quality system.
	 \returns material group name.
	 */
    FastName GetMaterialGroup() const;
    
    /**
	 \brief Sets material quality group name. Used by quality system.
     \param[in] group group name.
	 */
    void SetMaterialGroup(const FastName &group);

    /**
	 \brief Rebuilds cache of shader parameters
	 */
    void BuildActiveUniformsCacheParamsCache();
    
    /**
	 \brief Rebuilds cache of texture parameters
	 */
    void BuildTextureParamsCache();
    
    /**
	 \brief Marks all material properties as dirty to re-bind them to shader
	 */
    void InvalidateProperties();
    
protected:
	class TextureBucket
	{
    public:
    
		inline TextureBucket();
        
        inline ~TextureBucket();
        
        inline void SetTexture(Texture* tx);
        
        inline Texture* GetTexture() const;
        
        inline void SetPath(const FilePath& filePath);
        
        inline const FilePath& GetPath() const;

    private:
		Texture* texture; //VI: can be NULL
		FilePath path;
	};
		
	struct UniformCacheEntry
	{
		inline UniformCacheEntry();
        
		Shader::Uniform* uniform;
		int32 index;
		NMaterialProperty* prop;
	};
		
	class RenderPassInstance
	{
    public:
		
		inline RenderPassInstance();
        
        inline ~RenderPassInstance();
        
        inline void SetShader(Shader* curShader);
        inline Shader* GetShader() const;
        inline void SetRenderer(Core::eRenderer renderer);
        inline Core::eRenderer GetRenderer() const;
        inline void SetColor(const Color& color);
        inline const Color& GetColor() const;
        
        inline void FlushState();
        
        inline UniqueHandle GetRenderStateHandle() const;
        inline void SetRenderStateHandle(UniqueHandle handle);
        inline UniqueHandle GetTextureStateHandle() const;
        inline void SetTextureStateHandle(UniqueHandle handle);
        
        
        bool dirtyState;
		bool texturesDirty;
        bool propsDirty;
		
		HashMap<FastName, int32> textureIndexMap;
		Vector<UniformCacheEntry> activeUniformsCache;
		
		UniformCacheEntry* activeUniformsCachePtr;
		size_t activeUniformsCacheSize;

        
    private:
    
        RenderState renderState;
	};
	
protected:
		
	FastName materialName;
    FastName materialGroup;
	eMaterialType materialType;

	HashMap<FastName, NMaterialProperty*> materialProperties;
	
	HashMap<FastName, TextureBucket*> textures;
	
	NMaterial* parent;
	Vector<NMaterial*> children;
	
	//TODO: fill it from configuration
	uint32 requiredVertexFormat;

 	//{TODO: these should be removed and changed to a generic system
	//setting properties via special setters
    uint32 lightCount;
    Light * lights[8];
	//}END TODO
    
    uint8 dynamicBindFlags;

	uint16 materialSortKey; //VI: depends on baseTechnique
	RenderTechnique* baseTechnique;
	HashMap<FastName, RenderPassInstance*> instancePasses;
	HashMap<FastName, UniqueHandle> instancePassRenderStates;
	
	RenderPassInstance* activePassInstance;
	RenderTechniquePass* activeRenderPass;
	FastName activePassName;
	
	FastName currentQuality;
	FastName orderedQuality;
			
    IlluminationParams * illuminationParams;
	
	const NMaterialTemplate* materialTemplate;
	
	//VI: material flags alter per-instance shader. For example, adding fog, texture animation etc
	HashMap<FastName, int32> materialSetFlags; //VI: flags set in the current material only
	
    uint32                  renderLayerIDsBitmask;
		
protected:
	
	virtual ~NMaterial();
	
	inline void SetMaterialType(eMaterialType matType);
	void SetMaterialTemplate(const NMaterialTemplate* matTemplate, const FastName& defaultQuality);
	
	void BuildEffectiveFlagSet(FastNameSet& effectiveFlagSet);
	void BuildEffectiveFlagSetInternal(FastNameSet& effectiveFlagSet);
		
	void ReleaseInstancePasses();
	
	void UpdateMaterialTemplate();
	void UpdateRenderPass(const FastName& passName,
						  const FastNameSet& instanceDefines,
						  RenderTechniquePass* pass);
	void BuildTextureParamsCache(RenderPassInstance* passInstance);
	void BuildActiveUniformsCacheParamsCache(RenderPassInstance* passInstance);
	TextureBucket* GetEffectiveTextureBucket(const FastName& textureFastName) const;
	
	void LoadActiveTextures();
	void CleanupUnusedTextures();
	Texture* GetOrLoadTextureRecursive(const FastName& textureName);
	bool IsTextureActive(const FastName& textureName) const;
	void SetTexturesDirty();
	void PrepareTextureState(RenderPassInstance* passInstance);
	void UpdateShaderWithFlags();
	//static Texture* GetStubTexture(const FastName& uniformName);
	
	void BindMaterialTextures(RenderPassInstance* passInstance);
	void BindMaterialProperties(RenderPassInstance* passInstance);
	
	//VI: this method is for updating light. It's temporary solution hopefully
	//void UpdateLightingProperties(Light* light);
	bool IsLightingProperty(const FastName& propName) const;
	//void SetLightInternal(int index, Light* light, bool forceUpdate);

	void SetParentInternal(NMaterial *material);

    FastName GetEffectiveQuality() const;
	
    
public:
	static bool IsRuntimeFlag(const FastName& flagName);
    static bool IsRuntimeProperty(const FastName& propName);
    static bool IsRuntimeTexture(const FastName& textureName);

protected:
    static bool IsNamePartOfArray(const FastName& fastName, const Vector<FastName> & propertyArray);
		
protected:
	
	void OnParentChanged(NMaterial* newParent, bool inheritTemplate);
	void OnMaterialTemplateChanged();
	void OnInstanceQualityChanged();
	
	void OnMaterialPropertyAdded(const FastName& propName);
	void OnMaterialPropertyRemoved(const FastName& propName);
	
public:
	
	INTROSPECTION_EXTEND(NMaterial, DataNode,
				  //(DAVA::CreateIspProp("materialName", "Material name", &NMaterial::GetMaterialName, &NMaterial::SetMaterialName, I_SAVE | I_EDIT | I_VIEW),
				  MEMBER(materialName, "Material name", I_SAVE | I_EDIT | I_VIEW)
                  PROPERTY("materialGroup", "Material group", GetMaterialGroup, SetMaterialGroup, I_SAVE | I_EDIT | I_VIEW)
                  PROPERTY("materialTemplate", "Material template", GetMaterialTemplateName, SetMaterialTemplateName, I_SAVE)
				  DYNAMIC(materialSetFlags, "Material flags", new NMaterialStateDynamicFlagsInsp(), I_SAVE | I_EDIT | I_VIEW)
				  DYNAMIC(textures, "Material textures", new NMaterialStateDynamicTexturesInsp(), I_SAVE | I_EDIT | I_VIEW)
				  DYNAMIC(materialProperties, "Material properties", new NMaterialStateDynamicPropertiesInsp(), I_SAVE | I_EDIT | I_VIEW)
                  MEMBER(illuminationParams, "Illumination Params", I_SAVE | I_EDIT | I_VIEW)
				  );

};

inline void NMaterial::SetMaterialType(eMaterialType matType)
{
    materialType = matType;
}

inline uint32 NMaterial::GetRenderLayers() const
{
    return renderLayerIDsBitmask & ((1 << RENDER_LAYER_ID_BITMASK_MIN_POS) - 1);
}

void NMaterial::SetRenderLayers(uint32 bitmask)
{
    renderLayerIDsBitmask = bitmask;
    RenderLayerID minLayerID = RENDER_LAYER_ID_BITMASK_MAX_MASK;
    RenderLayerID maxLayerID = 0;
    for (uint32 k = 0; k < RENDER_LAYER_ID_COUNT; ++k)
    {
        if (bitmask & (1 << k))
        {
            RenderLayerID id = k;
            minLayerID = Min(id, minLayerID);
            maxLayerID = Max(id, maxLayerID);
        }
    }
    
    if (renderLayerIDsBitmask)
    {
        DVASSERT(minLayerID < RENDER_LAYER_ID_BITMASK_MIN_MASK);
        DVASSERT(maxLayerID < RENDER_LAYER_ID_BITMASK_MAX_MASK);
        renderLayerIDsBitmask |= (minLayerID << RENDER_LAYER_ID_BITMASK_MIN_POS);
        renderLayerIDsBitmask |= (maxLayerID << RENDER_LAYER_ID_BITMASK_MAX_POS);
    }
}

inline NMaterial::RenderPassInstance::RenderPassInstance() :
dirtyState(false),
texturesDirty(true),
propsDirty(true),
textureIndexMap(8),
activeUniformsCachePtr(NULL),
activeUniformsCacheSize(0)
{
    renderState.shader = NULL;
}

inline NMaterial::RenderPassInstance::~RenderPassInstance()
{
    SetRenderStateHandle(InvalidUniqueHandle);
    SetTextureStateHandle(InvalidUniqueHandle);
    SafeRelease(renderState.shader);
}

inline void NMaterial::RenderPassInstance::SetShader(Shader* curShader)
{
    if(renderState.shader != curShader)
    {
        SafeRelease(renderState.shader);
        renderState.shader = SafeRetain(curShader);
    }
}

inline Shader* NMaterial::RenderPassInstance::GetShader() const
{
    return renderState.shader;
}

inline UniqueHandle NMaterial::RenderPassInstance::GetRenderStateHandle() const
{
    return renderState.stateHandle;
}

inline void NMaterial::RenderPassInstance::SetRenderStateHandle(UniqueHandle handle)
{
    if(renderState.stateHandle != handle)
    {
        if(renderState.stateHandle != InvalidUniqueHandle)
        {
            RenderManager::Instance()->ReleaseRenderState(renderState.stateHandle);
        }
        
        renderState.stateHandle = handle;
        
        if(renderState.stateHandle != InvalidUniqueHandle)
        {
            RenderManager::Instance()->RetainRenderState(renderState.stateHandle);
        }
    }
}

inline UniqueHandle NMaterial::RenderPassInstance::GetTextureStateHandle() const
{
    return renderState.textureState;
}

inline void NMaterial::RenderPassInstance::SetTextureStateHandle(UniqueHandle handle)
{
    if(renderState.textureState != handle)
    {
        if(renderState.textureState != InvalidUniqueHandle)
        {
            RenderManager::Instance()->ReleaseTextureState(renderState.textureState);
        }
        
        renderState.textureState = handle;
        
        if(renderState.textureState != InvalidUniqueHandle)
        {
            RenderManager::Instance()->RetainTextureState(renderState.textureState);
        }
    }
}

inline void NMaterial::RenderPassInstance::SetRenderer(Core::eRenderer renderer)
{
    renderState.renderer = renderer;
}

inline Core::eRenderer NMaterial::RenderPassInstance::GetRenderer() const
{
    return renderState.renderer;
}

inline void NMaterial::RenderPassInstance::SetColor(const Color& color)
{
    renderState.color = color;
}

inline const Color& NMaterial::RenderPassInstance::GetColor() const
{
    return renderState.color;
}

inline void NMaterial::RenderPassInstance::FlushState()
{
    RenderManager::Instance()->FlushState(&renderState);
}

inline NMaterial* NMaterial::GetParent() const
{
    return parent;
}

inline uint32 NMaterial::GetChildrenCount() const
{
    return static_cast<uint32>(children.size());
}

inline NMaterial* NMaterial::GetChild(uint32 index) const
{
    DVASSERT(index < children.size());
    return children[index];
}

inline uint32 NMaterial::GetRequiredVertexFormat() const
{
    return requiredVertexFormat;
}

inline const FastName& NMaterial::GetMaterialName() const
{
    return materialName;
}

inline NMaterial::eMaterialType NMaterial::GetMaterialType() const
{
    return materialType;
}

inline uint16 NMaterial::GetSortingKey() const
{
    return materialSortKey;
}
	
inline uint32 NMaterial::GetRenderLayerIDsBitmask() const
{
    return renderLayerIDsBitmask;
}

inline const NMaterialTemplate* NMaterial::GetMaterialTemplate() const
{
    return materialTemplate;
}

inline uint8 NMaterial::GetDynamicBindFlags() const
{
    return dynamicBindFlags;
}

inline void NMaterial::GetFlags(Vector<FastName> &flagsCollection) const
{
    flagsCollection.reserve(flagsCollection.size() + materialSetFlags.size());

    const HashMap<FastName, int32>& hash = materialSetFlags;
    for (HashMap<FastName, int32>::iterator it = hash.begin(); it != hash.end(); ++it)
    {
        flagsCollection.push_back((*it).first);
    }
}

inline IlluminationParams::IlluminationParams(NMaterial* parentMaterial) :
                                                                isUsed(true),
                                                                castShadow(true),
                                                                receiveShadow(true),
                                                                lightmapSize(LIGHTMAP_SIZE_DEFAULT),
                                                                parent(parentMaterial)
{
}

inline IlluminationParams::IlluminationParams(const IlluminationParams & params)
{
    isUsed = params.isUsed;
    castShadow = params.castShadow;
    receiveShadow = params.receiveShadow;
    lightmapSize = params.lightmapSize;
    parent = NULL;
}

inline int32 IlluminationParams::GetLightmapSize() const
{
    return lightmapSize;
}

inline NMaterial* IlluminationParams::GetParent() const
{
    return parent;
}

inline NMaterialProperty::NMaterialProperty()
{
    type = Shader::UT_INT;
    size = 0;
    data = NULL;
}

inline NMaterialProperty::~NMaterialProperty()
{
    SafeDeleteArray(data);
}

inline NMaterialProperty* NMaterialProperty::Clone()
{
    NMaterialProperty* cloneProp = new NMaterialProperty();
    
    cloneProp->size = size;
    cloneProp->type = type;
    
    if(data)
    {
        size_t dataSize = Shader::GetUniformTypeSize(type) * size;
        cloneProp->data = new uint8[dataSize];
        memcpy(cloneProp->data, data, dataSize);
    }
    
    return cloneProp;
}

inline NMaterial::TextureBucket::TextureBucket() : texture(NULL)
{
}

inline NMaterial::TextureBucket::~TextureBucket()
{
    SafeRelease(texture);
}

inline void NMaterial::TextureBucket::SetTexture(Texture* tx)
{
    if(tx != texture)
    {
        SafeRelease(texture);
        texture = SafeRetain(tx);
    }
}

inline Texture* NMaterial::TextureBucket::GetTexture() const
{
    return texture;
}

inline void NMaterial::TextureBucket::SetPath(const FilePath& filePath)
{
    path = filePath;
}

inline const FilePath& NMaterial::TextureBucket::GetPath() const
{
    return path;
}

inline NMaterial::UniformCacheEntry::UniformCacheEntry() :
uniform(NULL),
index(-1),
prop(NULL)
{
}


};

#endif // __DAVAENGINE_MATERIAL_H__

