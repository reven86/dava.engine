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
#include "Render/RenderState.h"
#include "Render/Material/NMaterialConsts.h"
#include "Render/Shader.h"
#include "Render/RenderState.h"
#include "Base/Introspection.h"
#include "Scene3D/SceneFile/SerializationContext.h"

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
class MaterialChangeListener
{
public:
	virtual void ParentChanged(NMaterial* material) = 0;
	virtual void SystemChanged(NMaterial* material) = 0;
};

struct IlluminationParams : public InspBase
{
    static const int32 LIGHTMAP_SIZE_DEFAULT = 128;
    
    bool isUsed;
    bool castShadow;
    bool receiveShadow;
    int32 lightmapSize;

    void SetDefaultParams() 
    {
        isUsed = castShadow = receiveShadow = true;
        lightmapSize = LIGHTMAP_SIZE_DEFAULT;
    }

    INTROSPECTION(IlluminationParams, 
        MEMBER(isUsed, "Use Illumination", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(castShadow, "Cast Shadow", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(receiveShadow, "Receive Shadow", I_SAVE | I_VIEW | I_EDIT)
        MEMBER(lightmapSize, "Lightmap Size", I_SAVE | I_VIEW | I_EDIT)
        );
};

class NMaterialProperty
{
public:
    Shader::eUniformType type;
    uint8 size;
    void* data;
	
	virtual ~NMaterialProperty()
	{
	}
	
	virtual NMaterialProperty* Clone()
	{
		return NULL;
	}

};
	
template<typename MEMMANAGER>
class NManagedMaterialProperty : public NMaterialProperty
{
	
public:
	
	NManagedMaterialProperty()
	{
		MEMMANAGER::Init(this);
	}

	virtual ~NManagedMaterialProperty()
	{
		MEMMANAGER::Release(this);
	}
	
	virtual NMaterialProperty* Clone()
	{
		return MEMMANAGER::Clone(this);
	}
};
		    
class MaterialTechnique
{
		
public:
	
    MaterialTechnique(const FastName & _shaderName, const FastNameSet & _uniqueDefines, RenderState * _renderState);
    ~MaterialTechnique();
    
	void RecompileShader(const FastNameSet& materialDefines);

    const FastName & GetShaderName() const { return shaderName; }
    inline Shader * GetShader() const { return shader; }
    inline RenderState * GetRenderState() const { return renderState; }
    const FastNameSet & GetUniqueDefineSet() { return uniqueDefines; }
	    
protected:
    FastName shaderName;
    Shader * shader;
    RenderState * renderState;
    FastNameSet uniqueDefines;
	};

class NMaterial;
class NMaterialState : public BaseObject
{
	friend class NMaterial;
	friend class NMaterialStateDynamicPropertiesInsp;
	friend class NMaterialStateDynamicTexturesInsp;
	
public:
			
	NMaterialState();
	virtual ~NMaterialState();
	
	// Work with textures and properties
    void SetTexture(const FastName & textureFastName, Texture * texture);
    Texture * GetTexture(const FastName & textureFastName) const;
    Texture * GetTexture(uint32 index) const;
	const FastName& GetTextureName(uint32 index) const;
    uint32 GetTextureCount() const;
    
    void SetPropertyValue(const FastName & propertyFastName, Shader::eUniformType type, uint32 size, const void * data);
	NMaterialProperty* GetMaterialProperty(const FastName & keyName);
	
	virtual void SetMaterialName(const String& name);
	const FastName& GetMaterialName() const;
	virtual void SetParentName(const String& name);
	const FastName& GetParentName() const;
	
	void AddMaterialTechnique(const FastName & techniqueName, MaterialTechnique * materialTechnique);
    MaterialTechnique * GetTechnique(const FastName & techniqueName);
	
	bool LoadFromYamlNode(const YamlNode* stateNode);
	
	NMaterialState* CloneState();
	
	inline uint32 GetRequiredVertexFormat() {return requiredVertexFormat;}
	inline NMaterial* GetParent() {return parent;}
	
	NMaterialState* CreateTemplate(NMaterial* templateParent);
	
	void SetParentToState(NMaterial* material);
			
protected:
	
	class GenericPropertyManager
	{
	public:
		static void Init(NMaterialProperty* prop);
		static void Release(NMaterialProperty* prop);
		static NMaterialProperty* Clone(NMaterialProperty* prop);
	};
	
	class UniformPropertyManager
	{
	public:
		static void Init(NMaterialProperty* prop);
		static void Release(NMaterialProperty* prop);
		static NMaterialProperty* Clone(NMaterialProperty* prop);
	};
	
	typedef NManagedMaterialProperty<NMaterialState::GenericPropertyManager> GenericMaterialProperty;
	typedef NManagedMaterialProperty<NMaterialState::UniformPropertyManager> UniformMaterialProperty;
	
protected:
	
	struct TextureBucket
	{
		Texture* texture;
		int32 index;
	};
	
	FastName parentName;
	FastName materialName;
	
	FastNameSet layers;
    
	HashMap<FastName, MaterialTechnique *> techniqueForRenderPass;
	FastNameSet nativeDefines;
    
	HashMap<FastName, NMaterialProperty* > materialProperties;
    
	HashMap<FastName, TextureBucket*> textures;
	Vector<Texture*> texturesArray;
    Vector<FastName> textureNamesArray;
	UniqueHandle textureStateHandle;
	//Vector<int32> textureSlotArray;
	
	NMaterial* parent;
	Vector<NMaterial*> children;
	
	//TODO: fill it from configuration
	uint32 requiredVertexFormat;
	
	bool texturesDirty;
	
protected:
	
	void AddMaterialProperty(const String & keyName, const YamlNode * uniformNode);
	
	void AddMaterialDefineToState(const FastName& defineName);
	void RemoveMaterialDefineFromState(const FastName& defineName);
	
	void AddChildToState(NMaterial* material);
	void RemoveChildFromState(NMaterial* material);
	
	void ShallowCopyTo(NMaterialState* targetState, bool copyNames = true);
	void DeepCopyTo(NMaterialState* targetState);
	
	void MapTextureNameToSlot(const FastName& textureName);
	
	void CopyTechniquesTo(NMaterialState* targetState);

protected:
	
	class NMaterialStateDynamicTexturesInsp : public InspInfoDynamic
	{
	public:
		int MembersCount(void *object) const;
		InspDesc MemberDesc(void *object, int index) const;
		const char* MemberName(void *object, int index) const;
		VariantType MemberValueGet(void *object, int index) const;
		void MemberValueSet(void *object, int index, const VariantType &value);
	};
	
	class NMaterialStateDynamicPropertiesInsp : public InspInfoDynamic
	{
	public:
		int MembersCount(void *object) const;
		InspDesc MemberDesc(void *object, int index) const;
		const char* MemberName(void *object, int index) const;
		VariantType MemberValueGet(void *object, int index) const;
		void MemberValueSet(void *object, int index, const VariantType &value);
	};


public:
	INTROSPECTION(NMaterialState,
		DYNAMIC(textures, "Material textures", new NMaterialStateDynamicTexturesInsp(), I_SAVE | I_EDIT | I_VIEW)
		DYNAMIC(materialProperties, "Material properties", new NMaterialStateDynamicPropertiesInsp(), I_SAVE | I_EDIT | I_VIEW)
		);
};

class Camera;
class SerializationContext;
class MaterialSystem;
class NMaterial : public NMaterialState
{
	friend class MaterialSystem;

protected:
	virtual ~NMaterial();

public:
    static const FastName TEXTURE_ALBEDO;
    static const FastName TEXTURE_NORMAL;
    static const FastName TEXTURE_DETAIL;
    static const FastName TEXTURE_LIGHTMAP;
	static const FastName TEXTURE_DECAL;
	
	static const FastName PARAM_LIGHT_POSITION0;
	static const FastName PARAM_PROP_AMBIENT_COLOR;
	static const FastName PARAM_PROP_DIFFUSE_COLOR;
	static const FastName PARAM_PROP_SPECULAR_COLOR;
	static const FastName PARAM_LIGHT_AMBIENT_COLOR;
	static const FastName PARAM_LIGHT_DIFFUSE_COLOR;
	static const FastName PARAM_LIGHT_SPECULAR_COLOR;
	static const FastName PARAM_LIGHT_INTENSITY0;
	static const FastName PARAM_MATERIAL_SPECULAR_SHININESS;
	static const FastName PARAM_FOG_COLOR;
	static const FastName PARAM_FOG_DENSITY;
	static const FastName PARAM_FLAT_COLOR;
	static const FastName PARAM_TEXTURE0_SHIFT;
	static const FastName PARAM_UV_OFFSET;
	static const FastName PARAM_UV_SCALE;
    
	NMaterial();
	
	void SetParent(NMaterial* material);
	void AddChild(NMaterial* material);
	void RemoveChild(NMaterial* material);
    
    int32 GetChildrenCount() const;
    NMaterial *GetChild(int32 index) const;
	
	//VI: you need to manually rebuild material after defines have been changed
	//this is done in order to be able change defines serially without autorebuild
	void AddMaterialDefine(const FastName& defineName);
	void RemoveMaterialDefine(const FastName& defineName);

	inline const FastNameSet & GetRenderLayers();
	
	//{TODO: these should be removed and changed to a generic system
	//setting properties via special setters
    uint32 GetLightCount() { return lightCount; };
    void SetLight(uint32 index, Light * light);
    Light * GetLight(uint32 index) { return lights[index]; };
	inline bool IsDynamicLit() {return materialDynamicLit;}
	//}END TODO
	
    void Draw(PolygonGroup * polygonGroup);
	void Draw(RenderDataObject* renderData, uint16* indices = NULL, uint16 indexCount = 0);
    
	void BindMaterialTechnique(const FastName & techniqueName, Camera* camera);    
	
	void Rebuild(bool recursive = true);
	void Rebind(bool recursive = true);
	inline bool IsReady() {return ready;}
	inline bool IsConfigMaterial() {return configMaterial;}
	
	bool LoadFromFile(const FilePath & pathname);
	
	virtual void Save(KeyedArchive * archive, SerializationContext * serializationContext);
	virtual void Load(KeyedArchive * archive, SerializationContext * serializationContext);
	
	bool SwitchState(const FastName& stateName, MaterialSystem* materialSystem,
					 bool forceSwitch = false);
	
	bool IsSwitchable() const;
	
	NMaterial* Clone();
	NMaterial* Clone(const String& newName);
    
    virtual void SetMaterialName(const String& name);
	
	//VI: these 2 methods are used for old material conversion and for setting lightmap props during lightmap generation
	//VI: need to multiplex single old state to LOD states
	uint32 GetStateCount() const;
	NMaterialState* GetState(uint32 index);
	
	inline void SetMaterialSystem(MaterialSystem* system)
	{
		bool changed = (materialSystem != system);
		materialSystem = system;
		
		if(stateListener && changed)
		{
			stateListener->SystemChanged(this);
		}
	}
	inline MaterialSystem* GetMaterialSystem() const {return materialSystem;}
	inline bool HasDefine(const FastName& defineName) const {return (inheritedDefines.count(defineName) > 0);}
		
	void SwitchParent(const FastName& newParent);
	void SwitchParentForAllStates(const FastName& newParent);
	
	inline MaterialChangeListener* GetChangeListener() {return stateListener;}
	inline void SetChangeListener(MaterialChangeListener* listener) {stateListener = listener;}
	
    IlluminationParams * GetIlluminationParams();
    void ReleaseIlluminationParams();

protected:
	
	struct TextureParamCacheEntry
	{
		FastName textureName;
		int32 slot;
		Texture* tx;
	};
	
	struct UniformCacheEntry
	{
		Shader::Uniform* uniform;
		int32 index;
		//NMaterialProperty* prop;
		void* propData;
	};

    
 	FastNameSet inheritedDefines;
 	FastNameSet effectiveLayers;
	
	//{TODO: these should be removed and changed to a generic system
	//setting properties via special setters
    uint32 lightCount;
    Light * lights[8];
	bool materialDynamicLit;
	//}END TODO

    FastName activeTechniqueName;
    MaterialTechnique * activeTechnique;
	bool ready;
	
	bool configMaterial;
	
	FastName  currentStateName;
 	HashMap<FastName, NMaterialState*> states;
	
	//TODO: therse should be stored per technique
	Vector<TextureParamCacheEntry> textureParamsCache;
	Vector<UniformCacheEntry> activeUniformsCache;
	Vector<uint8> uniformDataStorage;
	
	TextureParamCacheEntry* textureParamsCachePtr;
	UniformCacheEntry* activeUniformsCachePtr;
	size_t textureParamsCacheSize;
	size_t activeUniformsCacheSize;
	
	static uint64 uniqueIdSequence;
	
	MaterialSystem* materialSystem;
	
	MaterialChangeListener* stateListener;
	
    IlluminationParams * illuminationParams;

protected:
	
	void ResetParent();
	
	void OnParentChanged();
	void NotifyChildrenOnChange();
	
	void PropagateParentLayers();
	void PropagateParentDefines();
	void UnPropagateParentDefines();
	
//	void BindTextures(NMaterial* curMaterial, RenderState* rs);
	void SetupPerFrameProperties(Camera* camera);
	
	void Serialize(const NMaterialState& materialState, KeyedArchive * archive, SerializationContext * serializationContext);
	void Deserialize(NMaterialState& materialState, KeyedArchive * archive, SerializationContext * serializationContext);
	
	void SerializeFastNameSet(const FastNameSet& srcSet, KeyedArchive* targetArchive);
	void DeserializeFastNameSet(const KeyedArchive* srcArchive, FastNameSet& targetSet);
		
	void GenerateName();
	
	void BuildTextureParamsCache(const MaterialTechnique& technique);
	void BuildActiveUniformsCache(const MaterialTechnique& technique);
	void BindMaterialTextures(RenderState* renderState);
	void BindMaterialProperties(Shader * shader);
	
	void OnDirtyTextures();

public:
    INTROSPECTION_EXTEND(NMaterial, NMaterialState,
        MEMBER(illuminationParams, "Illumination Parameters", I_SAVE | I_VIEW | I_EDIT)
	);

};

inline const FastNameSet& NMaterial::GetRenderLayers()
{
	return effectiveLayers;
}

};

#endif // __DAVAENGINE_MATERIAL_H__

