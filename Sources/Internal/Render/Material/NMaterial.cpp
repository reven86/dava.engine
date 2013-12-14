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


#include "Render/Material/MaterialSystem.h"
#include "Render/Material/NMaterial.h"
#include "Render/RenderManager.h"
#include "Render/RenderState.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/RenderBase.h"
#include "FileSystem/YamlParser.h"
#include "Render/Shader.h"
#include "Render/Material/MaterialGraph.h"
#include "Render/Material/MaterialCompiler.h"
#include "Render/ShaderCache.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Light.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
    
	static const FastName DEFINE_VERTEX_LIT("VERTEX_LIT");
	static const FastName DEFINE_PIXEL_LIT("PIXEL_LIT");
    static const FastName LAYER_SHADOW_VOLUME("ShadowVolumeRenderLayer");
		
	MaterialTechnique::MaterialTechnique(const FastName & _shaderName, const FastNameSet & _uniqueDefines, RenderState * _renderState)
	{
		shader = 0;
		shaderName = _shaderName;
		uniqueDefines = _uniqueDefines;
		renderState = _renderState;
	}
    
	MaterialTechnique::~MaterialTechnique()
	{
		SafeRelease(shader);
	}
	
	void MaterialTechnique::RecompileShader(const FastNameSet& materialDefines)
	{
		FastNameSet combinedDefines = materialDefines;
		
		if(uniqueDefines.size() > 0)
		{
			combinedDefines.Combine(uniqueDefines);
		}
		
		SafeRelease(shader);
		shader = SafeRetain(ShaderCache::Instance()->Get(shaderName, combinedDefines));
	}
    
	const FastName NMaterial::TEXTURE_ALBEDO("albedo");
	const FastName NMaterial::TEXTURE_NORMAL("normal");
	const FastName NMaterial::TEXTURE_DETAIL("detail");
	const FastName NMaterial::TEXTURE_LIGHTMAP("lightmap");
	const FastName NMaterial::TEXTURE_DECAL("decal");
	
	
	const FastName NMaterial::PARAM_LIGHT_POSITION0("lightPosition0");
	const FastName NMaterial::PARAM_PROP_AMBIENT_COLOR("prop_ambientColor");
	const FastName NMaterial::PARAM_PROP_DIFFUSE_COLOR("prop_diffuseColor");
	const FastName NMaterial::PARAM_PROP_SPECULAR_COLOR("prop_specularColor");
	const FastName NMaterial::PARAM_LIGHT_AMBIENT_COLOR("materialLightAmbientColor");
	const FastName NMaterial::PARAM_LIGHT_DIFFUSE_COLOR("materialLightDiffuseColor");
	const FastName NMaterial::PARAM_LIGHT_SPECULAR_COLOR("materialLightSpecularColor");
	const FastName NMaterial::PARAM_LIGHT_INTENSITY0("lightIntensity0");
	const FastName NMaterial::PARAM_MATERIAL_SPECULAR_SHININESS("materialSpecularShininess");
	const FastName NMaterial::PARAM_FOG_COLOR("fogColor");
	const FastName NMaterial::PARAM_FOG_DENSITY("fogDensity");
	const FastName NMaterial::PARAM_FLAT_COLOR("flatColor");
	const FastName NMaterial::PARAM_TEXTURE0_SHIFT("texture0Shift");
	const FastName NMaterial::PARAM_UV_OFFSET("uvOffset");
	const FastName NMaterial::PARAM_UV_SCALE("uvScale");
		
	static FastName TEXTURE_NAME_PROPS[] = {
		NMaterial::TEXTURE_ALBEDO,
		NMaterial::TEXTURE_NORMAL,
		NMaterial::TEXTURE_DETAIL,
		NMaterial::TEXTURE_LIGHTMAP,
		NMaterial::TEXTURE_DECAL
	};
	
	
	void NMaterialState::GenericPropertyManager::Init(NMaterialProperty* prop)
	{
		prop->type = Shader::UT_INT;
		prop->size = 0;
		prop->data = NULL;
	}
	
	void NMaterialState::GenericPropertyManager::Release(NMaterialProperty* prop)
	{
		if(prop->data)
		{
			uint8* fakePtr = (uint8*)prop->data;
			SafeDeleteArray(fakePtr);
		}
	}
	
	NMaterialProperty* NMaterialState::GenericPropertyManager::Clone(NMaterialProperty* prop)
	{
		NMaterialState::GenericMaterialProperty* cloneProp = new NMaterialState::GenericMaterialProperty();
		
		cloneProp->size = prop->size;
		cloneProp->type = prop->type;
		if(prop->data)
		{
			size_t dataSize = Shader::GetUniformTypeSize(prop->type) * prop->size;
			cloneProp->data = new uint8[dataSize];
			memcpy(cloneProp->data, prop->data, dataSize);
		}
		
		return cloneProp;
	}

	void NMaterialState::UniformPropertyManager::Init(NMaterialProperty* prop)
	{
		prop->type = Shader::UT_INT;
		prop->size = 0;
		prop->data = NULL;
	}
	
	void NMaterialState::UniformPropertyManager::Release(NMaterialProperty* prop)
	{
		//VI: do nothing here: memory is allocated in an external array
	}
	
	NMaterialProperty* NMaterialState::UniformPropertyManager::Clone(NMaterialProperty* prop)
	{
		NMaterialState::UniformMaterialProperty* cloneProp = new NMaterialState::UniformMaterialProperty();
		cloneProp->size = prop->size;
		cloneProp->type = prop->type;
		cloneProp->data = prop->data;
		
		return cloneProp;
	}
	
	NMaterialState::NMaterialState() :
	layers(4),
	techniqueForRenderPass(8),
	nativeDefines(16),
	materialProperties(32),
	textures(8),
	texturesDirty(false),
	textureStateHandle(InvalidUniqueHandle)
	{
		parent = NULL;
		requiredVertexFormat = EVF_FORCE_DWORD;
	}
	
	NMaterialState::~NMaterialState()
	{
		for(size_t i = 0; i < children.size(); ++i)
		{
			children[i]->SetParent(NULL);
		}
		
		for(HashMap<FastName, TextureBucket*>::iterator i = textures.begin();
			i != textures.end();
			++i)
		{
			SafeRelease(i->second->texture);
			delete i->second;
		}
		
		textures.clear();
		texturesArray.clear();
		
		for(HashMap<FastName, NMaterialProperty*>::iterator i = materialProperties.begin();
			i != materialProperties.end();
			++i)
		{
			SafeDelete(i->second);
		}		
	}
	
	void NMaterialState::AddMaterialProperty(const String & keyName, const YamlNode * uniformNode)
	{
		FastName uniformName(keyName);
		Logger::FrameworkDebug("Uniform Add:%s %s", keyName.c_str(), uniformNode->AsString().c_str());
		
		Shader::eUniformType type = Shader::UT_FLOAT;
		union
		{
			float val;
			float valArray[4];
			float valMatrix[4 * 4];
			int32 valInt;
		}data;
		
		uint32 size = 0;
		
		if (uniformNode->GetType() == YamlNode::TYPE_STRING)
		{
			String uniformValue = uniformNode->AsString();
			if (uniformValue.find('.') != String::npos)
				type = Shader::UT_FLOAT;
			else
				type = Shader::UT_INT;
		}
		else if (uniformNode->GetType() == YamlNode::TYPE_ARRAY)
		{
			size = uniformNode->GetCount();
			uint32 arrayCount = 0;
			for (uint32 k = 0; k < size; ++k)
			{
				if (uniformNode->Get(k)->GetType() == YamlNode::TYPE_ARRAY)
				{
					arrayCount++;
				}
			}
			if (size == arrayCount)
			{
				if (size == 2)type = Shader::UT_FLOAT_MAT2;
				else if (size == 3)type = Shader::UT_FLOAT_MAT3;
				else if (size == 4)type = Shader::UT_FLOAT_MAT4;
			}else if (arrayCount == 0)
			{
				if (size == 2)type = Shader::UT_FLOAT_VEC2;
				else if (size == 3)type = Shader::UT_FLOAT_VEC3;
				else if (size == 4)type = Shader::UT_FLOAT_VEC4;
			}else
			{
				DVASSERT(0 && "Something went wrong");
			}
		}
		
		switch (type) {
			case Shader::UT_INT:
				data.valInt = uniformNode->AsInt();
				break;
			case Shader::UT_FLOAT:
				data.val = uniformNode->AsFloat();
				break;
			case Shader::UT_FLOAT_VEC2:
			case Shader::UT_FLOAT_VEC3:
			case Shader::UT_FLOAT_VEC4:
				for (uint32 k = 0; k < size; ++k)
				{
					data.valArray[k] = uniformNode->Get(k)->AsFloat();
				}
				break;
				
			default:
				Logger::Error("Wrong material property or format not supported.");
				break;
		}
		
		NMaterialProperty * materialProperty = new NMaterialState::GenericMaterialProperty();
		materialProperty->size = 1;
		materialProperty->type = type;
		size_t dataSize = Shader::GetUniformTypeSize(type) * materialProperty->size;
		materialProperty->data = new char[dataSize];
		
		memcpy(materialProperty->data, &data, dataSize);
		
		materialProperties.insert(uniformName, materialProperty);
	}
    
	void NMaterialState::SetPropertyValue(const FastName & propertyFastName, Shader::eUniformType type, uint32 size, const void * data)
	{
		size_t dataSize = Shader::GetUniformTypeSize(type) * size;
		NMaterialProperty * materialProperty = materialProperties.at(propertyFastName);
		if (materialProperty)
		{
			if (materialProperty->type != type || materialProperty->size != size)
			{
				DVASSERT(false); //VI: technically change material property type or size in runtime should never happen
				
				NMaterialProperty* newProp = new NMaterialState::GenericMaterialProperty();
				newProp->size = size;
				newProp->type = type;
				newProp->data = new uint8[dataSize];
				
				SafeDelete(materialProperty);
				materialProperties.insert(propertyFastName, newProp);
				
				materialProperty = newProp;
			}
		}
		else
		{
			materialProperty = new NMaterialState::GenericMaterialProperty();
			materialProperty->size = size;
			materialProperty->type = type;
			materialProperty->data = new uint8[dataSize];
			materialProperties.insert(propertyFastName, materialProperty);
		}
		
		memcpy(materialProperty->data, data, dataSize);
	}
	
	NMaterialProperty* NMaterialState::GetMaterialProperty(const FastName & keyName)
	{
		NMaterialState * currentMaterial = this;
		NMaterialProperty * property = NULL;
		while(currentMaterial != 0)
		{
			property = currentMaterial->materialProperties.at(keyName);
			if (property)
			{
				break;
			}
			
			currentMaterial = currentMaterial->parent;
		}
		
		return property;
	}
	
	void NMaterialState::SetMaterialName(const String& name)
	{
		materialName = FastName(name);
	}
	
	const FastName& NMaterialState::GetMaterialName() const
	{
		return materialName;
	}
	
	void NMaterialState::SetParentName(const String& name)
	{
		parentName = FastName(name);
	}
	
	const FastName& NMaterialState::GetParentName() const
	{
		return parentName;
	}

	void NMaterialState::AddMaterialTechnique(const FastName & techniqueName, MaterialTechnique * materialTechnique)
	{
		techniqueForRenderPass.insert(techniqueName, materialTechnique);
	}
    
	MaterialTechnique * NMaterialState::GetTechnique(const FastName & techniqueName)
	{
		MaterialTechnique * technique = techniqueForRenderPass.at(techniqueName);
		
		if(NULL == technique)
		{
			//VI: try to find technique in parent and copy it
			NMaterialState* curState = parent;
			while(curState)
			{
				technique = curState->techniqueForRenderPass.at(techniqueName);
				if(technique)
				{
					//RenderState* newRenderState = new RenderState();
					//technique->GetRenderState()->CopyTo(newRenderState);
					//MaterialTechnique* materialTechnique = new MaterialTechnique(technique->GetShaderName(),
					//															 technique->GetUniqueDefineSet(),
					//															 technique->GetRenderState());
					//AddMaterialTechnique(techniqueName, materialTechnique);
					//technique = materialTechnique;
					
					AddMaterialTechnique(techniqueName, technique);
					
					break;
				}
				
				curState = curState->parent;
			}
		}

		DVASSERT(technique != 0);
		return technique;
	}
    
    
	void NMaterialState::SetTexture(const FastName & textureFastName, Texture * texture)
	{
		TextureBucket* bucket = textures.at(textureFastName);
		if(NULL != bucket)
		{
			DVASSERT(bucket->index < (int32)texturesArray.size());
			
			if(bucket->texture != texture)
			{
				SafeRelease(bucket->texture);
				bucket->texture = SafeRetain(texture);
				texturesArray[bucket->index] = texture;
				
				texturesDirty = true;
			}
		}
		else
		{
			TextureBucket* bucket = new TextureBucket();
			bucket->texture = SafeRetain(texture);
			bucket->index = texturesArray.size();
			
			textures.insert(textureFastName, bucket);
			texturesArray.push_back(texture);
			textureNamesArray.push_back(textureFastName);
			
			texturesDirty = true;
		}
	}
    
	Texture * NMaterialState::GetTexture(const FastName & textureFastName) const
	{
		TextureBucket* bucket = textures.at(textureFastName);
		if (!bucket)
		{
			NMaterial * currentMaterial = parent;
			while(currentMaterial != 0)
			{
				bucket = currentMaterial->textures.at(textureFastName);
				if (bucket)
				{
					break;
				}
				currentMaterial = currentMaterial->parent;
			}
		}
		return (bucket != NULL) ? bucket->texture : NULL;
	}
    
	Texture * NMaterialState::GetTexture(uint32 index) const
	{
		DVASSERT(index >= 0 && index < texturesArray.size());
		return texturesArray[index];
	}
	
	const FastName& NMaterialState::GetTextureName(uint32 index) const
	{
		DVASSERT(index >= 0 && index < textureNamesArray.size());
		return textureNamesArray[index];
	}
	
	uint32 NMaterialState::GetTextureCount() const
	{
		return (uint32)texturesArray.size();
	}
	
	void NMaterialState::MapTextureNameToSlot(const FastName& textureName)
	{
		GetMaterialProperty(textureName);
	}
	
	void NMaterialState::SetParentToState(NMaterial* material)
	{
		parent = SafeRetain(material);
		parentName = (NULL == parent) ? FastName("") : parent->GetMaterialName();
		
		DVASSERT(parentName.IsValid());
	}
	
	void NMaterialState::AddChildToState(NMaterial* material)
	{
		SafeRetain(material);
		children.push_back(material);
	}
	
	void NMaterialState::RemoveChildFromState(NMaterial* material)
	{
		Vector<NMaterial*>::iterator child = std::find(children.begin(), children.end(), material);
		if(children.end() != child)
		{
			SafeRelease(material);
			children.erase(child);
		}
	}
	
	void NMaterialState::AddMaterialDefineToState(const FastName& defineName)
	{
		nativeDefines.Insert(defineName);
	}
	
	void NMaterialState::RemoveMaterialDefineFromState(const FastName& defineName)
	{
		nativeDefines.erase(defineName);
	}
	
	void NMaterialState::ShallowCopyTo(NMaterialState* targetState, bool copyNames)
	{
		DVASSERT(this != targetState);
		
		targetState->nativeDefines.clear();
		targetState->layers.clear();
		targetState->textures.clear();
		targetState->texturesArray.clear();
		targetState->textureNamesArray.clear();

		if(copyNames)
		{
			targetState->parentName = parentName;
		}
		
		targetState->layers.Combine(layers);
		
		targetState->nativeDefines.Combine(nativeDefines);
		
		targetState->techniqueForRenderPass.clear();
		for(HashMap<FastName, MaterialTechnique *>::iterator it = techniqueForRenderPass.begin();
			it != techniqueForRenderPass.end();
			++it)
		{
			targetState->techniqueForRenderPass.insert(it->first, it->second);
		}
		
		targetState->materialProperties.clear();
		for(HashMap<FastName, NMaterialProperty *>::iterator it = materialProperties.begin();
			it != materialProperties.end();
			++it)
		{
			//targetState->materialProperties.insert(it->first, it->second);
			targetState->SetPropertyValue(it->first,
										  it->second->type,
										  it->second->size,
										  it->second->data);

		}

		
		for(HashMap<FastName, TextureBucket *>::iterator it = textures.begin();
			it != textures.end();
			++it)
		{
			targetState->SetTexture(it->first, it->second->texture);
		}
		
		targetState->requiredVertexFormat = requiredVertexFormat;
	}
	
	void NMaterialState::CopyTechniquesTo(NMaterialState* targetState)
	{
		HashMap<FastName, MaterialTechnique *>::iterator techIter = techniqueForRenderPass.begin();
		while(techIter != techniqueForRenderPass.end())
		{
			MaterialTechnique* technique = techIter->second;
			
			RenderState* newRenderState = new RenderState();
			technique->GetRenderState()->CopyTo(newRenderState);
			MaterialTechnique* childMaterialTechnique = new MaterialTechnique(technique->GetShaderName(),
																			  technique->GetUniqueDefineSet(),
																			  newRenderState);
			targetState->AddMaterialTechnique(techIter->first, childMaterialTechnique);
			
			++techIter;
		}
	}
	
	bool NMaterialState::LoadFromYamlNode(const YamlNode* stateNode)
	{
		bool result = false;
		
		const YamlNode * parentNameNode = stateNode->Get("Parent");
		if (parentNameNode)
		{
			parentName = FastName(parentNameNode->AsString());
			DVASSERT(parentName.IsValid());
		}

		
		const YamlNode * layersNode = stateNode->Get("Layers");
		if (layersNode)
		{
			int32 count = layersNode->GetCount();
			for (int32 k = 0; k < count; ++k)
			{
				const YamlNode * singleLayerNode = layersNode->Get(k);
				layers.Insert(FastName(singleLayerNode->AsString()));
			}
		}
		
		const YamlNode * uniformsNode = stateNode->Get("Uniforms");
		if (uniformsNode)
		{
			uint32 count = uniformsNode->GetCount();
			for (uint32 k = 0; k < count; ++k)
			{
				const YamlNode * uniformNode = uniformsNode->Get(k);
				if (uniformNode)
				{
					AddMaterialProperty(uniformsNode->GetItemKeyName(k), uniformNode);
				}
			}
		}
		
		const YamlNode * materialDefinesNode = stateNode->Get("MaterialDefines");
		if (materialDefinesNode)
		{
			uint32 count = materialDefinesNode->GetCount();
			for (uint32 k = 0; k < count; ++k)
			{
				const YamlNode * defineNode = materialDefinesNode->Get(k);
				if (defineNode)
				{
					nativeDefines.Insert(FastName(defineNode->AsString().c_str()));
				}
			}
		}
		
		uint32 techniqueCount = 0;
		for (int32 k = 0; k < stateNode->GetCount(); ++k)
		{
			const YamlNode * renderStepNode = stateNode->Get(k);
			
			if (renderStepNode->AsString() == "RenderPass")
			{
				Logger::FrameworkDebug("- RenderPass found: %s", renderStepNode->AsString().c_str());
				const YamlNode * shaderNode = renderStepNode->Get("Shader");
				const YamlNode * shaderGraphNode = renderStepNode->Get("ShaderGraph");
				
				if (!shaderNode && !shaderGraphNode)
				{
					Logger::Error("RenderPass:%s does not have shader or shader graph", renderStepNode->AsString().c_str());
					break;
				}
				
				FastName shaderName;
				if (shaderNode)
				{
					shaderName = FastName(shaderNode->AsString().c_str());
				}
				
				if (shaderGraphNode)
				{
					String shaderGraphPathname = shaderGraphNode->AsString();
					MaterialGraph * graph = new MaterialGraph();
					graph->LoadFromFile(shaderGraphPathname);
					
					MaterialCompiler * compiler = new MaterialCompiler();
					compiler->Compile(graph, 0, 4, 0);
										
					SafeRelease(compiler);
					SafeRelease(graph);
				}
				
				FastNameSet definesSet;
				const YamlNode * definesNode = renderStepNode->Get("UniqueDefines");
				if (definesNode)
				{
					int32 count = definesNode->GetCount();
					for (int32 k = 0; k < count; ++k)
					{
						const YamlNode * singleDefineNode = definesNode->Get(k);
						definesSet.Insert(FastName(singleDefineNode->AsString().c_str()));
					}
				}
				
				RenderState * renderState = new RenderState();
				if (renderStepNode)
				{
					renderState->LoadFromYamlNode(renderStepNode);
				}
				
				const YamlNode * renderPassNameNode = renderStepNode->Get("Name");
				FastName renderPassName;
				if (renderPassNameNode)
				{
					renderPassName = FastName(renderPassNameNode->AsString());
				}
				
				MaterialTechnique * technique = new MaterialTechnique(shaderName, definesSet, renderState);
				AddMaterialTechnique(renderPassName, technique);
				
				techniqueCount++;
			}
		}
				
		result = true;
		
		return result;
	}
	
	void NMaterialState::DeepCopyTo(NMaterialState* targetState)
	{
		DVASSERT(this != targetState);
		
		targetState->nativeDefines.clear();
		targetState->layers.clear();
		targetState->textures.clear();
		targetState->texturesArray.clear();
		targetState->textureNamesArray.clear();

        if(parentName.IsValid()) targetState->parentName = parentName;
		targetState->layers.Combine(layers);
		targetState->nativeDefines.Combine(nativeDefines);
		
		for(HashMap<FastName, TextureBucket *>::iterator it = textures.begin();
			it != textures.end();
			++it)
		{
			targetState->SetTexture(it->first, it->second->texture);
		}
		
		for(Vector<NMaterial*>::iterator it = children.begin();
			it != children.end();
			++it)
		{
			targetState->children.push_back(SafeRetain(*it));
		}
		
		CopyTechniquesTo(targetState);
		
		HashMap<FastName, NMaterialProperty*>::iterator propIter = materialProperties.begin();
		while(propIter != materialProperties.end())
		{
			//targetState->materialProperties.insert(propIter->first, propIter->second->Clone());
			targetState->SetPropertyValue(propIter->first,
										  propIter->second->type,
										  propIter->second->size,
										  propIter->second->data);
			
			++propIter;
		}
		
		targetState->requiredVertexFormat = requiredVertexFormat;
	}
	
	NMaterialState* NMaterialState::CloneState()
	{
		NMaterialState* clonedState = new NMaterialState();
				
		DeepCopyTo(clonedState);
		
		return clonedState;
	}
	
	NMaterialState* NMaterialState::CreateTemplate(NMaterial* templateParent)
	{
		NMaterialState* templateState = new NMaterialState();
		
		templateState->parent = templateParent;
		templateState->parentName = templateParent->materialName;
				
		return templateState;

	}

	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
	uint64 NMaterial::uniqueIdSequence = 0;
	
	NMaterial::NMaterial() : NMaterialState(),
	inheritedDefines(16),
	effectiveLayers(8),
	states(4)
	{
		stateListener = NULL;
		activeTechnique = 0;
		ready = false;
		materialDynamicLit = false;
		configMaterial = false;
		materialSystem = NULL;
		memset(lights, 0, sizeof(Light*) * 8);
		lightCount = 0;
		
		textureParamsCachePtr = NULL;
		activeUniformsCachePtr = NULL;
		textureParamsCacheSize = 0;
		activeUniformsCacheSize = 0;

        illuminationParams = 0;
	}
    
	NMaterial::~NMaterial()
	{
		if(IsConfigMaterial())
		{
			for(HashMap<FastName, MaterialTechnique*>::iterator i = techniqueForRenderPass.begin();
				i != techniqueForRenderPass.end();
				++i)
			{
				delete i->second;
			}
		}
		
		if(InvalidUniqueHandle != textureStateHandle)
		{
			RenderManager::Instance()->ReleaseTextureStateData(textureStateHandle);
		}
		
		if(materialSystem)
		{
			materialSystem->RemoveMaterial(this);
		}

        SafeDelete(illuminationParams);
		SetParent(NULL);
	}
    
	bool NMaterial::LoadFromFile(const FilePath & pathname)
	{
		bool result = false;
		YamlParser * parser = YamlParser::Create(pathname);
		if (!parser)
		{
			Logger::Error("Can't load requested material: %s", pathname.GetAbsolutePathname().c_str());
			return result;
		}
		
		YamlNode * rootNode = parser->GetRootNode();
		
		if (!rootNode)
		{
			SafeRelease(rootNode);
			SafeRelease(parser);
			return result;
		}
		
		const YamlNode * materialNode = rootNode->Get("Material");
		
		uint32 materialStateCount = 0;
		for(int32 i = 0; i < materialNode->GetCount(); ++i)
		{
			const YamlNode* materialStateNode = materialNode->Get(i);
			
			if (materialStateNode->AsString() == "MaterialState")
			{
				materialStateCount++;
			}
		}
		
		for(int32 i = 0; i < materialNode->GetCount(); ++i)
		{
			const YamlNode* materialStateNode = materialNode->Get(i);
			
			if (materialStateNode->AsString() == "MaterialState")
			{
				const YamlNode* materialStateNameNode = materialStateNode->Get("Name");
				
				if(materialStateNameNode)
				{
					FastName materialStateName(materialStateNameNode->AsString());
					
					if(!states.count(materialStateName))
					{
						if(materialStateCount > 1)
						{
							NMaterialState* materialState = new NMaterialState();
							if(materialState->LoadFromYamlNode(materialStateNode))
							{
								states.insert(materialStateName, materialState);
							}
							else
							{
								Logger::Error("[NMaterial::LoadFromFile] Failed to load a material state %s in file %s!",
											  materialStateName.c_str(),
											  pathname.GetAbsolutePathname().c_str());
								
								DVASSERT(false);
							}
						}
						else
						{
							LoadFromYamlNode(materialStateNode);
							currentStateName = materialStateName;
						}
					}
					else
					{
						Logger::Error("[NMaterial::LoadFromFile] Duplicate material state %s found in file %s!",
									  materialStateName.c_str(),
									  pathname.GetAbsolutePathname().c_str());
						
						DVASSERT(false);
					}
					
				}
				else
				{
					Logger::Error("[NMaterial::LoadFromFile] There's a material state without a name in file %s!", pathname.GetAbsolutePathname().c_str());
					DVASSERT(false);
				}
			}
		}
				
		result = true;
		SafeRelease(parser);
		return result;
	}
	
	void NMaterial::BindMaterialTextures(RenderState* renderState)
	{
		//Vector<TextureParamCacheEntry>::iterator texEnd = textureParamsCache.end();
		//for(Vector<TextureParamCacheEntry>::iterator i = textureParamsCache.begin(); i < texEnd; ++i)
		//TextureParamCacheEntry* data = textureParamsCache.data();
		//size_t dataCount = textureParamsCache.size();
		//for(size_t i = 0; i < dataCount; ++i)
		
		if(texturesDirty)
		{
			OnDirtyTextures();
			texturesDirty = false;
		}
		
		renderState->textureState = textureStateHandle;
	}
	
	void NMaterial::BindMaterialProperties(Shader * shader)
	{
		//VI: need to set uniforms AFTER FlushState (since program has been bind there)
		//Vector<UniformCacheEntry>::iterator uniformsEnd = activeUniformsCache.end();
		//UniformCacheEntry* data = activeUniformsCache.data();
		//size_t dataCount = activeUniformsCache.size();
		//for(Vector<UniformCacheEntry>::iterator i = activeUniformsCache.begin(); i != uniformsEnd; ++i)
		//for(size_t i = 0; i < dataCount; ++i)
		for(size_t i = 0; i < activeUniformsCacheSize; ++i)
		{
			UniformCacheEntry& uniformEntry = activeUniformsCachePtr[i];
			Shader::Uniform* uniform = uniformEntry.uniform;
			
			//Logger::FrameworkDebug("[NMaterial::BindMaterialTechnique] setting property %s", uniformEntry.uniform->name.c_str());
			shader->SetUniformValueByUniform(uniform,
										   uniform->type,
										   uniform->size,
										   uniformEntry.propData);
		}
	}
	
	void NMaterial::BindMaterialTechnique(const FastName & techniqueName, Camera* camera)
	{
		if (techniqueName != activeTechniqueName)
		{
			activeTechnique = GetTechnique(techniqueName);
			if (activeTechnique)
			{
				activeTechniqueName = techniqueName;
			}
			else
			{
				DVASSERT(activeTechnique);
				return;
			}
			
			BuildTextureParamsCache(*activeTechnique);
			BuildActiveUniformsCache(*activeTechnique);
			
			ready = true;
		}
		
		SetupPerFrameProperties(camera);
		
		RenderState* renderState = activeTechnique->GetRenderState();
				
		Shader * shader = activeTechnique->GetShader();
		
		BindMaterialTextures(renderState);
		/*Vector<TextureParamCacheEntry>::iterator texEnd = textureParamsCache.end();
		for(Vector<TextureParamCacheEntry>::iterator i = textureParamsCache.begin(); i < texEnd; ++i)
		{
			TextureParamCacheEntry& textureEntry = *i;
			Texture* texture = GetTexture(textureEntry.textureName);
			renderState->SetTexture(texture, textureEntry.slot);
		}*/
		
		renderState->SetShader(shader);
		
		RenderManager::Instance()->FlushState(renderState);
		
		BindMaterialProperties(shader);
		//VI: need to set uniforms AFTER FlushState (since program has been bind there)
		/*Vector<UniformCacheEntry>::iterator uniformsEnd = activeUniformsCache.end();
		for(Vector<UniformCacheEntry>::iterator i = activeUniformsCache.begin(); i != uniformsEnd; ++i)
		{
			UniformCacheEntry& uniformEntry = *i;
			
			if(NULL == uniformEntry.prop)
			{
				uniformEntry.prop = GetMaterialProperty(uniformEntry.uniform->name);
			}
			
			if(uniformEntry.prop)
			{
				//Logger::FrameworkDebug("[NMaterial::BindMaterialTechnique] setting property %s", uniformEntry.uniform->name.c_str());
				shader->SetUniformValueByIndex(uniformEntry.index,
											   uniformEntry.uniform->type,
											   uniformEntry.uniform->size,
											   uniformEntry.prop->data,
											   uniformEntry.prop->dataSize);
			}
		}*/
		
		DVASSERT(ready);
	}
	
	void NMaterial::Draw(PolygonGroup * polygonGroup)
	{
		DVASSERT(ready);
		// TODO: Remove support of OpenGL ES 1.0 from attach render data
		RenderManager::Instance()->SetRenderData(polygonGroup->renderDataObject);
		RenderManager::Instance()->AttachRenderData();
		
		// TODO: rethink this code
		if (polygonGroup->renderDataObject->GetIndexBufferID() != 0)
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, polygonGroup->indexCount, EIF_16, 0);
		}
		else
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, polygonGroup->indexCount, EIF_16, polygonGroup->indexArray);
		}
	}
	
	void NMaterial::Draw(RenderDataObject*	renderData, uint16* indices, uint16 indexCount)
	{
		DVASSERT(ready);
		DVASSERT(renderData);
		
		// TODO: Remove support of OpenGL ES 1.0 from attach render data
		RenderManager::Instance()->SetRenderData(renderData);
		RenderManager::Instance()->AttachRenderData();
		
		// TODO: rethink this code
		if (renderData->GetIndexBufferID() != 0)
		{
			RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, renderData->indexCount, EIF_16, 0);
		}
		else
		{
			if(renderData->indexCount)
			{
				RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, renderData->indexCount, EIF_16, renderData->indices);
			}
			else
			{
				RenderManager::Instance()->HWDrawElements(PRIMITIVETYPE_TRIANGLELIST, indexCount, EIF_16, indices);
			}
		}
	}
    
	void NMaterial::SetParent(NMaterial* material)
	{
		if(parent)
		{
			parent->RemoveChild(this);
		}
		
		ResetParent();
		SetParentToState(material);
		
		if(parent)
		{
			parent->AddChild(this);
		}
	}
	
	void NMaterial::AddChild(NMaterial* material)
	{
		DVASSERT(material);
		DVASSERT(material != this);
		//sanity check if such material is already present
		DVASSERT(std::find(NMaterialState::children.begin(), NMaterialState::children.end(), material) == NMaterialState::children.end());
		
		if(material)
		{
			AddChildToState(material);
			
			//TODO: propagate properties such as defines, textures, render state etc here
			material->OnParentChanged();
		}
	}
	
	void NMaterial::RemoveChild(NMaterial* material)
	{
		DVASSERT(material);
		DVASSERT(material != this);
		
		Vector<NMaterial*>::iterator child = std::find(NMaterialState::children.begin(), NMaterialState::children.end(), material);
		if(NMaterialState::children.end() != child)
		{
			material->ResetParent();
			
			RemoveChildFromState(material);
		}
	}
	
    int32 NMaterial::GetChildrenCount() const
    {
        return (int32)NMaterialState::children.size();
    }
    
    NMaterial * NMaterial::GetChild(int32 index) const
    {
        DVASSERT((0 <= index) && (index < (int32)NMaterialState::children.size()));
        
        return NMaterialState::children[index];
    }

    
	void NMaterial::ResetParent()
	{
		activeTechnique = NULL;
		activeTechniqueName.Reset();
		
		//TODO: clear parent states such as textures, defines, etc
		effectiveLayers.clear();
		effectiveLayers.Combine(layers);
		UnPropagateParentDefines();
		
		SetParentToState(NULL);
	}
	
	void NMaterial::PropagateParentDefines()
	{
		ready = false;
		inheritedDefines.clear();
		if(parent)
		{
			if(parent->inheritedDefines.size() > 0)
			{
				inheritedDefines.Combine(parent->inheritedDefines);
			}
			
			if(parent->nativeDefines.size() > 0)
			{
				inheritedDefines.Combine(parent->nativeDefines);
			}
		}
	}
	
	void NMaterial::UnPropagateParentDefines()
	{
		ready = false;
		inheritedDefines.clear();
	}
	
	void NMaterial::Rebuild(bool recursive)
	{
		FastNameSet combinedDefines = inheritedDefines;
		
		if(nativeDefines.size() > 0)
		{
			combinedDefines.Combine(nativeDefines);
		}
		
		if(IsConfigMaterial())
		{
			HashMap<FastName, MaterialTechnique *>::iterator iter = techniqueForRenderPass.begin();
			while(iter != techniqueForRenderPass.end())
			{
				MaterialTechnique* technique = iter->second;
				technique->RecompileShader(combinedDefines);
			
				//BuildTextureParamsCache(*technique);
				//BuildActiveUniformsCache(*technique);
			
				++iter;
			}
		}
				
		if(recursive)
		{
			size_t childrenCount = NMaterialState::children.size();
			for(size_t i = 0; i < childrenCount; ++i)
			{
				NMaterialState::children[i]->Rebuild(recursive);
			}
		}
		
		ready = !IsConfigMaterial() && (techniqueForRenderPass.size() > 0);
	}
	
	void NMaterial::AddMaterialDefine(const FastName& defineName)
	{
		ready = false;
		AddMaterialDefineToState(defineName);
		
		NotifyChildrenOnChange();
	}
	
	void NMaterial::RemoveMaterialDefine(const FastName& defineName)
	{
		ready = false;
		RemoveMaterialDefineFromState(defineName);
		
		NotifyChildrenOnChange();
	}
		
	void NMaterial::OnParentChanged()
	{
		PropagateParentLayers();
		PropagateParentDefines();
		
		if(!IsConfigMaterial())
		{
			techniqueForRenderPass.clear(); //VI: will copy parent techniques
			
			activeTechnique = NULL;
			activeTechniqueName.Reset();
		}
		
		//{TODO: remove this!
		
		materialDynamicLit = (parent) ? parent->IsDynamicLit() : false;
		materialDynamicLit = materialDynamicLit ||
							inheritedDefines.count(DEFINE_VERTEX_LIT) ||
							inheritedDefines.count(DEFINE_PIXEL_LIT) ||
							effectiveLayers.count(LAYER_SHADOW_VOLUME);
		//END TODO}
		
		NotifyChildrenOnChange();
		
		if(stateListener)
		{
			stateListener->ParentChanged(this);
		}
	}
	
	void NMaterial::NotifyChildrenOnChange()
	{
		size_t count = NMaterialState::children.size();
		for(size_t i = 0; i < count; ++i)
		{
			NMaterialState::children[i]->OnParentChanged();
		}
	}
	
	void NMaterial::PropagateParentLayers()
	{
		effectiveLayers.clear();
		effectiveLayers.Combine(layers);
		
		if(parent)
		{
			const FastNameSet& parentLayers = parent->GetRenderLayers();
			effectiveLayers.Combine(parentLayers);
		}
	}
		
	
	static const FastName PARAM_LIGHT_POSITION0;
	static const FastName PARAM_PROP_AMBIENT_COLOR;
	static const FastName PARAM_PROP_DIFFUSE_COLOR;
	static const FastName PARAM_PROP_SPECULAR_COLOR;
	static const FastName PARAM_LIGHT_AMBIENT_COLOR;
	static const FastName PARAM_LIGHT_DIFFUSE_COLOR;
	static const FastName PARAM_LIGHT_SPECULAR_COLOR;
	static const FastName PARAM_LIGHT_INTENSITY0;

	void NMaterial::SetupPerFrameProperties(Camera* camera)
	{
		//VI: this is vertex or pixel lit material
		//VI: setup light for the material
		//VI: TODO: deal with multiple lights
		if(camera && materialDynamicLit && lights[0])
		{
			const Matrix4 & matrix = camera->GetMatrix();
			Vector3 lightPosition0InCameraSpace = lights[0]->GetPosition() * matrix;
			
			SetPropertyValue(NMaterial::PARAM_LIGHT_POSITION0, Shader::UT_FLOAT_VEC3, 1, lightPosition0InCameraSpace.data);
		}
	}
	
	void NMaterial::SetLight(uint32 index, Light * light)
	{
		bool changed = (light != lights[index]);
		lights[index] = light;
		
		if(changed && materialDynamicLit && lights[0])
		{
			NMaterialProperty* propAmbientColor = GetMaterialProperty(NMaterial::PARAM_PROP_AMBIENT_COLOR);
			NMaterialProperty* propDiffuseColor = GetMaterialProperty(NMaterial::PARAM_PROP_DIFFUSE_COLOR);
			NMaterialProperty* propSpecularColor = GetMaterialProperty(NMaterial::PARAM_PROP_SPECULAR_COLOR);
			
			Color materialAmbientColor = (propAmbientColor) ? *(Color*)propAmbientColor->data : Color(1, 1, 1, 1);
			Color materialDiffuseColor = (propDiffuseColor) ? *(Color*)propDiffuseColor->data : Color(1, 1, 1, 1);
			Color materialSpecularColor = (propSpecularColor) ? *(Color*)propSpecularColor->data : Color(1, 1, 1, 1);
			float32 intensity = lights[0]->GetIntensity();
			
			materialAmbientColor = materialAmbientColor * lights[0]->GetAmbientColor();
			materialDiffuseColor = materialDiffuseColor * lights[0]->GetDiffuseColor();
			materialSpecularColor = materialSpecularColor * lights[0]->GetSpecularColor();
			
			SetPropertyValue(NMaterial::PARAM_LIGHT_AMBIENT_COLOR, Shader::UT_FLOAT_VEC3, 1, &materialAmbientColor);
			SetPropertyValue(NMaterial::PARAM_LIGHT_DIFFUSE_COLOR, Shader::UT_FLOAT_VEC3, 1, &materialDiffuseColor);
			SetPropertyValue(NMaterial::PARAM_LIGHT_SPECULAR_COLOR, Shader::UT_FLOAT_VEC3, 1, &materialSpecularColor);
			SetPropertyValue(NMaterial::PARAM_LIGHT_INTENSITY0, Shader::UT_FLOAT, 1, &intensity);
		}
		
	}
	
	void NMaterial::Save(KeyedArchive * archive, SerializationContext * serializationContext)
	{
		archive->SetString("materialName", (materialName.IsValid()) ? materialName.c_str() : "");
		
		if(!IsSwitchable())
		{
			KeyedArchive* defaultStateArchive = new KeyedArchive();
			Serialize(*this, defaultStateArchive, serializationContext);
			archive->SetArchive("__defaultState__", defaultStateArchive);
			SafeRelease(defaultStateArchive);
		}
		else
		{
			HashMap<FastName, NMaterialState*>::iterator stateIter = states.begin();
			while(stateIter != states.end())
			{
				KeyedArchive* stateArchive = new KeyedArchive();
				Serialize(*stateIter->second, stateArchive, serializationContext);
				archive->SetArchive(stateIter->first.c_str(), stateArchive);
				SafeRelease(stateArchive);
				
				++stateIter;
			}
		}

        if(illuminationParams)
        {
            archive->SetBool("illumination.isUsed", illuminationParams->isUsed);
            archive->SetBool("illumination.castShadow", illuminationParams->castShadow);
            archive->SetBool("illumination.receiveShadow", illuminationParams->receiveShadow);
            archive->SetInt32("illumination.lightmapSize", illuminationParams->lightmapSize);
        }
	}
	
	void NMaterial::Load(KeyedArchive * archive, SerializationContext * serializationContext)
	{
		//TODO: add code allowing to transition from switchable to non-switchable materials and vice versa
		const Map<String, VariantType*>& archiveData = archive->GetArchieveData();
		SetMaterialName(archive->GetString("materialName"));
		
        if(archive->IsKeyExists("__defaultState__"))
        {
            KeyedArchive* stateArchive = archive->GetArchive("__defaultState__");
            Deserialize(*this, stateArchive, serializationContext);
        }
        else
        {
            for(Map<String, VariantType*>::const_iterator it = archiveData.begin();
                it != archiveData.end();
                ++it)
            {
                if(VariantType::TYPE_KEYED_ARCHIVE == it->second->type)
                {
                    NMaterialState* matState = new NMaterialState();
                    Deserialize(*matState, it->second->AsKeyedArchive(), serializationContext);
                    states.insert(FastName(it->first), matState);
                }
            }
        }

        if(archive->IsKeyExists("illumination.isUsed"))
        {
            GetIlluminationParams(); //create only

            illuminationParams->isUsed = archive->GetBool("illumination.isUsed", illuminationParams->isUsed);
            illuminationParams->castShadow = archive->GetBool("illumination.castShadow", illuminationParams->castShadow);
            illuminationParams->receiveShadow = archive->GetBool("illumination.receiveShadow", illuminationParams->receiveShadow);
            illuminationParams->lightmapSize = archive->GetInt32("illumination.lightmapSize", illuminationParams->lightmapSize);
        }
	}
	
	void NMaterial::Serialize(const NMaterialState& materialState,
							  KeyedArchive * archive,
							  SerializationContext * serializationContext)
	{
		//Logger::FrameworkDebug("Serialize: %s - %s", materialName.c_str(), materialState.parentName.c_str());
		
		DVASSERT(materialState.parentName.IsValid());
		DVASSERT(materialState.materialName.IsValid());
		
		DVASSERT(materialState.parentName != FastName(""));
		DVASSERT(materialState.materialName != FastName(""));
		
		archive->SetString("parentName", (materialState.parentName.IsValid()) ? materialState.parentName.c_str() : "");
		archive->SetString("materialName", materialState.materialName.c_str());
		
		KeyedArchive* materialLayers = new KeyedArchive();
		SerializeFastNameSet(materialState.layers, materialLayers);
		archive->SetArchive("layers", materialLayers);
		SafeRelease(materialLayers);
		
		KeyedArchive* materialNativeDefines = new KeyedArchive();
		SerializeFastNameSet(materialState.nativeDefines, materialNativeDefines);
		archive->SetArchive("nativeDefines", materialNativeDefines);
		SafeRelease(materialNativeDefines);
		
		KeyedArchive* materialProps = new KeyedArchive();
		for(HashMap<FastName, NMaterialProperty*>::iterator it = materialState.materialProperties.begin();
			it != materialState.materialProperties.end();
			++it)
		{
			NMaterialProperty* property = it->second;
			
			uint32 dataSize = Shader::GetUniformTypeSize(property->type) * property->size;
			uint8* propertyStorage = new uint8[dataSize + sizeof(property->type) + sizeof(property->size)];
			
			memcpy(propertyStorage, &property->type, sizeof(property->type));
			memcpy(propertyStorage + sizeof(property->type), &property->size, sizeof(property->size));
			memcpy(propertyStorage + sizeof(property->type) + sizeof(property->size), property->data, dataSize);
			
			materialProps->SetByteArray(it->first.c_str(), propertyStorage, dataSize + sizeof(property->type) + sizeof(property->size));
			
			SafeDeleteArray(propertyStorage);
		}
		archive->SetArchive("properties", materialProps);
		SafeRelease(materialProps);
		
		KeyedArchive* materialTextures = new KeyedArchive();
		for(HashMap<FastName, TextureBucket*>::iterator it = materialState.textures.begin();
			it != materialState.textures.end();
			++it)
		{
			if(it->second->texture)
			{
				materialTextures->SetString(it->first.c_str(), it->second->texture->GetPathname().GetRelativePathname(serializationContext->GetScenePath()));
			}
		}
		archive->SetArchive("textures", materialTextures);
		SafeRelease(materialTextures);
		
		int techniqueIndex = 0;
		KeyedArchive* materialTechniques = new KeyedArchive();
		for(HashMap<FastName, MaterialTechnique *>::iterator it = materialState.techniqueForRenderPass.begin();
			it != materialState.techniqueForRenderPass.end();
			++it)
		{
			MaterialTechnique* technique = it->second;
			KeyedArchive* techniqueArchive = new KeyedArchive();
			
			techniqueArchive->SetString("renderPass", it->first.c_str());
			techniqueArchive->SetString("shaderName", it->second->GetShaderName().c_str());
			
			KeyedArchive* techniqueDefines = new KeyedArchive();
			const FastNameSet& techniqueDefinesSet = technique->GetUniqueDefineSet();
			SerializeFastNameSet(techniqueDefinesSet, techniqueDefines);
			techniqueArchive->SetArchive("defines", techniqueDefines);
			SafeRelease(techniqueDefines);
			
			KeyedArchive* techniqueRenderState = new KeyedArchive();
			RenderState* renderState = technique->GetRenderState();
			renderState->Serialize(techniqueRenderState, serializationContext);
			techniqueArchive->SetArchive("renderState", techniqueRenderState);
			SafeRelease(techniqueRenderState);
			
			materialTechniques->SetArchive(Format("technique.%d", techniqueIndex), techniqueArchive);
			SafeRelease(techniqueArchive);
			techniqueIndex++;
		}
		archive->SetArchive("techniques", materialTechniques);
		SafeRelease(materialTechniques);
	}
	
	void NMaterial::Deserialize(NMaterialState& materialState,
								KeyedArchive * archive,
								SerializationContext * serializationContext)
	{
		materialState.parentName = FastName(archive->GetString("parentName"));
		materialState.materialName = FastName(archive->GetString("materialName"));
		
		DVASSERT(materialState.parentName.IsValid());
		DVASSERT(materialState.materialName.IsValid());
		DVASSERT(materialState.parentName != FastName(""));
		DVASSERT(materialState.materialName != FastName(""));

		//Logger::FrameworkDebug("Deserialize: %s - %s", materialName.c_str(), materialState.parentName.c_str());
		
		DeserializeFastNameSet(archive->GetArchive("layers"), materialState.layers);
		DeserializeFastNameSet(archive->GetArchive("nativeDefines"), materialState.nativeDefines);
		
		const Map<String, VariantType*>& propsMap = archive->GetArchive("properties")->GetArchieveData();
		for(Map<String, VariantType*>::const_iterator it = propsMap.begin();
			it != propsMap.end();
			++it)
		{
			const VariantType* propVariant = it->second;
			DVASSERT(VariantType::TYPE_BYTE_ARRAY == propVariant->type);
			DVASSERT(propVariant->AsByteArraySize() >= (sizeof(uint32) + sizeof(uint32)));
			
			const uint8* ptr = propVariant->AsByteArray();
			
			materialState.SetPropertyValue(FastName(it->first), *(Shader::eUniformType*)ptr, *(ptr + sizeof(Shader::eUniformType)), ptr + sizeof(Shader::eUniformType) + sizeof(uint8));
		}
		
		const Map<String, VariantType*>& texturesMap = archive->GetArchive("textures")->GetArchieveData();
		for(Map<String, VariantType*>::const_iterator it = texturesMap.begin();
			it != texturesMap.end();
			++it)
		{
			Texture* tex = NULL;
			String relativePathname = it->second->AsString();
			
			if (!relativePathname.empty())
			{
				FilePath texturePath;
				if(relativePathname[0] == '~') //path like ~res:/Gfx...
				{
					texturePath = FilePath(relativePathname);
				}
				else
				{
					texturePath = serializationContext->GetScenePath() + relativePathname;
				}
				
				if(serializationContext->IsDebugLogEnabled())
					Logger::Debug("--- load material texture: %s src:%s", relativePathname.c_str(), texturePath.GetAbsolutePathname().c_str());
				
				tex = Texture::CreateFromFile(texturePath);
			}
			
			materialState.SetTexture(FastName(it->first), tex);
		}
		
		const Map<String, VariantType*>& techniquesMap = archive->GetArchive("techniques")->GetArchieveData();
		for(Map<String, VariantType*>::const_iterator it = techniquesMap.begin();
			it != techniquesMap.end();
			++it)
		{
			KeyedArchive* techniqueArchive = it->second->AsKeyedArchive();
			
			String renderPassName = techniqueArchive->GetString("renderPass");
			String shaderName = techniqueArchive->GetString("shaderName");
			
			FastNameSet techniqueDefines;
			DeserializeFastNameSet(techniqueArchive->GetArchive("defines"), techniqueDefines);
			
			KeyedArchive* renderStateArchive = techniqueArchive->GetArchive("renderState");
			RenderState* renderState = new RenderState();
			renderState->Deserialize(renderStateArchive, serializationContext);
			
			MaterialTechnique* technique = new MaterialTechnique(FastName(shaderName), techniqueDefines, renderState);
			materialState.AddMaterialTechnique(FastName(renderPassName), technique);
        }
	}
	
	void NMaterial::DeserializeFastNameSet(const KeyedArchive* srcArchive, FastNameSet& targetSet)
	{
		const Map<String, VariantType*>& setData = srcArchive->GetArchieveData();
		for(Map<String, VariantType*>::const_iterator it = setData.begin();
			it != setData.end();
			++it)
		{
			targetSet.Insert(FastName(it->first));
		}
	}
	
	void NMaterial::SerializeFastNameSet(const FastNameSet& srcSet, KeyedArchive* targetArchive)
	{
		for(FastNameSet::iterator it = srcSet.begin();
			it != srcSet.end();
			++it)
		{
			targetArchive->SetBool(it->first.c_str(), true);
		}
	}
	
	bool NMaterial::SwitchState(const FastName& stateName,
								MaterialSystem* materialSystem,
								bool forceSwitch)
	{
		if(!forceSwitch && (stateName == currentStateName))
		{
			return true;
		}
		
		NMaterialState* state = states.at(stateName);
		
		if(state != NULL)
		{
			SetParent(NULL);
			
			inheritedDefines.clear();
			effectiveLayers.clear();
			
			//{TODO: these should be removed and changed to a generic system
			//setting properties via special setters
			lightCount = 0;
			materialDynamicLit = false;
			//}END TODO
			
			activeTechniqueName.Reset();
			activeTechnique = NULL;
			ready = false;

			if(currentStateName.IsValid() && !forceSwitch)
			{
				NMaterialState* prevState = states.at(currentStateName);
				DVASSERT(prevState);
				
				//VI: do not copy material name and parent name back
				//VI: since they could be modified by SetParent or other calls
				ShallowCopyTo(prevState, false);
			}
			
			state->ShallowCopyTo(this);
			
			NMaterial* newParent = materialSystem->GetMaterial(parentName);
			DVASSERT(newParent);
			if(NULL == newParent)
			{
				newParent = materialSystem->GetDefaultMaterial();
			}
			
			SetParent(newParent);
			
			currentStateName = stateName;			
			texturesDirty = true;
		}
		
		return (state != NULL);
	}
	
	bool NMaterial::IsSwitchable() const
	{
		return (states.size() > 0);
	}
	
	NMaterial* NMaterial::Clone(const String& newName)
	{
		NMaterial* clonedMaterial = new NMaterial();
		
		if(this->IsSwitchable())
		{
			HashMap<FastName, NMaterialState*>::iterator stateIter = states.begin();
			
			while(stateIter != states.end())
			{
				clonedMaterial->states.insert(stateIter->first,
											  stateIter->second->CloneState());
				
				++stateIter;
			}
		}
		else
		{
			DeepCopyTo(clonedMaterial);
		}
		
		if(newName.length())
		{
			clonedMaterial->SetMaterialName(newName);
		}
		else
		{
			if(IsConfigMaterial())
			{
				clonedMaterial->SetMaterialName(GetMaterialName().c_str());
			}
			else
			{
				clonedMaterial->GenerateName();
			}
		}
		
		clonedMaterial->SetMaterialSystem(materialSystem);
		
		if(clonedMaterial->materialSystem)
		{
			clonedMaterial->materialSystem->AddMaterial(clonedMaterial);
			
			if(clonedMaterial->IsSwitchable())
			{
				clonedMaterial->SwitchState(clonedMaterial->currentStateName, clonedMaterial->materialSystem, true);
			}
			else if(clonedMaterial->parentName.IsValid())
			{
				NMaterial* newParent = clonedMaterial->materialSystem->GetMaterial(clonedMaterial->parentName);
				if(newParent)
				{
					clonedMaterial->SetParent(newParent);
				}
			}
		}
		
        if(illuminationParams)
        {
            IlluminationParams * params = clonedMaterial->GetIlluminationParams();
            params->isUsed = illuminationParams->isUsed;
            params->castShadow = illuminationParams->castShadow;
            params->receiveShadow = illuminationParams->receiveShadow;
            params->lightmapSize = illuminationParams->lightmapSize;
        }
		
		return clonedMaterial;

	}
	
	NMaterial* NMaterial::Clone()
	{
		return Clone("");
	}
    
    void NMaterial::SetMaterialName(const String& name)
    {
        NMaterialState::SetMaterialName(name);
		
		HashMap<FastName, NMaterialState*>::iterator stateIter = states.begin();
		while(stateIter != states.end())
		{
			stateIter->second->SetMaterialName(name);
			++stateIter;
		}
    }
	
	uint32 NMaterial::GetStateCount() const
	{
		return states.size();
	}
	
	NMaterialState* NMaterial::GetState(uint32 index)
	{
		NMaterialState* matState = NULL;
		
		/*uint32 curIndex = 0;
		HashMap<FastName, NMaterialState*>::iterator stateIter = states.begin();
		while(stateIter != states.end() &&
			  curIndex < index)
		{
			++curIndex;
			++stateIter;
		}
		
		matState = stateIter->second;*/
		
		matState = states.valueByIndex((int32)index);

		return matState;
	}
	
	void NMaterial::GenerateName()
	{
		SetMaterialName(Format("%lu", (uint64)this));
	}
	
	void NMaterial::SwitchParent(const FastName& newParent)
	{
		if(materialSystem)
		{
			NMaterial* parent = materialSystem->GetMaterial(newParent);
			
			SetParent(parent);
		}
		else
		{
			parentName = newParent;
		}
		
		DVASSERT(parentName.IsValid());
	}
	
	void NMaterial::SwitchParentForAllStates(const FastName& newParent)
	{
		if(states.size() > 0)
		{
			for(HashMap<FastName, NMaterialState*>::iterator it = states.begin();
				it != states.end();
				++it)
			{
				it->second->SetParentName(newParent.c_str());
			}
		}

		SwitchParent(newParent);
 	}
	
	void NMaterial::BuildTextureParamsCache(const MaterialTechnique& technique)
	{
		textureParamsCache.clear();
		Shader * shader = technique.GetShader();
		
		uint32 uniformCount = shader->GetUniformCount();
		for (uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
		{
			Shader::Uniform * uniform = shader->GetUniform(uniformIndex);
			if (uniform->id == Shader::UNIFORM_NONE)
			{
				if(uniform->type == Shader::UT_SAMPLER_2D ||
				   uniform->type == Shader::UT_SAMPLER_CUBE)
				{
					NMaterialProperty* slotProp = GetMaterialProperty(uniform->name);
					DVASSERT(slotProp); //material has wrong setup if there's no property for sampler
					
					if(slotProp)
					{
						TextureParamCacheEntry entry;
						entry.textureName = uniform->name;
						entry.slot = *(int32*)slotProp->data;
						entry.tx = NULL;
						
						textureParamsCache.push_back(entry);
					}
				}
			}
		}
		
		textureParamsCachePtr = NULL;
		textureParamsCacheSize = 0;
		if(textureParamsCache.size())
		{
			textureParamsCachePtr = &textureParamsCache[0];
			textureParamsCacheSize = textureParamsCache.size();
		}
		
		texturesDirty = true;
	}
	
	void NMaterial::BuildActiveUniformsCache(const MaterialTechnique& technique)
	{
		activeUniformsCache.clear();
		
		Shader * shader = technique.GetShader();
		
		//VI: ugly copypaste
		uint32 cacheTotalSize = 0;
		uint32 uniformCount = shader->GetUniformCount();
		for (uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
		{
			Shader::Uniform * uniform = shader->GetUniform(uniformIndex);
			
			if (Shader::UNIFORM_NONE == uniform->id  ||
				Shader::UNIFORM_COLOR == uniform->id) //TODO: do something with conditional binding
			{
				NMaterialProperty* prop = GetMaterialProperty(uniform->name);
				NMaterialProperty* localProp = materialProperties.at(uniform->name);
				
				//VI: we need to track only properties not set by parent to shader
				if(localProp ||
				   (NULL == localProp && NULL == prop))
				{
					cacheTotalSize += Shader::GetUniformTypeSize(uniform->type) * uniform->size;
				}
			}
		}
		
		uniformDataStorage.resize(cacheTotalSize, 0);
				
		size_t dataOffset = 0;
		for (uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
		{
			Shader::Uniform * uniform = shader->GetUniform(uniformIndex);
						
			if (Shader::UNIFORM_NONE == uniform->id  ||
				Shader::UNIFORM_COLOR == uniform->id) //TODO: do something with conditional binding
			{
				NMaterialProperty* prop = GetMaterialProperty(uniform->name);
				NMaterialProperty* localProp = materialProperties.at(uniform->name);
				
				//VI: we need to track only properties not set by parent to shader
				if(localProp ||
				   (NULL == localProp && NULL == prop))
				{
					if(localProp)
					{
						NMaterialProperty* newProp = new NMaterialState::UniformMaterialProperty();
						newProp->size = uniform->size;
						newProp->type = uniform->type;
						newProp->data = &uniformDataStorage[dataOffset];

						size_t dataSize = Shader::GetUniformTypeSize(localProp->type) * localProp->size;
						DVASSERT(dataSize > 0);
						memcpy(newProp->data, localProp->data, dataSize);
						
						materialProperties.insert(uniform->name, newProp);
						SafeDelete(localProp);
						
						localProp = newProp;
					}
					else
					{
						localProp = new NMaterialState::UniformMaterialProperty();
						localProp->size = uniform->size;
						localProp->type = uniform->type;
						localProp->data = &uniformDataStorage[dataOffset];
						
						materialProperties.insert(uniform->name, localProp);
					}
					
					UniformCacheEntry entry;
					entry.uniform = uniform;
					entry.index = uniformIndex;
					
					entry.propData = localProp->data;

					activeUniformsCache.push_back(entry);
					
					dataOffset += Shader::GetUniformTypeSize(uniform->type) * uniform->size;
				}
			}
		}
		
		activeUniformsCachePtr = NULL;
		activeUniformsCacheSize = 0;
		if(activeUniformsCache.size())
		{
			activeUniformsCachePtr = &activeUniformsCache[0];
			activeUniformsCacheSize = activeUniformsCache.size();

		}
	}
		
	void NMaterial::Rebind(bool recursive)
	{
		size_t childrenCount = NMaterialState::children.size();
		if(childrenCount > 0 ||
		   IsConfigMaterial())
		{
			//VI: need to propagate properties defined for the current material
			//VI: up in the material tree
			
			NMaterial* curMaterial = this;
			while(curMaterial != NULL)
			{
				
				HashMap<FastName, MaterialTechnique *>::iterator iter = curMaterial->techniqueForRenderPass.begin();
				while(iter != curMaterial->techniqueForRenderPass.end())
				{
					MaterialTechnique* technique = iter->second;
					
					Shader * shader = technique->GetShader();
					shader->Bind();
					
					uint32 uniformCount = shader->GetUniformCount();
					for (uint32 uniformIndex = 0; uniformIndex < uniformCount; ++uniformIndex)
					{
						Shader::Uniform * uniform = shader->GetUniform(uniformIndex);
						
						if (Shader::UNIFORM_NONE == uniform->id  ||
							Shader::UNIFORM_COLOR == uniform->id) //TODO: do something with conditional binding
						{
							NMaterialProperty* prop = GetMaterialProperty(uniform->name);
							
							if(prop)
							{
								shader->SetUniformValueByUniform(uniform,
																 uniform->type,
																 uniform->size,
																 prop->data);
							}
						}
					}
					
					shader->Unbind();
					
					++iter;
				}
				
				curMaterial = curMaterial->parent;
			}
			
			if(recursive)
			{
				
				for(size_t i = 0; i < childrenCount; ++i)
				{
					NMaterialState::children[i]->Rebind(recursive);
				}
			}
		}
	}	

    IlluminationParams * NMaterial::GetIlluminationParams()
    {
        if(!illuminationParams)
        {
            illuminationParams = new IlluminationParams();
            illuminationParams->SetDefaultParams();
        }
        return illuminationParams;
    }

    void NMaterial::ReleaseIlluminationParams()
    {
        SafeDelete(illuminationParams);
    }
	
	void NMaterial::OnDirtyTextures()
	{
		TextureStateData stateData;
		for(size_t i = 0; i < textureParamsCacheSize; ++i)
		{
			TextureParamCacheEntry& textureEntry = textureParamsCachePtr[i];
			
			textureEntry.tx = GetTexture(textureEntry.textureName);

			stateData.textures[textureEntry.slot] = textureEntry.tx;
		}
		
		UniqueHandle prevHandle = textureStateHandle;
		
		textureStateHandle = RenderManager::Instance()->AddTextureStateData(&stateData);
		
		//VI: release state handle AFTER adding new
		if(InvalidUniqueHandle != prevHandle)
		{
			RenderManager::Instance()->ReleaseTextureStateData(prevHandle);
		}
	}
	
	///////////////////////////////////////////////////////////////////////////
	///// NMaterialState::NMaterialStateDynamicTexturesInsp implementation
	
	int NMaterialState::NMaterialStateDynamicTexturesInsp::MembersCount(void *object) const
	{
		NMaterialState *state = (NMaterialState*)object;
		DVASSERT(state);
		
		return state->texturesArray.size();
	}
	
	InspDesc NMaterialState::NMaterialStateDynamicTexturesInsp::MemberDesc(void *object, int index) const
	{
		return InspDesc(MemberName(object, index));
	}
	
	const char* NMaterialState::NMaterialStateDynamicTexturesInsp::MemberName(void *object, int index) const
	{
		NMaterialState *state = (NMaterialState*)object;
		DVASSERT(state && index >= 0 && index < state->textureNamesArray.size());
		
		return state->textureNamesArray[index].c_str();
	}
	
	VariantType NMaterialState::NMaterialStateDynamicTexturesInsp::MemberValueGet(void *object, int index) const
	{
		VariantType ret;
		NMaterialState *state = (NMaterialState*)object;
		DVASSERT(state && index >= 0 && index < state->texturesArray.size());
		
		Texture* tex = state->texturesArray[index];
		if(NULL != tex)
		{
			ret.SetFilePath(tex->GetPathname());
		}
		else
		{
			ret.SetFilePath(FilePath());
		}
		
		return ret;
	}
	
	void NMaterialState::NMaterialStateDynamicTexturesInsp::MemberValueSet(void *object, int index, const VariantType &value)
	{
		NMaterialState *state = (NMaterialState *)object;
		DVASSERT(state && index >= 0 && index < state->textureNamesArray.size());
		
		if(state && index >= 0 &&
		   index < state->textureNamesArray.size() &&
		   value.type == VariantType::TYPE_FILEPATH)
		{
			state->SetTexture(state->textureNamesArray[index], Texture::CreateFromFile(value.AsFilePath()));
		}
	}
	
	///////////////////////////////////////////////////////////////////////////
	///// NMaterialState::NMaterialStateDynamicPropertiesInsp implementation
	int NMaterialState::NMaterialStateDynamicPropertiesInsp::MembersCount(void *object) const
	{
		NMaterialState *state = (NMaterialState*)object;
		DVASSERT(state);
		
		return state->materialProperties.size();
	}
	
	InspDesc NMaterialState::NMaterialStateDynamicPropertiesInsp::MemberDesc(void *object, int index) const
	{
		return InspDesc(MemberName(object, index));
	}
	
	const char* NMaterialState::NMaterialStateDynamicPropertiesInsp::MemberName(void *object, int index) const
	{
		NMaterialState *state = (NMaterialState*)object;
		DVASSERT(state && index >= 0 && index < state->materialProperties.size());
		
		const FastName& propName = state->materialProperties.keyByIndex(index);
		return propName.c_str();
	}
	
	VariantType NMaterialState::NMaterialStateDynamicPropertiesInsp::MemberValueGet(void *object, int index) const
	{
		NMaterialState *state = (NMaterialState*)object;
		DVASSERT(state && index >= 0 && index < state->materialProperties.size());
		
		VariantType ret;
		const NMaterialProperty* prop = state->materialProperties.valueByIndex(index);
		switch(prop->type)
		{
			case Shader::UT_FLOAT:
			{
				ret.SetFloat(*(float32*)prop->data);
				break;
			}
				
			case Shader::UT_FLOAT_VEC2:
			{
				ret.SetVector2(*(Vector2*)prop->data);
				break;
			}

			case Shader::UT_FLOAT_VEC3:
			{
				ret.SetVector3(*(Vector3*)prop->data);
				break;
			}

			case Shader::UT_FLOAT_VEC4:
			{
				ret.SetVector4(*(Vector4*)prop->data);
				break;
			}

			case Shader::UT_INT:
			{
				ret.SetInt32(*(int32*)prop->data);
				break;
			}

			case Shader::UT_INT_VEC2:
			case Shader::UT_INT_VEC3:
			case Shader::UT_INT_VEC4:
			{
				DVASSERT(false);
				//TODO: add a way to set int[]
				break;
			}

			case Shader::UT_BOOL:
			{
				ret.SetBool((*(int32*)prop->data) != 0);
				break;
			}

			case Shader::UT_BOOL_VEC2:
			case Shader::UT_BOOL_VEC3:
			case Shader::UT_BOOL_VEC4:
			{
				DVASSERT(false);
				//TODO: add a way to set bool[]
				
				break;
			}

			case Shader::UT_FLOAT_MAT2:
			{
				ret.SetMatrix2(*(Matrix2*)prop->data);
				break;
			}

			case Shader::UT_FLOAT_MAT3:
			{
				ret.SetMatrix3(*(Matrix3*)prop->data);
				break;
			}

			case Shader::UT_FLOAT_MAT4:
			{
				ret.SetMatrix4(*(Matrix4*)prop->data);
				break;
			}

			case Shader::UT_SAMPLER_2D:
			{
				ret.SetInt32(*(int32*)prop->data);
				break;
			}

			case Shader::UT_SAMPLER_CUBE:
			{
				ret.SetInt32(*(int32*)prop->data);
				break;
			}
				
			default:
			{
				DVASSERT(false);
				break;
			}
		}
		
		return ret;
	}
	
	void NMaterialState::NMaterialStateDynamicPropertiesInsp::MemberValueSet(void *object, int index, const VariantType &value)
	{
		NMaterialState *state = (NMaterialState*)object;
		DVASSERT(state && index >= 0 && index < state->materialProperties.size());
		
		//VI: we will only change exisiting properties here
		
		const FastName& propName = state->materialProperties.keyByIndex(index);
		const NMaterialProperty* prop = state->materialProperties.valueByIndex(index);
		switch(prop->type)
		{
			case Shader::UT_FLOAT:
			{
				float32 val = value.AsFloat();
				state->SetPropertyValue(propName, prop->type, prop->size, &val);
				break;
			}
				
			case Shader::UT_FLOAT_VEC2:
			{
				const Vector2& val = value.AsVector2();
				state->SetPropertyValue(propName, prop->type, prop->size, &val);
				break;
			}
				
			case Shader::UT_FLOAT_VEC3:
			{
				const Vector3& val = value.AsVector3();
				state->SetPropertyValue(propName, prop->type, prop->size, &val);
				break;
			}
				
			case Shader::UT_FLOAT_VEC4:
			{
				const Vector4& val = value.AsVector4();
				state->SetPropertyValue(propName, prop->type, prop->size, &val);
				break;
			}
				
			case Shader::UT_INT:
			{
				int32 val = value.AsInt32();
				state->SetPropertyValue(propName, prop->type, prop->size, &val);
				break;
			}
				
			case Shader::UT_INT_VEC2:
			case Shader::UT_INT_VEC3:
			case Shader::UT_INT_VEC4:
			{
				DVASSERT(false);
				//TODO: add a way to set int[]
				break;
			}
				
			case Shader::UT_BOOL:
			{
				bool val = value.AsBool();
				state->SetPropertyValue(propName, prop->type, prop->size, &val);
				break;
			}
				
			case Shader::UT_BOOL_VEC2:
			case Shader::UT_BOOL_VEC3:
			case Shader::UT_BOOL_VEC4:
			{
				DVASSERT(false);
				//TODO: add a way to set bool[]
				
				break;
			}
				
			case Shader::UT_FLOAT_MAT2:
			{
				const Matrix2& val = value.AsMatrix2();
				state->SetPropertyValue(propName, prop->type, prop->size, &val);
				break;
			}
				
			case Shader::UT_FLOAT_MAT3:
			{
				const Matrix3& val = value.AsMatrix3();
				state->SetPropertyValue(propName, prop->type, prop->size, &val);
				break;
			}
				
			case Shader::UT_FLOAT_MAT4:
			{
				const Matrix3& val = value.AsMatrix3();
				state->SetPropertyValue(propName, prop->type, prop->size, &val);
				break;
			}
				
			case Shader::UT_SAMPLER_2D:
			{
				//VI: samplers are set by config materials
				break;
			}
				
			case Shader::UT_SAMPLER_CUBE:
			{
				//VI: samplers are set by config materials
				break;
			}
				
			default:
			{
				DVASSERT(false);
				break;
			}
		}
	}
};
