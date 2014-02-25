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


#include "Scene3D/SceneFileV2.h"
#include "Scene3D/Entity.h"
#include "Scene3D/MeshInstanceNode.h"
#include "Render/Texture.h"
#include "Render/Material.h"
#include "Render/3D/AnimatedMesh.h"
#include "Scene3D/PathManip.h"
#include "Scene3D/SkeletonNode.h"
#include "Scene3D/BoneNode.h"
#include "Scene3D/SwitchNode.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Mesh.h"

#include "Scene3D/SceneNodeAnimationList.h"
#include "Scene3D/LodNode.h"
#include "Scene3D/Systems/TransformSystem.h"
#include "Scene3D/Components/LodComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/ParticleEmitterNode.h"
#include "Scene3D/ParticleEffectNode.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Components/UserComponent.h"
#include "Scene3D/ShadowVolumeNode.h"
#include "Scene3D/UserNode.h"

#include "Utils/StringFormat.h"
#include "FileSystem/FileSystem.h"
#include "Base/ObjectFactory.h"
#include "Base/TemplateHelpers.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/ShadowVolume.h"

#include "Scene3D/SpriteNode.h"
#include "Render/Highlevel/SpriteObject.h"

#include "Scene3D/Components/CustomPropertiesComponent.h"

#include "Scene3D/Scene.h"


namespace DAVA
{
    
SceneFileV2::SceneFileV2()
{
    isDebugLogEnabled = false;
    isSaveForGame = false;
    lastError = ERROR_NO_ERROR;

	UserNode *n = new UserNode();
	n->Release();
}

SceneFileV2::~SceneFileV2()
{
}
    
const FilePath SceneFileV2::GetScenePath()
{
    return FilePath(rootNodePathName.GetDirectory());
}
        
    
void SceneFileV2::EnableSaveForGame(bool _isSaveForGame)
{
    isSaveForGame = _isSaveForGame;
}

void SceneFileV2::EnableDebugLog(bool _isDebugLogEnabled)
{
    isDebugLogEnabled = _isDebugLogEnabled;
}

bool SceneFileV2::DebugLogEnabled()
{
    return isDebugLogEnabled;
}
    
Material * SceneFileV2::GetMaterial(int32 index)
{
    return materials[index];
}
    
StaticMesh * SceneFileV2::GetStaticMesh(int32 index)
{
    return staticMeshes[index];
}

DataNode * SceneFileV2::GetNodeByPointer(uint64 pointer)
{
    Map<uint64, DataNode*>::iterator it = dataNodes.find(pointer);
    if (it != dataNodes.end())
    {
        return it->second;
    }
    return 0;
}

int32 SceneFileV2::GetVersion()
{
    return header.version;
}
    
void SceneFileV2::SetError(eError error)
{
    lastError = error;
}

SceneFileV2::eError SceneFileV2::GetError()
{
    return lastError;
}

SceneFileV2::eError SceneFileV2::SaveScene(const FilePath & filename, DAVA::Scene *_scene)
{
    File * file = File::Create(filename, File::CREATE | File::WRITE);
    if (!file)
    {
        Logger::Error("SceneFileV2::SaveScene failed to create file: %s", filename.GetAbsolutePathname().c_str());
        SetError(ERROR_FAILED_TO_CREATE_FILE);
        return GetError();
    }
    
    rootNodePathName = filename;

    // save header
    header.signature[0] = 'S';
    header.signature[1] = 'F';
    header.signature[2] = 'V';
    header.signature[3] = '2';
    
	//VI: version = 7 is in the feature-new-materials branch
    header.version = 8;
    header.nodeCount = _scene->GetChildrenCount();
    
    file->Write(&header, sizeof(Header));
    
    // save data objects
    if(isDebugLogEnabled)
    {
        Logger::FrameworkDebug("+ save data objects");
        Logger::FrameworkDebug("- save file path: %s", rootNodePathName.GetDirectory().GetAbsolutePathname().c_str());
    }
    
//    // Process file paths
//    for (int32 mi = 0; mi < _scene->GetMaterials()->GetChildrenCount(); ++mi)
//    {
//        Material * material = dynamic_cast<Material*>(_scene->GetMaterials()->GetChild(mi));
//        for (int k = 0; k < Material::TEXTURE_COUNT; ++k)
//        {
//            if (material->names[k].length() > 0)
//            {
//                replace(material->names[k], rootNodePath, String(""));
//                Logger::FrameworkDebug("- preprocess mat path: %s rpn: %s", material->names[k].c_str(), material->textures[k]->relativePathname.c_str());
//            }
//        }   
//    }
    
//    SaveDataHierarchy(_scene->GetMaterials(), file, 1);
//    SaveDataHierarchy(_scene->GetStaticMeshes(), file, 1);

    List<DataNode*> nodes;
	if (isSaveForGame)
		_scene->OptimizeBeforeExport();
    _scene->GetDataNodes(nodes);
    int32 dataNodesCount = (int32)nodes.size();
    file->Write(&dataNodesCount, sizeof(int32));
    for (List<DataNode*>::iterator it = nodes.begin(); it != nodes.end(); ++it)
        SaveDataNode(*it, file);
    
    // save hierarchy
    if(isDebugLogEnabled)
        Logger::FrameworkDebug("+ save hierarchy");

    for (int ci = 0; ci < header.nodeCount; ++ci)
    {
        if (!SaveHierarchy(_scene->GetChild(ci), file, 1))
        {
            Logger::Error("SceneFileV2::SaveScene failed to save hierarchy file: %s", filename.GetAbsolutePathname().c_str());
            SafeRelease(file);
            return GetError();
        }
    }
    
    SafeRelease(file);
    return GetError();
};	
    
SceneFileV2::eError SceneFileV2::LoadScene(const FilePath & filename, Scene * _scene)
{
    File * file = File::Create(filename, File::OPEN | File::READ);
    if (!file)
    {
        Logger::Error("SceneFileV2::LoadScene failed to create file: %s", filename.GetAbsolutePathname().c_str());
        SetError(ERROR_FAILED_TO_CREATE_FILE);
        return GetError();
    }   

    scene = _scene;
    rootNodePathName = filename;

    file->Read(&header, sizeof(Header));
    int requiredVersion = 3;
    if (    (header.signature[0] != 'S') 
        ||  (header.signature[1] != 'F') 
        ||  (header.signature[2] != 'V') 
        ||  (header.signature[3] != '2'))
    {
        Logger::Error("SceneFileV2::LoadScene header version is wrong: %d, required: %d", header.version, requiredVersion);
        
        SafeRelease(file);
        SetError(ERROR_VERSION_IS_TOO_OLD);
        return GetError();
    }
    
    if(isDebugLogEnabled)
        Logger::FrameworkDebug("+ load data objects");

    if (GetVersion() >= 2)
    {
        int32 dataNodeCount = 0;
        file->Read(&dataNodeCount, sizeof(int32));
        
        for (int k = 0; k < dataNodeCount; ++k)
            LoadDataNode(0, file);
    }
    
    if(isDebugLogEnabled)
        Logger::FrameworkDebug("+ load hierarchy");
        
    Entity * rootNode = new Entity();
    rootNode->SetName(rootNodePathName.GetFilename());
	rootNode->SetScene(0);
    for (int ci = 0; ci < header.nodeCount; ++ci)
    {
        LoadHierarchy(0, rootNode, file, 1);
    }
		    
    OptimizeScene(rootNode);	
    
	rootNode->SceneDidLoaded();
    
    if (GetError() == ERROR_NO_ERROR)
    {
        // TODO: Check do we need to releae root node here
        _scene->AddRootNode(rootNode, rootNodePathName);
    }
    else
    {
        SafeRelease(rootNode);
    }
    
    for (size_t mi = 0; mi < materials.size(); ++mi)
    {
        SafeRelease(materials[mi]);
    }
    materials.clear();
    
    for (size_t mi = 0; mi < staticMeshes.size(); ++mi)
    {
        SafeRelease(staticMeshes[mi]);
    }
    staticMeshes.clear();
    
    for (Map<uint64, DataNode*>::iterator it = dataNodes.begin(); it != dataNodes.end(); ++it)
    {
        SafeRelease(it->second);
    }
    dataNodes.clear();
    
    SafeRelease(rootNode);
    SafeRelease(file);
    return GetError();
}

bool SceneFileV2::SaveDataNode(DataNode * node, File * file)
{
    KeyedArchive * archive = new KeyedArchive();
    if (isDebugLogEnabled)
        Logger::FrameworkDebug("- %s(%s)", node->GetName().c_str(), node->GetClassName().c_str());
    
    
    node->Save(archive, this);  
    archive->SetInt32("#childrenCount", node->GetChildrenCount());
    archive->Save(file);
    
    for (int ci = 0; ci < node->GetChildrenCount(); ++ci)
    {
        DataNode * child = node->GetChild(ci);
        SaveDataNode(child, file);
    }
    
    SafeRelease(archive);
    return true;
}
    
void SceneFileV2::LoadDataNode(DataNode * parent, File * file)
{
    KeyedArchive * archive = new KeyedArchive();
    archive->Load(file);
    
    String name = archive->GetString("##name");
    DataNode * node = dynamic_cast<DataNode *>(ObjectFactory::Instance()->New(name));
    
    if (node)
    {
        if (node->GetClassName() == "DataNode")
        {
            SafeRelease(node);
            return;
        }   
        node->SetScene(scene);
        
        if (isDebugLogEnabled)
        {
            String name = archive->GetString("name");
            Logger::FrameworkDebug("- %s(%s)", name.c_str(), node->GetClassName().c_str());
        }
        node->Load(archive, this);
        AddToNodeMap(node);
        
        if (parent)
            parent->AddNode(node);
        
        int32 childrenCount = archive->GetInt32("#childrenCount", 0);
        for (int ci = 0; ci < childrenCount; ++ci)
        {
            LoadDataNode(node, file);
        }
        
        SafeRelease(node);
    }
    SafeRelease(archive);
}

bool SceneFileV2::SaveDataHierarchy(DataNode * node, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    if (isDebugLogEnabled)
        Logger::FrameworkDebug("%s %s(%s)", GetIndentString('-', level), node->GetName().c_str(), node->GetClassName().c_str());

    node->Save(archive, this);    
    
    
    archive->SetInt32("#childrenCount", node->GetChildrenCount());
    archive->Save(file);
    
	for (int ci = 0; ci < node->GetChildrenCount(); ++ci)
	{
		DataNode * child = node->GetChild(ci);
		SaveDataHierarchy(child, file, level + 1);
	}
    
    SafeRelease(archive);
    return true;
}

void SceneFileV2::LoadDataHierarchy(Scene * scene, DataNode * root, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    archive->Load(file);
    
    // DataNode * node = dynamic_cast<DataNode*>(BaseObject::LoadFromArchive(archive));
    
    String name = archive->GetString("##name");
    DataNode * node = dynamic_cast<DataNode *>(ObjectFactory::Instance()->New(name));

    if (node)
    {
        if (node->GetClassName() == "DataNode")
        {
            SafeRelease(node);
            node = SafeRetain(root); // retain root here because we release it at the end
        }  
        
        node->SetScene(scene);
        
        // TODO: Rethink here
        Material * material = dynamic_cast<Material*>(node);
        if (material)
        {
            materials.push_back(SafeRetain(material));
        }
        StaticMesh * staticMesh = dynamic_cast<StaticMesh*>(node);
        if (staticMesh)
        {
            staticMeshes.push_back(SafeRetain(staticMesh));
        }
        if (isDebugLogEnabled)
        {
            String name = archive->GetString("name");
            Logger::FrameworkDebug("%s %s(%s)", GetIndentString('-', level), name.c_str(), node->GetClassName().c_str());
        }
        node->Load(archive, this);
        
        
        AddToNodeMap(node);
        
        if (node != root)
            root->AddNode(node);
        
        int32 childrenCount = archive->GetInt32("#childrenCount", 0);
        for (int ci = 0; ci < childrenCount; ++ci)
        {
            LoadDataHierarchy(scene, node, file, level + 1);
        }
        SafeRelease(node);
    }
    
    SafeRelease(archive);
}
    
void SceneFileV2::AddToNodeMap(DataNode * node)
{
    uint64 ptr = node->GetPreviousPointer();
    
    if(isDebugLogEnabled)
        Logger::FrameworkDebug("* add ptr: %llx class: %s(%s)", ptr, node->GetName().c_str(), node->GetClassName().c_str());
    
    dataNodes[ptr] = SafeRetain(node);
}
    
bool SceneFileV2::SaveHierarchy(Entity * node, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    if (isDebugLogEnabled)
        Logger::FrameworkDebug("%s %s(%s) %d", GetIndentString('-', level), node->GetName().c_str(), node->GetClassName().c_str(), node->GetChildrenCount());
    node->Save(archive, this);    
    
	archive->SetInt32("#childrenCount", node->GetChildrenCount());
 
    archive->Save(file);

	for (int ci = 0; ci < node->GetChildrenCount(); ++ci)
	{
		Entity * child = node->GetChild(ci);
		SaveHierarchy(child, file, level + 1);
	}
    
    SafeRelease(archive);
    return true;
}

void SceneFileV2::LoadHierarchy(Scene * scene, Entity * parent, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    archive->Load(file);
    //Entity * node = dynamic_cast<Entity*>(BaseObject::LoadFromArchive(archive));
    
    String name = archive->GetString("##name");
    BaseObject * baseObject = 0;
    Entity * node = 0;
    
    bool skipNode = false;
    bool removeChildren = false;
    
    if (name == "LandscapeNode")
    {
        node = new Entity();
        baseObject = node;

        node->SetScene(scene);
        node->Load(archive, this);
        
        Landscape * landscapeRenderObject = new Landscape();
        landscapeRenderObject->Load(archive, this);
        
        node->AddComponent(ScopedPtr<RenderComponent> (new RenderComponent(landscapeRenderObject)));

        parent->AddNode(node);
        
        SafeRelease(landscapeRenderObject);
        // Elegant fix became part of architecture....
        skipNode = true;
    }else if (name == "Camera")
    {
        node = new Entity();
        baseObject = node;
        
        node->SetScene(scene);
        node->Load(archive, this);
        
        Camera * cameraObject = new Camera();
        cameraObject->Load(archive);
        
        node->AddComponent(ScopedPtr<CameraComponent> (new CameraComponent(cameraObject)));
        parent->AddNode(node);
        
        SafeRelease(cameraObject);
        skipNode = true;
    }else if ((name == "LightNode"))// || (name == "EditorLightNode"))
    {
        node = new Entity();
        baseObject = node;
        
        node->SetScene(scene);
        node->Load(archive, this);
        
        bool isDynamic = node->GetCustomProperties()->GetBool("editor.dynamiclight.enable", true);
        
        Light * light = new Light();
        light->Load(archive, this);
        light->SetDynamic(isDynamic);
        
        node->AddComponent(ScopedPtr<LightComponent> (new LightComponent(light)));
        parent->AddNode(node);
        
        SafeRelease(light);
        skipNode = true;
        removeChildren = true;
    }
	else if(name == "SceneNode")
	{
		node = new Entity();
		baseObject = node;
	}
	else
    {
        baseObject = ObjectFactory::Instance()->New(name);
        node = dynamic_cast<Entity*>(baseObject);
    }

	//TODO: refactor this elegant fix
	if(!node) //in case if editor class is loading in non-editor sprsoject
	{
		node = new Entity();
		skipNode = true;
	}

    //if(node)
    {
        if (isDebugLogEnabled)
        {
            String name = archive->GetString("name");
            Logger::FrameworkDebug("%s %s(%s)", GetIndentString('-', level), name.c_str(), node->GetClassName().c_str());
        }

		if(!skipNode)
		{
			node->SetScene(scene);
			node->Load(archive, this);
            
            //ReplaceNodeAfterLoad(&node);
            
			parent->AddNode(node);
		}
        
		int32 childrenCount = archive->GetInt32("#childrenCount", 0);

		for (int ci = 0; ci < childrenCount; ++ci)
		{
			LoadHierarchy(scene, node, file, level + 1);
		}
        if (removeChildren)
            node->RemoveAllChildren();

        SafeRelease(node);
    }
    
    SafeRelease(archive);
}


void SceneFileV2::ConvertShadows(Entity * currentNode)
{
	for(int32 c = 0; c < currentNode->GetChildrenCount(); ++c)
	{
		Entity * childNode = currentNode->GetChild(c);
		if(String::npos != childNode->GetName().find("_shadow"))
		{
			DVASSERT(childNode->GetChildrenCount() == 1);
			Entity * svn = childNode->FindByName("dynamicshadow.shadowvolume");
			if(!svn)
			{
				MeshInstanceNode * mi = dynamic_cast<MeshInstanceNode*>(childNode->GetChild(0));
				DVASSERT(mi);
				mi->ConvertToShadowVolume();
				childNode->RemoveNode(mi);
			}
		}
		else
		{
			ConvertShadows(childNode);
		}
	}
}
    
bool SceneFileV2::RemoveEmptySceneNodes(DAVA::Entity * currentNode)
{
    for (int32 c = 0; c < currentNode->GetChildrenCount(); ++c)
    {
        Entity * childNode = currentNode->GetChild(c);
        bool dec = RemoveEmptySceneNodes(childNode);
        if(dec)c--;
    }
    if ((currentNode->GetChildrenCount() == 0) && (typeid(*currentNode) == typeid(Entity)))
    {
        KeyedArchive *customProperties = currentNode->GetCustomProperties();
        bool doNotRemove = customProperties && customProperties->IsKeyExists("editor.donotremove");
        
        uint32 componentCount = currentNode->GetComponentCount();

        if ((componentCount > 0 && (0 == currentNode->GetComponent(Component::TRANSFORM_COMPONENT))) //has only component, not transform
			|| componentCount > 1)
        {
            doNotRemove = true;
        }
        
        if (!doNotRemove)
        {
            Entity * parent  = currentNode->GetParent();
            if (parent)
            {
                parent->RemoveNode(currentNode);
                removedNodeCount++;
                return true;
            }
        }
    }
    return false;
}
    
bool SceneFileV2::RemoveEmptyHierarchy(Entity * currentNode)
{
    for (int32 c = 0; c < currentNode->GetChildrenCount(); ++c)
    {
        Entity * childNode = currentNode->GetChild(c);

        bool dec = RemoveEmptyHierarchy(childNode);
        if(dec)c--;
    }
    
    if(currentNode->GetChildrenCount() == 1)
    {
		uint32 allowed_comp_count = 0;
		if(NULL != currentNode->GetComponent(Component::TRANSFORM_COMPONENT))
		{
			allowed_comp_count++;
		}

		if(NULL != currentNode->GetComponent(Component::CUSTOM_PROPERTIES_COMPONENT))
		{
			allowed_comp_count++;
		}

		if (currentNode->GetComponentCount() > allowed_comp_count)
		{
            return false;
		}
        
        if (currentNode->GetFlags() & Entity::NODE_LOCAL_MATRIX_IDENTITY)
        {
            Entity * parent  = currentNode->GetParent();

            if (parent)
            {
                Entity * childNode = SafeRetain(currentNode->GetChild(0));
                String currentName = currentNode->GetName();
				KeyedArchive * currentProperties = currentNode->GetCustomProperties();
                
                //Logger::FrameworkDebug("remove node: %s %p", currentNode->GetName().c_str(), currentNode);
				parent->InsertBeforeNode(childNode, currentNode);
                
                childNode->SetName(currentName);
				//merge custom properties
				KeyedArchive * newProperties = childNode->GetCustomProperties();
				const Map<String, VariantType*> & oldMap = currentProperties->GetArchieveData();
				Map<String, VariantType*>::const_iterator itEnd = oldMap.end();
				for(Map<String, VariantType*>::const_iterator it = oldMap.begin(); it != itEnd; ++it)
				{
					newProperties->SetVariant(it->first, *it->second);
				}
				
				//VI: remove node after copying its properties since properties become invalid after node removal
				parent->RemoveNode(currentNode);
				
                removedNodeCount++;
                SafeRelease(childNode);
				
                return true;
            }
            //RemoveEmptyHierarchy(childNode);
        }
    }
    return false;
}

    
bool SceneFileV2::ReplaceNodeAfterLoad(Entity * node)
{
    MeshInstanceNode * oldMeshInstanceNode = dynamic_cast<MeshInstanceNode*>(node);
    if (oldMeshInstanceNode)
    {
        Vector<PolygonGroupWithMaterial*> polygroups = oldMeshInstanceNode->GetPolygonGroups();

        for (uint32 k = 0; k < (uint32)polygroups.size(); ++k)
        {
            PolygonGroupWithMaterial * group = polygroups[k];
            if (group->GetMaterial()->type == Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP)
            {
                if (oldMeshInstanceNode->GetLightmapCount() == 0)
                {
                    Logger::FrameworkDebug(Format("%s - lightmaps:%d", oldMeshInstanceNode->GetFullName().c_str(), 0));
                }
                
                //DVASSERT(oldMeshInstanceNode->GetLightmapCount() > 0);
                //DVASSERT(oldMeshInstanceNode->GetLightmapDataForIndex(0)->lightmap != 0)
            }
        }
        Entity * newMeshInstanceNode = new Entity();
        oldMeshInstanceNode->Entity::Clone(newMeshInstanceNode);

		Component *clonedComponent = oldMeshInstanceNode->GetComponent(Component::TRANSFORM_COMPONENT)->Clone(newMeshInstanceNode);
        newMeshInstanceNode->RemoveComponent(clonedComponent->GetType());
        newMeshInstanceNode->AddComponent(clonedComponent);
		clonedComponent->Release();
        
        //Vector<PolygonGroupWithMaterial*> polygroups = oldMeshInstanceNode->GetPolygonGroups();
        
        Mesh * mesh = new Mesh();
        
        for (uint32 k = 0; k < (uint32)polygroups.size(); ++k)
        {
            PolygonGroupWithMaterial * group = polygroups[k];
            mesh->AddPolygonGroup(group->GetPolygonGroup(), group->GetMaterial());
            
            
            if (group->GetMaterial()->type == Material::MATERIAL_UNLIT_TEXTURE_LIGHTMAP)
            {
//                if (oldMeshInstanceNode->GetLightmapCount() == 0)
//                {
//                    Logger::FrameworkDebug(Format("%s - lightmaps:%d", oldMeshInstanceNode->GetFullName().c_str(), 0));
//                }
                
                //DVASSERT(oldMeshInstanceNode->GetLightmapCount() > 0);
                //DVASSERT(oldMeshInstanceNode->GetLightmapDataForIndex(0)->lightmap != 0)
            }
            
            if (oldMeshInstanceNode->GetLightmapCount() > 0)
            {
                RenderBatch * batch = mesh->GetRenderBatch(k);
                batch->GetMaterialInstance()->SetLightmap(oldMeshInstanceNode->GetLightmapDataForIndex(k)->lightmap,
                                                          oldMeshInstanceNode->GetLightmapDataForIndex(k)->lightmapName);
                batch->GetMaterialInstance()->SetUVOffsetScale(oldMeshInstanceNode->GetLightmapDataForIndex(k)->uvOffset,
                                                               oldMeshInstanceNode->GetLightmapDataForIndex(k)->uvScale);
            }
        }
        
        mesh->SetOwnerDebugInfo(oldMeshInstanceNode->GetName());
        
        //
        Entity * parent = oldMeshInstanceNode->GetParent();
        for (int32 k = 0; k < parent->GetChildrenCount(); ++k)
        {
            ShadowVolumeNode * oldShadowVolumeNode = dynamic_cast<ShadowVolumeNode*>(parent->GetChild(k));
            if (oldShadowVolumeNode)
            {
                ShadowVolume * newShadowVolume = new ShadowVolume();
				PolygonGroup * pg = oldShadowVolumeNode->GetPolygonGroup();
				Matrix4 matrix = oldMeshInstanceNode->GetLocalTransform();
				if(matrix != Matrix4::IDENTITY)
				{
					matrix.Inverse();
					pg->ApplyMatrix(matrix);
					pg->BuildBuffers();
				}

                newShadowVolume->SetPolygonGroup(pg);
                mesh->AddRenderBatch(newShadowVolume);
                
                mesh->SetOwnerDebugInfo(oldMeshInstanceNode->GetName() + " shadow:" + oldShadowVolumeNode->GetName());
                
                parent->RemoveNode(oldShadowVolumeNode);
                SafeRelease(newShadowVolume);
            }
        }
        
        
        
        
        RenderComponent * renderComponent = new RenderComponent;
        renderComponent->SetRenderObject(mesh);
        newMeshInstanceNode->AddComponent(renderComponent);
		renderComponent->Release();
        
		if(parent)
		{
			parent->InsertBeforeNode(newMeshInstanceNode, oldMeshInstanceNode);
			parent->RemoveNode(oldMeshInstanceNode);
		}
		else
		{
			DVASSERT(0 && "How we appeared here");
		}
		newMeshInstanceNode->Release();
		mesh->Release();
        return true;
    }

	LodNode * lod = dynamic_cast<LodNode*>(node);
	if(lod)
	{
		Entity * newNode = new Entity();
		lod->Entity::Clone(newNode);
		Entity * parent = lod->GetParent();

		newNode->AddComponent(ScopedPtr<LodComponent> (new LodComponent()));
		LodComponent * lc = DynamicTypeCheck<LodComponent*>(newNode->GetComponent(Component::LOD_COMPONENT));

		for(int32 iLayer = 0; iLayer < LodComponent::MAX_LOD_LAYERS; ++iLayer)
		{
			lc->lodLayersArray[iLayer].distance = lod->GetLodLayerDistance(iLayer);
			lc->lodLayersArray[iLayer].nearDistanceSq = lod->GetLodLayerNearSquare(iLayer);
			lc->lodLayersArray[iLayer].farDistanceSq = lod->GetLodLayerFarSquare(iLayer);
		}

		List<LodNode::LodData*> oldLodData;
		lod->GetLodData(oldLodData);
		for(List<LodNode::LodData*>::iterator it = oldLodData.begin(); it != oldLodData.end(); ++it)
		{
			LodNode::LodData * oldDataItem = *it;
			LodComponent::LodData newLodDataItem;
			newLodDataItem.indexes = oldDataItem->indexes;
			newLodDataItem.isDummy = oldDataItem->isDummy;
			newLodDataItem.layer = oldDataItem->layer;
			newLodDataItem.nodes = oldDataItem->nodes;

			lc->lodLayers.push_back(newLodDataItem);
		}

		DVASSERT(parent);
		if(parent)
		{
			parent->InsertBeforeNode(newNode, lod);
			parent->RemoveNode(lod);
		}

		//GlobalEventSystem::Instance()->Event(newNode, )
		//newNode->GetScene()->transformSystem->ImmediateEvent(newNode, EventSystem::LOCAL_TRANSFORM_CHANGED);
		newNode->Release();
		return true;
	}

	ParticleEmitterNode * particleEmitterNode = dynamic_cast<ParticleEmitterNode*>(node);
	if(particleEmitterNode)
	{
		Entity * newNode = new Entity();
		particleEmitterNode->Entity::Clone(newNode);
		Entity * parent = particleEmitterNode->GetParent();

		ParticleEmitter * emitter = particleEmitterNode->GetEmitter();
		RenderComponent * renderComponent = new RenderComponent();
		newNode->AddComponent(renderComponent);
		renderComponent->SetRenderObject(emitter);
		renderComponent->Release();
		
		DVASSERT(parent);
		if(parent)
		{
			parent->InsertBeforeNode(newNode, particleEmitterNode);
			parent->RemoveNode(particleEmitterNode);
		}

		newNode->Release();
		return true;
	}

	ParticleEffectNode * particleEffectNode = dynamic_cast<ParticleEffectNode*>(node);
	if(particleEffectNode)
	{
		Entity * newNode = new Entity();
		particleEffectNode->Entity::Clone(newNode);
		Entity * parent = particleEffectNode->GetParent();

		DVASSERT(parent);
		if(parent)
		{
			parent->InsertBeforeNode(newNode, particleEffectNode);
			parent->RemoveNode(particleEffectNode);
		}

		newNode->AddComponent(ScopedPtr<ParticleEffectComponent> (new ParticleEffectComponent()));
		newNode->Release();
		return true;
	}

	SwitchNode * sw = dynamic_cast<SwitchNode*>(node);
	if(sw)
	{
		Entity * newNode = new Entity();
		sw->Entity::Clone(newNode);

		SwitchComponent * swConponent = new SwitchComponent();
		newNode->AddComponent(swConponent);
		swConponent->SetSwitchIndex(sw->GetSwitchIndex());
		swConponent->Release();

		Entity * parent = sw->GetParent();
		DVASSERT(parent);
		if(parent)
		{
			parent->InsertBeforeNode(newNode, sw);
			parent->RemoveNode(sw);
		}

		newNode->Release();
		return true;
	}

	UserNode *un = dynamic_cast<UserNode*>(node);
	if(un)
	{
		Entity * newNode = new Entity();
		un->Clone(newNode);

		newNode->AddComponent(ScopedPtr<UserComponent> (new UserComponent()));

		Entity * parent = un->GetParent();
		DVASSERT(parent);
		if(parent)
		{
			parent->InsertBeforeNode(newNode, un);
			parent->RemoveNode(un);
		}

		newNode->Release();
		return true;
	}

	SpriteNode * spr = dynamic_cast<SpriteNode*>(node);
	if(spr)
	{
		Entity * newNode = new Entity();
		spr->Clone(newNode);

		SpriteObject *spriteObject = new SpriteObject(spr->GetSprite(), spr->GetFrame(), spr->GetScale(), spr->GetPivot());
		spriteObject->SetSpriteType((SpriteObject::eSpriteType)spr->GetType());

		newNode->AddComponent(ScopedPtr<RenderComponent> (new RenderComponent(spriteObject)));

		Entity * parent = spr->GetParent();
		DVASSERT(parent);
		if(parent)
		{
			parent->InsertBeforeNode(newNode, spr);
			parent->RemoveNode(spr);
		}

		spriteObject->Release();
		newNode->Release();
		return true;
	}


	return false;
} 
    


void SceneFileV2::ReplaceOldNodes(Entity * currentNode)
{
	for(int32 c = 0; c < currentNode->GetChildrenCount(); ++c)
	{
		Entity * childNode = currentNode->GetChild(c);
		ReplaceOldNodes(childNode);
		/**
			Here it's very important to call ReplaceNodeAfterLoad after recursion, to replace nodes that 
			was deep in hierarchy first.
			*/
		bool wasReplace = ReplaceNodeAfterLoad(childNode);
		if(wasReplace)
		{
			c--;
		}
	}
}

    
void SceneFileV2::OptimizeScene(Entity * rootNode)
{
    int32 beforeCount = rootNode->GetChildrenCountRecursive();
    removedNodeCount = 0;
    rootNode->BakeTransforms();
    
	//ConvertShadows(rootNode);
    //RemoveEmptySceneNodes(rootNode);
	ReplaceOldNodes(rootNode);
	RemoveEmptyHierarchy(rootNode);
	
    
//    for (int32 k = 0; k < rootNode->GetChildrenCount(); ++k)
//    {
//        Entity * node = rootNode->GetChild(k);
//        if (node->GetName() == "instance_0")
//            node->SetName(rootNodeName);
//    }
    int32 nowCount = rootNode->GetChildrenCountRecursive();
    Logger::FrameworkDebug("nodes removed: %d before: %d, now: %d, diff: %d", removedNodeCount, beforeCount, nowCount, beforeCount - nowCount);
}


};
