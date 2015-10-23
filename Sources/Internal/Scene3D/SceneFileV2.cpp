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
#include "Render/3D/AnimatedMesh.h"
#include "Scene3D/PathManip.h"
#include "Scene3D/SkeletonNode.h"
#include "Scene3D/BoneNode.h"
#include "Scene3D/SwitchNode.h"
#include "Render/Highlevel/Camera.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/3D/MeshUtils.h"
#include "Render/Material/NMaterialNames.h"

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
#include "Render/Highlevel/SpriteObject.h"
#include "Render/Highlevel/RenderObject.h"

#include "Render/Material/NMaterial.h"
#include "Scene3D/Components/CustomPropertiesComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"

#include "Scene3D/Scene.h"
#include "Scene3D/Systems/QualitySettingsSystem.h"

#include "Scene3D/Converters/LodToLod2Converter.h"
#include "Scene3D/Converters/SwitchToRenerObjectConverter.h"
#include "Scene3D/Converters/TreeToAnimatedTreeConverter.h"

#include "Job/JobManager.h"

#include <functional>


namespace DAVA
{

SceneFileV2::SceneFileV2()
{
    isDebugLogEnabled = false;
    isSaveForGame = false;
    lastError = ERROR_NO_ERROR;
	
	serializationContext.SetDebugLogEnabled(isDebugLogEnabled);
	serializationContext.SetLastError(lastError);

	UserNode *n = new UserNode();
	n->Release();
}

SceneFileV2::~SceneFileV2()
{
}
    
void SceneFileV2::EnableSaveForGame(bool _isSaveForGame)
{
    isSaveForGame = _isSaveForGame;
}

void SceneFileV2::EnableDebugLog(bool _isDebugLogEnabled)
{
    isDebugLogEnabled = _isDebugLogEnabled;
	serializationContext.SetDebugLogEnabled(isDebugLogEnabled);
}

bool SceneFileV2::DebugLogEnabled()
{
    return isDebugLogEnabled;
}
    
void SceneFileV2::SetError(eError error)
{
    lastError = error;
}

SceneFileV2::eError SceneFileV2::GetError()
{
    return lastError;
}

SceneFileV2::eError SceneFileV2::SaveScene(const FilePath & filename, DAVA::Scene *scene, SceneFileV2::eFileType fileType)
{
    File * file = File::Create(filename, File::CREATE | File::WRITE);
    if (!file)
    {
        Logger::Error("SceneFileV2::SaveScene failed to create file: %s", filename.GetAbsolutePathname().c_str());
        SetError(ERROR_FAILED_TO_CREATE_FILE);
        return GetError();
    }
    
    // save header
    header.signature[0] = 'S';
    header.signature[1] = 'F';
    header.signature[2] = 'V';
    header.signature[3] = '2';
    
    header.version = VersionInfo::Instance()->GetCurrentVersion().version;
    header.nodeCount = scene->GetChildrenCount();

    if(scene->GetGlobalMaterial())
    {
        header.nodeCount++;
    }
	
	descriptor.size = sizeof(descriptor.fileType); // + sizeof(descriptor.additionalField1) + sizeof(descriptor.additionalField1) +....
	descriptor.fileType = fileType;
	
	serializationContext.SetRootNodePath(filename);
    serializationContext.SetScenePath(FilePath(filename.GetDirectory()));
	serializationContext.SetVersion(header.version);
	serializationContext.SetScene(scene);
    
    file->Write(&header, sizeof(Header));
    
    // save version tags
    {
        KeyedArchive * tagsArchive = new KeyedArchive();
        const VersionInfo::TagsMap& tags = VersionInfo::Instance()->GetCurrentVersion().tags;
        for ( VersionInfo::TagsMap::const_iterator it = tags.begin(); it != tags.end(); it++ )
        {
            tagsArchive->SetUInt32( it->first, it->second );
        }
        tagsArchive->Save( file );
        SafeRelease( tagsArchive );
    }
    
	WriteDescriptor(file, descriptor);

    // save data objects
    if(isDebugLogEnabled)
    {
        Logger::FrameworkDebug("+ save data objects");
        Logger::FrameworkDebug("- save file path: %s", filename.GetDirectory().GetAbsolutePathname().c_str());
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

	if (isSaveForGame)
		scene->OptimizeBeforeExport();

    Set<DataNode*> nodes;
    scene->GetDataNodes(nodes);

    uint32 serializableNodesCount = 0;
    uint64 maxDataNodeID = 0;

    // compute maxid for datanodes
    for (auto node : nodes)
    {
        // TODO: now one datanode can be used in multiple scenes,
        // but datanote->scene points only on single scene. This should be
        // discussed and fixed in the future.
        if (node->GetScene() == scene && node->GetNodeID() > maxDataNodeID)
        {
            maxDataNodeID = node->GetNodeID();
        }
    }

    // assign datanode id-s and 
    // count serializable nodes
    for (auto node : nodes)
    {
        if (IsDataNodeSerializable(node))
        {
            // TODO: if datanode is from another scene, it should be saved with newly
            // generated datanode-id. Unfortunately this ID will be generated on every scene save,
            // because we don't change scene pointer in datanode->scene.
            // This should be discussed and fixed in the future.
            serializableNodesCount++;
            if (node->GetScene() != scene || node->GetNodeID() == DataNode::INVALID_ID)
            {
                node->SetNodeID(++maxDataNodeID);
            }
        }
    }

    // do we need to save globalmaterial?
    NMaterial *globalMaterial = scene->GetGlobalMaterial();
    if (nullptr != globalMaterial)
    {
		if (nodes.count(globalMaterial) > 0)
		{
			// remove global material from set, 
			// as it should be saved exclusively
			// on the top of data nodes
			nodes.erase(globalMaterial);
		}
		else 
		{
			serializableNodesCount++;
		}
    }

    // save datanodes count
    file->Write(&serializableNodesCount, sizeof(uint32));

    // save global material on top of datanodes
    if (nullptr != globalMaterial)
    {
        if (globalMaterial->GetNodeID() == DataNode::INVALID_ID)
        {
            globalMaterial->SetNodeID(++maxDataNodeID);
        }
        SaveDataNode(globalMaterial, file);
    }

    // sort in ascending ID order 
    Set<DataNode*, std::function<bool(DataNode *, DataNode *)>> orderedNodes(nodes.begin(), nodes.end(),
        [](DataNode* a, DataNode* b) { return a->GetNodeID() < b->GetNodeID(); });

    // save the rest of datanodes
    for (auto node : orderedNodes)
    {
        if (IsDataNodeSerializable(node))
        {
            SaveDataNode(node, file);
        }
    }
    

    // save global material settings
    if (nullptr != globalMaterial)
    {
        KeyedArchive * archive = new KeyedArchive();
        uint64 globalMaterialId = scene->GetGlobalMaterial()->GetNodeID();
    
        archive->SetString("##name", "GlobalMaterial");
        archive->SetUInt64("globalMaterialId", globalMaterialId);
        archive->Save(file);
    
        SafeRelease(archive);
    }

    // save hierarchy
    if(isDebugLogEnabled)
        Logger::FrameworkDebug("+ save hierarchy");

    for (int ci = 0; ci < scene->GetChildrenCount(); ++ci)
    {
        if (!SaveHierarchy(scene->GetChild(ci), file, 1))
        {
            Logger::Error("SceneFileV2::SaveScene failed to save hierarchy file: %s", filename.GetAbsolutePathname().c_str());
            SafeRelease(file);
            return GetError();
        }
    }
    
    SafeRelease(file);
    return GetError();
}

bool SceneFileV2::ReadHeader(SceneFileV2::Header& _header, File * file)
{
    DVASSERT(file);
    
    file->Read( &_header, sizeof( Header ) );

    if (   ( _header.signature[0] != 'S' )
        || ( _header.signature[1] != 'F' )
        || ( _header.signature[2] != 'V' )
        || ( _header.signature[3] != '2' ) )
    {
        Logger::Error( "SceneFileV2::LoadSceneVersion header is wrong" );
        return false;
    }

    return true;
}

bool SceneFileV2::ReadVersionTags(VersionInfo::SceneVersion& _version, File * file)
{
    DVASSERT(file);

    bool loaded = false;
    if ( _version.version >= 14 )
    {
        KeyedArchive * tagsArchive = new KeyedArchive();
        loaded = tagsArchive->Load( file );

        if (loaded)
        {
            using KeyedTagsMap = Map<String, VariantType*>;
            const KeyedTagsMap& keyedTags = tagsArchive->GetArchieveData();
            for (KeyedTagsMap::const_iterator it = keyedTags.begin(); it != keyedTags.end(); it++)
            {
                const String& tag = it->first;
                const uint32 ver = it->second->AsUInt32();
                _version.tags.insert(VersionInfo::TagsMap::value_type(tag, ver));
            }
        }
        SafeRelease(tagsArchive);
    }
    else
    {
        loaded = true;
    }

    return loaded;
}

VersionInfo::SceneVersion SceneFileV2::LoadSceneVersion(const FilePath & filename)
{
    File * file = File::Create( filename, File::OPEN | File::READ );
    if ( !file )
    {
        Logger::Error( "SceneFileV2::LoadSceneVersion failed to open file: %s", filename.GetAbsolutePathname().c_str() );
        return VersionInfo::SceneVersion();
    }

    VersionInfo::SceneVersion version;

    Header header;
    const bool headerValid = ReadHeader(header, file);
    if (headerValid)
    {
        version.version = header.version;
        const bool versionValid = ReadVersionTags(version, file);
        if (!versionValid)
        {
            version = VersionInfo::SceneVersion();
        }
    }

    SafeRelease(file);
    return version;
}
    
SceneFileV2::eError SceneFileV2::LoadScene(const FilePath & filename, Scene * scene)
{
    File * file = File::Create(filename, File::OPEN | File::READ);
    if (!file)
    {
        Logger::Error("SceneFileV2::LoadScene failed to open file: %s", filename.GetAbsolutePathname().c_str());
        SetError(ERROR_FAILED_TO_CREATE_FILE);
        return GetError();
    }   

    const bool headerValid = ReadHeader(header, file);

    if (!headerValid)
    {
        SafeRelease(file);
        Logger::Error("SceneFileV2::LoadScene: scene header is not valid");
        SetError(ERROR_VERSION_IS_TOO_OLD);
        return GetError();
    }

    if (header.version < SCENE_FILE_MINIMAL_SUPPORTED_VERSION)
    {
        SafeRelease(file);
        Logger::Error("SceneFileV2::LoadScene: scene version %d is too old. Minimal supported version is %d", header.version, SCENE_FILE_MINIMAL_SUPPORTED_VERSION);
        SetError(ERROR_VERSION_IS_TOO_OLD);
        return GetError();
    }

    // load version tags
    scene->version.version = header.version;
    const bool versionValid = ReadVersionTags(scene->version, file);
    if ( !versionValid )
    {
        Logger::Error("SceneFileV2::LoadScene version tags are wrong");

        SafeRelease(file);
        SetError(ERROR_VERSION_TAGS_INVALID);
        return GetError();
    }
	
	if(header.version >= 10)
	{
		ReadDescriptor(file, descriptor);
	}

    VersionInfo::eStatus status = VersionInfo::Instance()->TestVersion(scene->version);
    switch (status)
    {
    case VersionInfo::COMPATIBLE:
        {
            const String tags = VersionInfo::Instance()->UnsupportedTagsMessage(scene->version);
            Logger::Warning("SceneFileV2::LoadScene scene was saved with older version of framework. Saving scene will broke compatibility. Missed tags: %s", tags.c_str());
        }
        break;
    case VersionInfo::INVALID:
        {
            const String tags = VersionInfo::Instance()->NoncompatibleTagsMessage(scene->version);
            Logger::Error( "SceneFileV2::LoadScene scene is incompatible with current version. Wrong tags: %s", tags.c_str());
            SafeRelease( file );
            SetError( ERROR_VERSION_TAGS_INVALID );
            return GetError();
        }
    default:
        break;
    }

	serializationContext.SetRootNodePath(filename);
	serializationContext.SetScenePath(filename.GetDirectory());
	serializationContext.SetVersion(header.version);
	serializationContext.SetScene(scene);
	serializationContext.SetDefaultMaterialQuality(NMaterialQualityName::DEFAULT_QUALITY_NAME);
    
    if(isDebugLogEnabled)
        Logger::FrameworkDebug("+ load data objects");

    if (header.version >= 2)
    {
        int32 dataNodeCount = 0;
        file->Read(&dataNodeCount, sizeof(int32));
        
        for (int k = 0; k < dataNodeCount; ++k)
		{
            LoadDataNode(scene, nullptr, file);
		}
		
        // load global material
        {
            uint32 filePos = file->GetPos();
            KeyedArchive * archive = new KeyedArchive();
            archive->Load(file);

            String name = archive->GetString("##name");
            if (name == "GlobalMaterial")
            {
                uint64 globalMaterialId = archive->GetUInt64("globalMaterialId");
                NMaterial * globalMaterial = static_cast<NMaterial*>(serializationContext.GetDataBlock(globalMaterialId));                

                scene->SetGlobalMaterial(globalMaterial);
                serializationContext.SetGlobalMaterialKey(globalMaterialId);
                
                --header.nodeCount;
            }
            else
            {
                file->Seek(filePos, File::SEEK_FROM_START);
            }

            SafeRelease(archive);
        }

		serializationContext.ResolveMaterialBindings();
    }
    
    if(isDebugLogEnabled)
        Logger::FrameworkDebug("+ load hierarchy");

    scene->children.reserve(header.nodeCount);
    for (int ci = 0; ci < header.nodeCount; ++ci)
    {
        LoadHierarchy(0, scene, file, 1);
    }
		    
    //as we are going to take information about required attribute streams from shader - we are to wait for shader compilation
	JobManager::Instance()->WaitMainJobs();

	UpdatePolygonGroupRequestedFormatRecursively(scene);
    serializationContext.LoadPolygonGroupData(file);

    OptimizeScene(scene);

    if(GetError() == ERROR_NO_ERROR)
    {
        scene->SceneDidLoaded();
        scene->OnSceneReady(scene);
    }
    
    SafeRelease(file);
    return GetError();
}

SceneArchive *SceneFileV2::LoadSceneArchive(const FilePath & filename)
{
    SceneArchive *res = nullptr;
    File * file = File::Create(filename, File::OPEN | File::READ);
    if (!file)
    {
        Logger::Error("SceneFileV2::LoadScene failed to open file: %s", filename.GetAbsolutePathname().c_str());
        return res;
    }   

    const bool headerValid = ReadHeader(header, file);

    if (!headerValid)
    {
        Logger::Error("SceneFileV2::LoadScene: scene header is not valid");
        SafeRelease(file);
        return res;
    }

    if (header.version < SCENE_FILE_MINIMAL_SUPPORTED_VERSION)
    {
        Logger::Error("SceneFileV2::LoadScene: scene version %d is too old. Minimal supported version is %d", header.version, SCENE_FILE_MINIMAL_SUPPORTED_VERSION);
        SafeRelease(file);
        return res;
    }

    // load version tags
    VersionInfo::SceneVersion version;
    version.version = header.version;
    const bool versionValid = ReadVersionTags(version, file);
    if (!versionValid)
    {
        Logger::Error("SceneFileV2::LoadScene version tags are wrong");
        SafeRelease(file);
        return res;
    }
	
    if(header.version >= 10)
    {
        ReadDescriptor(file, descriptor);
    }

    VersionInfo::eStatus status = VersionInfo::Instance()->TestVersion(version);
    switch (status)
    {
    case VersionInfo::COMPATIBLE:
        {
            const String tags = VersionInfo::Instance()->UnsupportedTagsMessage(version);
            Logger::Warning("SceneFileV2::LoadScene scene was saved with older version of framework. Saving scene will broke compatibility. Missed tags: %s", tags.c_str());
        }
        break;
    case VersionInfo::INVALID:
        {
            const String tags = VersionInfo::Instance()->NoncompatibleTagsMessage(version);
            Logger::Error( "SceneFileV2::LoadScene scene is incompatible with current version. Wrong tags: %s", tags.c_str());
            SafeRelease(file);
            return res;
        }
    default:
        break;
    }

   res = new SceneArchive();

    if (header.version >= 2)
    {
        int32 dataNodeCount = 0;
        file->Read(&dataNodeCount, sizeof(int32));
        for (int k = 0; k < dataNodeCount; ++k)
        {
            KeyedArchive * archive = new KeyedArchive();
            archive->Load(file);
            res->dataNodes.push_back(archive);
        }
    }

    res->children.reserve(header.nodeCount);
    for (int ci = 0; ci < header.nodeCount; ++ci)
    {                
        SceneArchive::SceneArchiveHierarchyNode * child = new SceneArchive::SceneArchiveHierarchyNode();
        child->LoadHierarchy(file);
        res->children.push_back(child);
        
    }    
    SafeRelease(file);
    return res;
}

void SceneFileV2::WriteDescriptor(File* file, const Descriptor& descriptor) const
{
	file->Write(&descriptor.size, sizeof(descriptor.size));
	file->Write(&descriptor.fileType, sizeof(descriptor.fileType));
}
	
void SceneFileV2::ReadDescriptor(File* file, /*out*/ Descriptor& descriptor)
{
	file->Read(&descriptor.size, sizeof(descriptor.size));
	DVASSERT(descriptor.size >= sizeof(descriptor.fileType));
	
	file->Read(&descriptor.fileType, sizeof(descriptor.fileType));
	
	if(descriptor.size > sizeof(descriptor.fileType))
	{
		//skip extra data probably added by future versions
		file->Seek(descriptor.size - sizeof(descriptor.fileType), File::SEEK_FROM_CURRENT);
	}
}


bool SceneFileV2::SaveDataNode(DataNode * node, File * file)
{
    KeyedArchive * archive = new KeyedArchive();
    
    node->Save(archive, &serializationContext);
    archive->Save(file);

    SafeRelease(archive);
    return true;
}
    
void SceneFileV2::LoadDataNode(Scene *scene, DataNode * parent, File * file)
{
    uint32 currFilePos = file->GetPos();
    KeyedArchive * archive = new KeyedArchive();
    archive->Load(file);
    
    String name = archive->GetString("##name");
    DataNode * node = dynamic_cast<DataNode *>(ObjectFactory::Instance()->New<BaseObject>(name));
    
    if (node)
    {
        if (node->GetClassName() == "DataNode")
        {
            SafeRelease(node);
            SafeRelease(archive);
            return;
        }   
        node->SetScene(scene);
        
        if (isDebugLogEnabled)
        {
            String arcName = archive->GetString("name");
            Logger::FrameworkDebug("- %s(%s)", arcName.c_str(), node->GetClassName().c_str());
        }
        node->Load(archive, &serializationContext);
        AddToNodeMap(node);

        if (name == "PolygonGroup")
        {
            serializationContext.AddLoadedPolygonGroup(static_cast<PolygonGroup*>(node), currFilePos);
        }
        
        int32 childrenCount = archive->GetInt32("#childrenCount", 0);
        DVASSERT(0 == childrenCount && "We don't support hierarchical dataNodes load.");
        
        SafeRelease(node);
    }
    SafeRelease(archive);
}

bool SceneFileV2::SaveDataHierarchy(DataNode * node, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    node->Save(archive, &serializationContext);
    
    SafeRelease(archive);
    return true;
}

void SceneFileV2::LoadDataHierarchy(Scene * scene, DataNode * root, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    archive->Load(file);
    
    // DataNode * node = dynamic_cast<DataNode*>(BaseObject::LoadFromArchive(archive));
    
    String name = archive->GetString("##name");
    DataNode * node = dynamic_cast<DataNode *>(ObjectFactory::Instance()->New<BaseObject>(name));

    if (node)
    {
        if (node->GetClassName() == "DataNode")
        {
            SafeRelease(node);
            node = SafeRetain(root); // retain root here because we release it at the end
        }  
        
        node->SetScene(scene);
        
        if (isDebugLogEnabled)
        {
            String arcName = archive->GetString("name");
            Logger::FrameworkDebug("%s %s(%s)", GetIndentString('-', level).c_str(), arcName.c_str(), node->GetClassName().c_str());
        }

        node->Load(archive, &serializationContext);
        AddToNodeMap(node);
        
        int32 childrenCount = archive->GetInt32("#childrenCount", 0);
        DVASSERT(0 == childrenCount && "We don't support hierarchical dataNodes load.");
        
        SafeRelease(node);
    }
    
    SafeRelease(archive);
}
    
void SceneFileV2::AddToNodeMap(DataNode * node)
{
    uint64 id = node->GetNodeID();
	serializationContext.SetDataBlock(id, SafeRetain(node));
}
    
bool SceneFileV2::SaveHierarchy(Entity * node, File * file, int32 level)
{
    KeyedArchive * archive = new KeyedArchive();
    if (isDebugLogEnabled)
        Logger::FrameworkDebug("%s %s(%s) %d", GetIndentString('-', level).c_str(), node->GetName().c_str(), node->GetClassName().c_str(), node->GetChildrenCount());
    node->Save(archive, &serializationContext);
    
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
    bool keepUnusedQualityEntities = QualitySettingsSystem::Instance()->GetKeepUnusedEntities();
    KeyedArchive * archive = new KeyedArchive();
    archive->Load(file);

    String name = archive->GetString("##name");
    
    bool removeChildren = false;
    bool skipNode = false;
    
    Entity * node = nullptr;
    if (name == "LandscapeNode")
    {
        node = LoadLandscape(scene, archive);
    }
    else if (name == "Camera")
    {
        node = LoadCamera(scene, archive);
    }
    else if ((name == "LightNode"))// || (name == "EditorLightNode"))
    {
        node = LoadLight(scene, archive);
        removeChildren = true;
    }
	else if(name == "SceneNode")
	{
		node = LoadEntity(scene, archive);
	}
	else
    {
        BaseObject *obj = ObjectFactory::Instance()->New<BaseObject>(name);
        node = dynamic_cast<Entity*>(obj);
        if(node)
        {
            node->SetScene(scene);
            node->Load(archive, &serializationContext);
            
        }
        else //in case if editor class is loading in non-editor sprsoject
        {
            SafeRelease(obj);
            node = new Entity();
            skipNode = true;
        }
    }

    if(nullptr != node)
    {
        if(isDebugLogEnabled)
        {
            String arcName = archive->GetString("name");
            Logger::FrameworkDebug("%s %s(%s)", GetIndentString('-', level).c_str(), arcName.c_str(), node->GetClassName().c_str());
        }

        if (!skipNode && (keepUnusedQualityEntities||QualitySettingsSystem::Instance()->IsQualityVisible(node)))
        {
            parent->AddNode(node);
        }

        int32 childrenCount = archive->GetInt32("#childrenCount", 0);
        node->children.reserve(childrenCount);
        for(int ci = 0; ci < childrenCount; ++ci)
        {
            LoadHierarchy(scene, node, file, level + 1);
        }

        if(removeChildren && childrenCount)
        {
            node->RemoveAllChildren();
        }

        ParticleEffectComponent *effect = static_cast<ParticleEffectComponent*>(node->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
        if(effect && (effect->loadedVersion == 0))
            effect->CollapseOldEffect(&serializationContext);

        SafeRelease(node);
    }

    SafeRelease(archive);
}
    
Entity * SceneFileV2::LoadEntity(Scene * scene, KeyedArchive * archive)
{
    Entity * entity = new Entity();
    entity->SetScene(scene);
    entity->Load(archive, &serializationContext);
    return entity;
}

Entity * SceneFileV2::LoadLandscape(Scene * scene, KeyedArchive * archive)
{
    Entity * landscapeEntity = LoadEntity(scene, archive);
    
    Landscape * landscapeRenderObject = new Landscape();
    landscapeRenderObject->Load(archive, &serializationContext);
    
    landscapeEntity->AddComponent(new RenderComponent(landscapeRenderObject));
    SafeRelease(landscapeRenderObject);
    
    return landscapeEntity;
}


Entity * SceneFileV2::LoadCamera(Scene * scene, KeyedArchive * archive)
{
    Entity * cameraEntity = LoadEntity(scene, archive);
    
    Camera * cameraObject = new Camera();
    cameraObject->LoadObject(archive);
    
    cameraEntity->AddComponent(new CameraComponent(cameraObject));
    SafeRelease(cameraObject);
    
    return cameraEntity;
}

Entity * SceneFileV2::LoadLight(Scene * scene, KeyedArchive * archive)
{
    Entity * lightEntity = LoadEntity(scene, archive);
    
    bool isDynamic = true;
    KeyedArchive *props = GetCustomPropertiesArchieve(lightEntity);
    if(props)
    {
        isDynamic = props->GetBool("editor.dynamiclight.enable", true);
    }
    
    Light * light = new Light();
    light->Load(archive, &serializationContext);
    light->SetDynamic(isDynamic);
    
    lightEntity->AddComponent(new LightComponent(light));
    SafeRelease(light);
    
    return lightEntity;
}


void SceneFileV2::ConvertShadows(Entity * currentNode)
{
	for(int32 c = 0; c < currentNode->GetChildrenCount(); ++c)
	{
		Entity * childNode = currentNode->GetChild(c);
		if(String::npos != childNode->GetName().find("_shadow"))
		{
			DVASSERT(childNode->GetChildrenCount() == 1);
			Entity * svn = childNode->FindByName(FastName("dynamicshadow.shadowvolume"));
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
        KeyedArchive *customProperties = GetCustomPropertiesArchieve(currentNode);
        bool doNotRemove = customProperties && customProperties->IsKeyExists("editor.donotremove");
        
        uint32 componentCount = currentNode->GetComponentCount();

        Component * tr = currentNode->GetComponent(Component::TRANSFORM_COMPONENT);
        Component * cp = currentNode->GetComponent(Component::CUSTOM_PROPERTIES_COMPONENT);
        if (((componentCount == 2) && (!cp || !tr)) ||
            (componentCount > 2))
        {
            doNotRemove = true;
        }
        
        if(currentNode->GetName().find("dummy") != String::npos)
        {
            doNotRemove = true;
        }
        
        if (!doNotRemove)
        {
            Entity * parent  = currentNode->GetParent();
            if (parent)
            {
                if(header.version < OLD_LODS_SCENE_VERSION && GetLodComponent(parent))
				{
					return false;
				}

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
		if(nullptr != currentNode->GetComponent(Component::TRANSFORM_COMPONENT))
		{
			allowed_comp_count++;
		}

		if(nullptr != currentNode->GetComponent(Component::CUSTOM_PROPERTIES_COMPONENT))
		{
			allowed_comp_count++;
		}

		if (currentNode->GetComponentCount() > allowed_comp_count)
		{
            return false;
		}
        
        
        if (currentNode->GetLocalTransform() == Matrix4::IDENTITY)
        {
            Entity * parent  = currentNode->GetParent();
            uint32 curNodeID = currentNode->GetID();

            if (parent)
            {
				if(header.version < OLD_LODS_SCENE_VERSION && GetLodComponent(parent))
				{
					return false;
				}

                Entity * childNode = SafeRetain(currentNode->GetChild(0));

				FastName currentName = currentNode->GetName();
				KeyedArchive * currentProperties = GetCustomPropertiesArchieve(currentNode);
                
                //Logger::FrameworkDebug("remove node: %s %p", currentNode->GetName().c_str(), currentNode);
				parent->InsertBeforeNode(childNode, currentNode);
                
                //MEGA kostyl
                if(!childNode->GetComponent(Component::PARTICLE_EFFECT_COMPONENT))//do not rename effects
                {
                    childNode->SetName(currentName);
                }
				//merge custom properties
                
                if(currentProperties)
                {
                    KeyedArchive * newProperties = GetOrCreateCustomProperties(childNode)->GetArchive();
                    const Map<String, VariantType*> & oldMap = currentProperties->GetArchieveData();
                    Map<String, VariantType*>::const_iterator itEnd = oldMap.end();
                    for(Map<String, VariantType*>::const_iterator it = oldMap.begin(); it != itEnd; ++it)
                    {
                        newProperties->SetVariant(it->first, *it->second);
                    }
                }
				
				//VI: remove node after copying its properties since properties become invalid after node removal
				parent->RemoveNode(currentNode);

                removedNodeCount++;
                childNode->SetID(curNodeID);
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
        Entity * newMeshInstanceNode = new Entity();
        oldMeshInstanceNode->Entity::Clone(newMeshInstanceNode);

		Component *clonedComponent = oldMeshInstanceNode->GetComponent(Component::TRANSFORM_COMPONENT)->Clone(newMeshInstanceNode);
		newMeshInstanceNode->RemoveComponent(Component::TRANSFORM_COMPONENT);
        newMeshInstanceNode->AddComponent(clonedComponent);
        
        Mesh * mesh = new Mesh();

        Vector<PolygonGroupWithMaterial*> polygroups = oldMeshInstanceNode->GetPolygonGroups();
        for (uint32 k = 0; k < (uint32)polygroups.size(); ++k)
        {
            PolygonGroupWithMaterial * group = polygroups[k];
            NMaterial * material = group->GetMaterial();
            if (material)
            {
                NMaterial * batchMaterial = new NMaterial();
                batchMaterial->SetParent(material);
                batchMaterial->SetMaterialName(FastName(Format("Instance-%u",
                                                               static_cast<uint32>(material->GetChildren().size()))));
                mesh->AddPolygonGroup(group->GetPolygonGroup(), batchMaterial);
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
                
                mesh->SetOwnerDebugInfo(FastName(Format("%s shadow:%s", oldMeshInstanceNode->GetName().c_str(), oldShadowVolumeNode->GetName().c_str()).c_str()));
                
                parent->RemoveNode(oldShadowVolumeNode);
                SafeRelease(newShadowVolume);
            }
        }
        
        RenderComponent * renderComponent = new RenderComponent;
        renderComponent->SetRenderObject(mesh);
        newMeshInstanceNode->AddComponent(renderComponent);
        
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

		LodComponent *lc = new LodComponent();
		newNode->AddComponent(lc);

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
			
//			newLodDataItem.nodes = oldDataItem->nodes;
			for(uint32 n = 0; n < oldDataItem->nodes.size(); ++n)
			{
				Entity *nn = oldDataItem->nodes[n];

				int32 childrenCount = lod->GetChildrenCount();
				for(int32 c = 0; c < childrenCount; ++c)
				{
					if(nn == lod->GetChild(c))
					{
						newLodDataItem.nodes.push_back(newNode->GetChild(c));
						break;
					}
				}
			}


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

		newNode->AddComponent(new ParticleEffectComponent());
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

		newNode->AddComponent(new UserComponent());

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

void SceneFileV2::RemoveDeprecatedMaterialFlags(Entity * node)
{      
    RenderObject * ro = GetRenderObject(node);
    if (ro)
    {
        static const FastName FLAG_TILED_DECAL = FastName("TILED_DECAL");
        static const FastName FLAG_FOG_EXP = FastName("FOG_EXP");

        uint32 batchCount = ro->GetRenderBatchCount();
        for (uint32 ri = 0; ri < batchCount; ++ri)
        {
            int32 flagValue = 0;
            RenderBatch * batch = ro->GetRenderBatch(ri);
            NMaterial * material = batch->GetMaterial();

            while (material)
            {
                /*if (material->HasLocalFlag(FLAG_FOG_EXP))
                    material->RemoveFlag(FLAG_FOG_EXP);                */

                if (material->HasLocalFlag(FLAG_TILED_DECAL))
                {
                    material->AddFlag(NMaterialFlagName::FLAG_TILED_DECAL_MASK, material->GetLocalFlagValue(FLAG_TILED_DECAL));
                    material->RemoveFlag(FLAG_TILED_DECAL);
                }

                material = material->GetParent();
            }
        }
    }

    uint32 size = node->GetChildrenCount();
    for (uint32 i = 0; i < size; ++i)
    {
        Entity * child = node->GetChild(i);
        RemoveDeprecatedMaterialFlags(child);
    }
}

void SceneFileV2::ConvertAlphatestValueMaterials(Entity * node)
{
    static const float32 alphatestThresholdValue = .3f;
    static const Array<FastName, 7> alphatestValueMaterials =
    {
        FastName("~res:/Materials/NormalizedBlinnPhongPerPixel.Alphatest.material"),
        FastName("~res:/Materials/NormalizedBlinnPhongPerPixel.Alphatest.Alphablend.material"),
        FastName("~res:/Materials/NormalizedBlinnPhongPerPixelFast.Alphatest.material"),
        FastName("~res:/Materials/NormalizedBlinnPhongPerVertex.Alphatest.material"),
        FastName("~res:/Materials/NormalizedBlinnPhongPerVertex.Alphatest.Alphablend.material"),
        FastName("~res:/Materials/NormalizedBlinnPhongAllQualities.Alphatest.material"),
        FastName("~res:/Materials/NormalizedBlinnPhongAllQualities.Alphatest.Alphablend.material"),
    };

    RenderObject * ro = GetRenderObject(node);
    if (ro)
    {
        uint32 batchCount = ro->GetRenderBatchCount();
        for (uint32 ri = 0; ri < batchCount; ++ri)
        {
            int32 flagValue = 0;
            RenderBatch * batch = ro->GetRenderBatch(ri);
            NMaterial * material = batch->GetMaterial();

            while (material)
            {
                for (auto & alphatestTemplate : alphatestValueMaterials)
                {
                    if (alphatestTemplate == material->GetLocalFXName())
                    {
                        if (!material->HasLocalProperty(NMaterialParamName::PARAM_ALPHATEST_THRESHOLD))
                            material->AddProperty(NMaterialParamName::PARAM_ALPHATEST_THRESHOLD, &alphatestThresholdValue, rhi::ShaderProp::TYPE_FLOAT1);
                    }
                }
                material = material->GetParent();
            }
        }
    }

    uint32 size = node->GetChildrenCount();
    for (uint32 i = 0; i < size; ++i)
    {
        Entity * child = node->GetChild(i);
        ConvertAlphatestValueMaterials(child);
    }
}

void SceneFileV2::RebuildTangentSpace(Entity *entity)
{
    static int32 prerequiredFormat = EVF_TANGENT|EVF_NORMAL;
    RenderObject *ro = GetRenderObject(entity);

    if (ro)
    {
        for (int32 i=0, sz=ro->GetRenderBatchCount(); i<sz; ++i)
        {
            RenderBatch *renderBatch = ro->GetRenderBatch(i);
            PolygonGroup *group = renderBatch->GetPolygonGroup();
            if (group)
            {
                int32 format = group->GetFormat();
                if (((format&prerequiredFormat)==prerequiredFormat)&&!(format&EVF_BINORMAL))
                    MeshUtils::RebuildMeshTangentSpace(group, true);
            }
        }
    }

    for (int32 i=0, sz = entity->GetChildrenCount(); i<sz; ++i)
        RebuildTangentSpace(entity->GetChild(i));
}

void SceneFileV2::ConvertShadowVolumes(Entity * entity, NMaterial * shadowMaterialParent)
{

    RenderObject * ro = GetRenderObject(entity);
    if(ro)
    {
        int32 batchCount = ro->GetRenderBatchCount();
        for(int32 ri = 0; ri < batchCount; ++ri)
        {
            RenderBatch * batch = ro->GetRenderBatch(ri);
            if(typeid(*batch) == typeid(ShadowVolume) || entity->GetName().find("_shadow") != String::npos)
            {
                RenderBatch * shadowBatch = new RenderBatch();
                if(typeid(*batch) == typeid(ShadowVolume))
                {
                    shadowBatch->SetPolygonGroup(batch->GetPolygonGroup());
                }
                else
                {
                    PolygonGroup * shadowPg = MeshUtils::CreateShadowPolygonGroup(batch->GetPolygonGroup());
                    shadowBatch->SetPolygonGroup(shadowPg);
                    shadowPg->Release();
                }

                NMaterial* shadowMaterial = new NMaterial();
                shadowMaterial->SetParent(shadowMaterialParent);

                shadowMaterial->SetMaterialName(FastName(Format("%s-%u",
                                                                shadowMaterialParent->GetMaterialName().c_str(),
                                                                static_cast<DAVA::uint32>(shadowMaterialParent->GetChildren().size()))));

                shadowBatch->SetMaterial(shadowMaterial);
                shadowMaterial->Release();

                ro->ReplaceRenderBatch(ri, shadowBatch);
                shadowBatch->Release();
            }
        }
    }

    uint32 size = entity->GetChildrenCount();
    for(uint32 i = 0; i < size; ++i)
    {
        Entity * child = entity->GetChild(i);
        ConvertShadowVolumes(child, shadowMaterialParent);
    }

}

void SceneFileV2::OptimizeScene(Entity * rootNode)
{
    int32 beforeCount = rootNode->GetChildrenCountRecursive();
    removedNodeCount = 0;
    rootNode->BakeTransforms();
    
	//ConvertShadows(rootNode);
    RemoveEmptySceneNodes(rootNode);
	ReplaceOldNodes(rootNode);
	RemoveEmptyHierarchy(rootNode);

    if (header.version < SHADOW_VOLUME_SCENE_VERSION)
    {
        NMaterial * shadowMaterial = new NMaterial();
        shadowMaterial->SetMaterialName(FastName("Shadow_Material"));
        shadowMaterial->SetFXName(NMaterialName::SHADOW_VOLUME);
        ConvertShadowVolumes(rootNode, shadowMaterial);
        shadowMaterial->Release();
    }

    if(header.version < OLD_LODS_SCENE_VERSION)
    {
	    LodToLod2Converter lodConverter;
	    lodConverter.ConvertLodToV2(rootNode);
	    SwitchToRenerObjectConverter switchConverter;
	    switchConverter.ConsumeSwitchedRenderObjects(rootNode);
    }    
	
    if(header.version < TREE_ANIMATION_SCENE_VERSION)
    {
        TreeToAnimatedTreeConverter treeConverter;
        treeConverter.ConvertTrees(rootNode);
    }

    if (header.version < PREREQUIRED_BINORMAL_SCENE_VERSION)
    {     
        RebuildTangentSpace(rootNode);
    }

    if (header.version < DEPRECATED_MATERIAL_FLAGS_SCENE_VERSION)
    {
        RemoveDeprecatedMaterialFlags(rootNode);
    }

    if (header.version < ALPHATEST_VALUE_FLAG_SCENE_VERSION)
    {
        ConvertAlphatestValueMaterials(rootNode);
    }

    QualitySettingsSystem::Instance()->UpdateEntityAfterLoad(rootNode);
    
//    for (int32 k = 0; k < rootNode->GetChildrenCount(); ++k)
//    {
//        Entity * node = rootNode->GetChild(k);
//        if (node->GetName() == "instance_0")
//            node->SetName(rootNodeName);
//    }
    int32 nowCount = rootNode->GetChildrenCountRecursive();
    Logger::FrameworkDebug("nodes removed: %d before: %d, now: %d, diff: %d", removedNodeCount, beforeCount, nowCount, beforeCount - nowCount);
}

void SceneFileV2::UpdatePolygonGroupRequestedFormatRecursively(Entity *entity)
{

    RenderObject *ro = GetRenderObject(entity);

    if (ro)
    {
        for (int32 i=0, sz=ro->GetRenderBatchCount(); i<sz; ++i)
        {
            RenderBatch *renderBatch = ro->GetRenderBatch(i);
            PolygonGroup *group = renderBatch->GetPolygonGroup();
            NMaterial *material = renderBatch->GetMaterial();
            if (group && material)
                serializationContext.AddRequestedPolygonGroupFormat(group, material->GetRequiredVertexFormat());            
        }
    }

    for (int32 i=0, sz = entity->GetChildrenCount(); i<sz; ++i)
        UpdatePolygonGroupRequestedFormatRecursively(entity->GetChild(i));
}

SceneArchive::~SceneArchive()
{
    for(auto& node : dataNodes)
    {
        SafeRelease(node);
    }
    for(auto& child : children)
    {
        SafeRelease(child);
    }
}

SceneArchive::SceneArchiveHierarchyNode::SceneArchiveHierarchyNode():archive(nullptr)
{
}

void SceneArchive::SceneArchiveHierarchyNode::LoadHierarchy(File *file)
{
    archive = new KeyedArchive();
    archive->Load(file);
    int32 childrenCount = archive->GetInt32("#childrenCount", 0);
    children.reserve(childrenCount);
    for (int ci = 0; ci < childrenCount; ++ci)
    {
        SceneArchiveHierarchyNode * child = new SceneArchiveHierarchyNode();
        child->LoadHierarchy(file);
        children.push_back(child);
    }
}

SceneArchive::SceneArchiveHierarchyNode::~SceneArchiveHierarchyNode()
{
    SafeRelease(archive);
    for(auto& child : children)
    {
        SafeRelease(child);
    }
}

};
