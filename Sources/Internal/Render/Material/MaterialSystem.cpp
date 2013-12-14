/*==================================================================================
    Copyright (c) 2008, DAVA Consulting, LLC
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the DAVA Consulting, LLC nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE DAVA CONSULTING, LLC AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL DAVA CONSULTING, LLC BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

    Revision History:
        * Created by Vitaliy Borodovsky 
=====================================================================================*/
#include "Render/Material/MaterialSystem.h"
#include "Render/Material/MaterialCompiler.h"
#include "Render/Material/MaterialGraph.h"
#include "Render/Material/MaterialGraphNode.h"
#include "Render/Material/NMaterial.h"
#include "Render/3D/PolygonGroup.h"
#include "FileSystem/FileSystem.h"
#include "Render/Shader.h"
#include "Utils/StringFormat.h"
#include "FileSystem/YamlParser.h"
#include "Render/RenderManager.h"

namespace DAVA
{
	
	MaterialSystem::MaterialSystem() : materials(8192)
{
	defaultMaterial = NULL;
}
	
MaterialSystem::~MaterialSystem()
{
	SafeRelease(defaultMaterial);
	
	Clear();
}
    
NMaterial * MaterialSystem::GetMaterial(const FastName & name)
{
	NMaterial* material = materials.at(name);
		
	//DVASSERT(material);
	if(NULL == material)
	{
		Logger::FrameworkDebug("[MaterialSystem::GetMaterial] Couldn't find material %s! Returning default material.", name.c_str());
		material = defaultMaterial;
	}
	
	return material;
}
	
NMaterial* MaterialSystem::GetSpecificMaterial(const FastName & name)
{
	NMaterial* material = materials.at(name);
		
	return material;
}
	
void MaterialSystem::BuildMaterialList(NMaterial* parent, /*out*/ Vector<NMaterial*>& materialList) const
{
	if(NULL == parent)
	{
		HashMap<FastName, NMaterial*>::iterator mapIter = materials.begin();
		while(mapIter != materials.end())
		{
			NMaterial* material = mapIter->second;
			if(NULL == material->parent)
			{
				materialList.push_back(material);
				BuildMaterialList(material, materialList);
			}
			
			++mapIter;
		}
	}
	else
	{
		for(size_t i = 0; i < parent->NMaterialState::children.size(); ++i)
		{
			NMaterial* material = parent->NMaterialState::children[i];
			materialList.push_back(material);
			BuildMaterialList(material, materialList);
		}
	}
}

bool MaterialSystem::LoadMaterialConfig(const FilePath& filePath)
{
	if(!currentMaterialQuality.IsValid())
	{
		Logger::Error("[MaterialSystem::LoadMaterialConfig] Material quality was not set prior loading material system description. Cannot load the material system!");
		DVASSERT(false);
		
		return false;
	}
	
	YamlParser * parser = YamlParser::Create(filePath);
    if (!parser)
    {
        Logger::Error("[MaterialSystem::LoadMaterialConfig] Can't load material tree configuration: %s", filePath.GetAbsolutePathname().c_str());
        return false;
    }
	
    YamlNode * rootNode = parser->GetRootNode();
    
    if (!rootNode)
    {
		SafeRelease(parser);
        return false;
    }

	String defaultParentName = "";
	Map<String, Vector<MaterialData> > nodes;
	Vector<MaterialData> lodNodes;
	int32 nodeCount = rootNode->GetCount();
	for(int32 i = 0; i < nodeCount; ++i)
	{
		const YamlNode* materialNode = rootNode->Get(i);
		String nodeName = materialNode->AsString();
		if(nodeName == "Material" ||
		   nodeName == "LodMaterial")
		{
			MaterialData data;
			data.isLod = (nodeName == "LodMaterial");
			
			const YamlNode* nameNode = materialNode->Get("Name");
			DVASSERT(nameNode);
			
			if(nameNode)
			{
				data.name = nameNode->AsString();
				
				DVASSERT(data.name.size() > 0);
				if(data.name.size() > 0)
				{
					const YamlNode* pathNode = materialNode->Get("Path");
					DVASSERT(pathNode);
					
					if(pathNode)
					{
						data.path = pathNode->AsString();
						
						DVASSERT(data.path.size() > 0);
						if(data.path.size() > 0)
						{
							//only parent node is optional
							const YamlNode* parentNode = materialNode->Get("Parent");
							if(parentNode)
							{
								data.parent = parentNode->AsString();
							}
							
							if(data.isLod)
							{
								lodNodes.push_back(data);
							}
							else
							{
								Map<String, Vector<MaterialData> >::iterator iter = nodes.find(data.parent);
								if(iter != nodes.end())
								{
									Vector<MaterialData>& dataList = iter->second;
									dataList.push_back(data);
								}
								else
								{
									Vector<MaterialData> dataList;
									dataList.push_back(data);
									nodes[data.parent] = dataList;
								}
							}
						}
						else
						{
							Logger::Error("[MaterialSystem::LoadMaterialConfig] Material node %s in the material configuration has empty Path. Skipping...", data.name.c_str());
						}
					}
					else
					{
						Logger::Error("[MaterialSystem::LoadMaterialConfig] Material node %s in the material configuration doesn't contain Path. Skipping...", data.name.c_str());
					}
				}
				else
				{
					Logger::Error("[MaterialSystem::LoadMaterialConfig] Material node %d in the material configuration has empty Name. Skipping...", i);
				}
			}
			else
			{
				Logger::Error("[MaterialSystem::LoadMaterialConfig] Material node %d in the material configuration doesn't contain Name. Skipping...", i);
			}
		}
		else if(materialNode->AsString() == "DefaultParent")
		{
			DVASSERT(defaultParentName.size() == 0);
			
			if(defaultParentName.size() == 0)
			{
				const YamlNode* nameNode = materialNode->Get("Name");
				DVASSERT(nameNode);
				
				if(nameNode)
				{
					defaultParentName = nameNode->AsString();
				}
				else
				{
					Logger::Error("[MaterialSystem::LoadMaterialConfig] Default material has no name specified! Skipping...");
				}
			}
			else
			{
				Logger::Error("[MaterialSystem::LoadMaterialConfig] Multiple default material names detected! Skipping...");
			}
		}
	}
	
	SafeRelease(parser);
	
	//TODO: validate material tree structure. It shouldn't contain loops or nodes belonging to several roots
	
	//fetch root nodes
	Map<String, Vector<MaterialData> >::iterator rootsIter = nodes.find("");
	DVASSERT(rootsIter != nodes.end());
	if(rootsIter != nodes.end())
	{
		Vector<MaterialData>& roots = rootsIter->second;
		size_t rootCount = roots.size();
		for(size_t i = 0; i < rootCount; ++i)
		{
			MaterialData& currentData = roots[i];
			LoadMaterial(FastName(currentData.name),
						 currentData.path,
						 NULL,
						 currentData.isLod,
						 nodes);
		}
		
		for(size_t i = 0; i < lodNodes.size(); ++i)
		{
			MaterialData& currentData = lodNodes[i];
			LoadMaterial(FastName(currentData.name),
						 currentData.path,
						 NULL,
						 currentData.isLod,
						 nodes);
		}
		
		for(size_t i = 0; i < rootCount; ++i)
		{
			MaterialData& currentData = roots[i];
			NMaterial* rootMaterial = GetMaterial(FastName(currentData.name));
			DVASSERT(rootMaterial);
			
			//rootMaterial->Rebuild();
			//rootMaterial->Rebind();
			
			ScopedPtr<Job> job = JobManager::Instance()->CreateJob(JobManager::THREAD_MAIN,
																   Message(this, &MaterialSystem::BuildAndBindOnMainThread,
																		   rootMaterial));
			JobInstanceWaiter waiter(job);
			waiter.Wait();
		}
	}
	else
	{
		Logger::Error("[MaterialSystem::LoadMaterialConfig] Material config %s contains no root material(s). Skipping...", filePath.GetAbsolutePathname().c_str());
		return false;
	}
	
	defaultMaterial = SafeRetain(GetMaterial(FastName(defaultParentName)));
	DVASSERT(defaultMaterial);
	
	return true;
}
	
void MaterialSystem::BuildAndBindOnMainThread(BaseObject * caller,
											  void * param,
											  void *callerData)
{
	NMaterial* rootMaterial = static_cast<NMaterial*>(param);
	
	rootMaterial->Rebuild();
	rootMaterial->Rebind();
}
	
NMaterial* MaterialSystem::LoadMaterial(const FastName& name,
										const FilePath& filePath,
										NMaterial* parentMaterial,
										bool isLod,
										Map<String, Vector<MaterialData> >& nodes)
{
	NMaterial* material = new NMaterial();
	material->configMaterial = true;
	bool result = material->LoadFromFile(filePath);
	
	DVASSERT(result);
	if(result)
	{
		material->SetMaterialName(name.c_str());
		AddMaterial(material);
		//material->Release(); //need to release material since material system took ownership
		material->SetParent(parentMaterial);
		
		Map<String, Vector<MaterialData> >::iterator childrenIter = nodes.find(material->GetMaterialName().c_str());
		if(childrenIter != nodes.end())
		{
			Vector<MaterialData>& materials = childrenIter->second;
			size_t materialCount = materials.size();
			for(size_t i = 0; i < materialCount; ++i)
			{
				MaterialData& currentData = materials[i];
				LoadMaterial(FastName(currentData.name),
							 currentData.path,
							 material,
							 currentData.isLod,
							 nodes);
			}
		}
	}
	else
	{
		Logger::Error("[MaterialSystem::LoadMaterialConfig] Failed to load material %s", filePath.GetAbsolutePathname().c_str());
		SafeRelease(material);
	}
	
	return material;
}
	
	void MaterialSystem::AddMaterial(NMaterial* material)
	{
		DVASSERT(material);
		//SafeRetain(material);
		
		material->SetMaterialSystem(this);
		
		NMaterial* collisionMaterial = materials.at(material->GetMaterialName());
		DVASSERT(material != collisionMaterial); //should not add same material several times
		if(collisionMaterial != NULL &&
		   collisionMaterial != material)
		{
			//need to resolve name collision
			//just add some number to the end
			uint32 i = 0;
			String baseName = material->GetMaterialName().c_str();
			while(true)
			{
				String uniqueName = Format("%s.%d", baseName.c_str(), i);
				if(!materials.count(FastName(uniqueName.c_str())))
				{
					material->SetMaterialName(uniqueName);
					break;
				}
				
				i++;
				
				DVASSERT(i <= 65536);//something is wrong when there's no unique name after 64K steps
			}
		}
		
		materials.insert(material->GetMaterialName(), material);
		
		if(material->IsSwitchable())
		{
			material->SwitchState(currentMaterialQuality, this);
		}
	}

void MaterialSystem::RemoveMaterial(NMaterial* material)
{
	DVASSERT(material);
	materials.erase(material->GetMaterialName());
	//SafeRelease(material);
}
	
void MaterialSystem::Clear()
{
	//remove all materials in correct order - from leaves to root
	
	Vector<NMaterial*> materialList;
	BuildMaterialList(NULL, materialList);
	
	for(Vector<NMaterial*>::reverse_iterator it = materialList.rbegin();
		it != materialList.rend();
		++it)
	{
		RemoveMaterial(*it);
	}
	
	
	
	DVASSERT(materials.size() == 0);
}
	
	void MaterialSystem::SetDefaultMaterialQuality(const FastName& qualityLevelName)
	{
		defaultMaterialQuality = qualityLevelName;
		
		if(!currentMaterialQuality.IsValid())
		{
			currentMaterialQuality = defaultMaterialQuality;
		}
	}
	
	const FastName& MaterialSystem::GetDefaultMaterialQuality() const
	{
		return defaultMaterialQuality;
	}
	
	const FastName& MaterialSystem::GetCurrentMaterialQuality() const
	{
		return currentMaterialQuality;
	}

	
void MaterialSystem::SwitchMaterialQuality(const FastName& qualityLevelName,
										   bool forceSwitch)
{
	Vector<NMaterial*> materialList;
	BuildMaterialList(NULL, materialList);
	
	currentMaterialQuality = qualityLevelName;
	
	for(Vector<NMaterial*>::iterator it = materialList.begin();
		it != materialList.end();
		++it)
	{
		NMaterial* mat = *it;
		
		if(mat->IsSwitchable())
		{
			SafeRetain(mat);
			mat->SwitchState(qualityLevelName, this, forceSwitch);
			SafeRelease(mat);
		}
	}
	
	for(Vector<NMaterial*>::iterator it = materialList.begin();
		it != materialList.end();
		++it)
	{
		NMaterial* mat = *it;
		if(mat->GetParent() == NULL) //parent == NULL means it's root material
		{
			mat->Rebuild();
			mat->Rebind();
		}
	}
}
	
NMaterial* MaterialSystem::CreateInstanceChild(NMaterial* parent)
{
	NMaterial* child = MaterialSystem::CreateNamed();
	
	child->SetParent(parent);
	
	return child;
}
	
	NMaterial* MaterialSystem::CreateInstanceChild(const FastName& parentName)
	{
		NMaterial* parent = GetMaterial(parentName);
		return CreateInstanceChild(parent);
	}
	
	void MaterialSystem::BindMaterial(NMaterial* newMaterial)
	{
		AddMaterial(newMaterial);
		
		FastName parentName = newMaterial->GetParentName();
		NMaterial* newParent = GetMaterial(parentName);
		if(newParent)
		{
			newMaterial->SetParent(newParent);
		}
	}
	
	NMaterial* MaterialSystem::CreateSwitchableChild(NMaterial* parent)
	{
		NMaterial* child = MaterialSystem::CreateNamed();
		uint32 stateCount = parent->GetStateCount();
		String matName = child->GetMaterialName().c_str();
		
		if(stateCount > 0)
		{
			HashMap<FastName, NMaterialState*>::iterator stateIter = parent->states.begin();
			while(stateIter != parent->states.end())
			{
				NMaterialState* matState = stateIter->second->CreateTemplate(parent);
				matState->SetMaterialName(matName);
				child->states.insert(stateIter->first, matState);
				
				++stateIter;
			}
		}
		
		child->SetParent(parent);
		
		return child;
	}
	
	NMaterial* MaterialSystem::CreateSwitchableChild(const FastName& parentName)
	{
		NMaterial* parent = GetMaterial(parentName);
		return CreateSwitchableChild(parent);
	}
	
	NMaterial* MaterialSystem::CreateNamed()
	{
		NMaterial* mat = new NMaterial();
		mat->GenerateName();
		
		return mat;
	}
};

