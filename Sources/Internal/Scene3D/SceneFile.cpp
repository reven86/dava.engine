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


#include "Scene3D/SceneFile.h"
#include "Scene3D/Entity.h"
#include "Scene3D/MeshInstanceNode.h"
#include "Render/Texture.h"
#include "Render/3D/AnimatedMesh.h"
#include "Scene3D/PathManip.h"
#include "Scene3D/SkeletonNode.h"
#include "Scene3D/BoneNode.h"
#include "Render/Highlevel/Camera.h"
#include "Scene3D/LodNode.h"
#include "Scene3D/SceneNodeAnimationList.h"
#include "Scene3D/DataNode.h"
#include "Utils/StringFormat.h"
#include "FileSystem/FileSystem.h"
#include "Render/TextureDescriptor.h"
#include "Render/PixelFormatDescriptor.h"
#include "Render/3D/MeshUtils.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/AnimationData.h"

namespace DAVA
{

void ReadString(FILE * fp, char * res, int maxSize);  // XCode 4 fix
void ReadString(FILE * fp, char * res, int maxSize)
{
	int pos = 0;
	while(pos < maxSize)
	{
		char c;
		fread(&c, 1, 1, fp);
		res[pos++] = c;
		if (c == 0)break;
	}
}
	
SceneFile::Header::Header()
{
	descriptor[0] = 'D'; descriptor[1] = 'V';
	descriptor[2] = 'S'; descriptor[3] = 'C';
	
	version = 106;
	materialCount = 0;
	staticMeshCount = 0;

	lightCount = 0;
	cameraCount = 0;
}
	
SceneFile::SceneFile()
{
	sceneFP = 0;
	scene = 0;
    rootNode = nullptr;
	currentSkeletonNode = 0;
	debugLogEnabled = false;
}
	
void SceneFile::SetDebugLog(bool _debugLogEnabled)
{
	debugLogEnabled = _debugLogEnabled;
}

bool SceneFile::LoadScene(const FilePath & filename, Scene * _scene, bool relToBundle /*= true*/)
{
    materials.clear();
    
	scene = _scene;
    rootNodePath = filename;
    
    
//  textureIndexOffset = scene->GetTextureCount();
//	staticMeshIndexOffset = scene->GetStaticMeshCount();
	animatedMeshIndexOffset = scene->GetAnimatedMeshCount();
	cameraIndexOffset = scene->GetCameraCount();
    

	sceneFP = File::Create(filename, File::OPEN | File::READ);
	if(!sceneFP)
	{
		Logger::Warning("SceneFile::LoadScene failed to open file %s", filename.GetAbsolutePathname().c_str());
		return false;
	}

	// get scene path, store it to add to texture paths
    //TODO: what this?
	scenePath = PathManip(filename.GetAbsolutePathname().c_str()).GetPath();

	Logger::FrameworkDebug("scene start load: path = %s\n", scenePath.c_str());
	
	sceneFP->Read(&header, sizeof(SceneFile::Header));
	
    if (header.version < 106)
    {
        SafeRelease(sceneFP);
        return false;
    }
    DVASSERT(header.version >= 106 && "Do not support old converted formats after refactoring. Please reconvert everything");
    

    
	if (debugLogEnabled)Logger::FrameworkDebug("File version: %d\n", header.version);
	if (debugLogEnabled)Logger::FrameworkDebug("Material count: %d\n", header.materialCount);
	if (debugLogEnabled)Logger::FrameworkDebug("Static Mesh count: %d\n", header.staticMeshCount);
	if (debugLogEnabled)Logger::FrameworkDebug("Animated Mesh count: %d\n", header.animatedMeshCount);
	if (debugLogEnabled)Logger::FrameworkDebug("Cameras count: %d\n", header.cameraCount);
	if (debugLogEnabled)Logger::FrameworkDebug("Node animations count: %d\n", header.nodeAnimationsCount);
	if (debugLogEnabled)Logger::FrameworkDebug("Lights count: %d\n", header.lightCount);

//  Do not read textures anymore
//	for (uint32 textureIndex = 0; textureIndex < header.textureCount; ++textureIndex)
//	{
//		ReadTexture();
//		if (sceneFP->IsEof())return false;
//	}

	for (uint32 materialIndex = 0; materialIndex < header.materialCount; ++materialIndex)
	{
		ReadMaterial();
		if (sceneFP->IsEof())return false;
	}
	
	for (uint32 staticMeshIndex = 0; staticMeshIndex < header.staticMeshCount; ++staticMeshIndex)
	{
		ReadStaticMesh();
		if (sceneFP->IsEof())return false;
	}
	
	for (uint32 animatedMeshIndex = 0; animatedMeshIndex < header.animatedMeshCount; ++animatedMeshIndex)
	{
		ReadAnimatedMesh();
		if (sceneFP->IsEof())return false;
	}
	
	for (uint32 lightIndex = 0; lightIndex < header.lightCount; ++lightIndex)
	{
		ReadLight();
		if (sceneFP->IsEof())return false;
	}
	
	for (uint32 camIndex = 0; camIndex < header.cameraCount; ++camIndex)
	{
		ReadCamera();
		if (sceneFP->IsEof())return false;
	}
	
	for (int32 animationIndex = 0; animationIndex < (int32)header.nodeAnimationsCount; ++animationIndex)
	{
		ReadAnimation();
		if (sceneFP->IsEof())return false;
	}
	
	ReadSceneGraph();
	
	for (int32 animatedMeshIndex = 0; animatedMeshIndex < (int32)header.animatedMeshCount; ++animatedMeshIndex)
	{
		AnimatedMesh * aMesh = scene->GetAnimatedMesh(animatedMeshIndex + animatedMeshIndexOffset);
		aMesh->RestoreBonesFromNames();
	}
	// Binding of animations to scene nodes 
	for (int32 animationIndex = 0; animationIndex < (int32)header.nodeAnimationsCount; ++animationIndex)
	{
		SceneNodeAnimationList * aList = animations[animationIndex];
		for (int32 k = 0; k < (int32)aList->animations.size(); ++k)
		{
			SceneNodeAnimation * anim = aList->animations[k];
			if (!anim)
			{
				if (debugLogEnabled)Logger::Error("*** ERROR: animation: %d can't find anim: %s\n", animationIndex, aList->GetName().c_str());
				continue;
			}
			FastName & name = anim->bindName;
			Entity * bindNode = rootNode->FindByName(name);
			anim->SetBindNode(bindNode);
			if (!bindNode)
			{
				if (debugLogEnabled)Logger::Error("*** ERROR: animation: %d can't find bind node: %s\n", animationIndex, name.c_str());
			}
			else
			{
				if (bindNode->GetParent() && dynamic_cast< LodNode* >(bindNode->GetParent()))
					bindNode = bindNode->GetParent();

				if (!bindNode->GetComponent(Component::ANIMATION_COMPONENT))
				{
					AnimationComponent* animComp = new AnimationComponent();

					AnimationData* animData = new AnimationData();
					for (int32 keyIndex = 0; keyIndex < anim->GetKeyCount(); ++keyIndex)
					{
						animData->AddKey(anim->keys[keyIndex]);
					}
					animData->SetInvPose(anim->GetInvPose());
					animData->SetDuration(anim->GetDuration());

					animComp->SetAnimation(animData);
					bindNode->AddComponent(animComp);
				}
			}
		}
	}
	SafeRelease(sceneFP);
	
	Logger::FrameworkDebug("scene loaded");
	
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

    for (size_t animationIndex = 0; animationIndex < animations.size(); ++animationIndex)
    {
        SafeRelease(animations[animationIndex]);
    }
    animations.clear();

    return true;
}

bool SceneFile::SaveScene(const FilePath & filename)
{
	return true;
};
	
bool SceneFile::ReadTexture()
{
	SceneFile::TextureDef textureDef;
	sceneFP->ReadString(textureDef.name, 512);
	
	// Texture * texture = Texture::CreateFromPNG(Format("XResources//%s//", textureDef.name));
	String tname = textureDef.name;
	if (tname.c_str()[0] == '/')
	{
		tname = tname.substr(1, tname.length()-1);
	}

	tname = scenePath + tname;
    
    uint8 hasOpacity = false;
    sceneFP->Read(&hasOpacity, sizeof(hasOpacity));
	
	DAVA::Texture * texture = DAVA::Texture::CreateFromFile(tname);//textureDef.name);//0;
	if (debugLogEnabled)
		Logger::FrameworkDebug("- Texture: %s hasOpacity: %s %s\n", textureDef.name, (hasOpacity) ? ("yes") : ("no"), PixelFormatDescriptor::GetPixelFormatString(texture->texDescriptor->format));
    
    SafeRelease(texture);
	return true;
}
	
bool SceneFile::ReadMaterial()
{
	SceneFile::MaterialDef materialDef; 
	sceneFP->ReadString(materialDef.name, 512);

	sceneFP->Read(&materialDef.ambient, sizeof(materialDef.ambient));
	sceneFP->Read(&materialDef.diffuse, sizeof(materialDef.diffuse));
	sceneFP->Read(&materialDef.emission, sizeof(materialDef.emission));
	sceneFP->Read(&materialDef.indexOfRefraction, sizeof(materialDef.indexOfRefraction));
	sceneFP->Read(&materialDef.reflective, sizeof(materialDef.reflective));
	sceneFP->Read(&materialDef.reflectivity, sizeof(materialDef.reflectivity));
	sceneFP->Read(&materialDef.shininess, sizeof(materialDef.shininess));
	sceneFP->Read(&materialDef.specular, sizeof(materialDef.specular));
	sceneFP->Read(&materialDef.transparency, sizeof(materialDef.transparency));
	sceneFP->Read(&materialDef.transparent, sizeof(materialDef.transparent));

    sceneFP->ReadString(materialDef.diffuseTexture, 512);
    sceneFP->ReadString(materialDef.lightmapTexture, 512);
    sceneFP->ReadString(materialDef.reflectiveTexture, 512);
    sceneFP->ReadString(materialDef.specularTexture, 512);
    sceneFP->ReadString(materialDef.normalMapTexture, 512);

    materialDef.hasOpacity = false;
    sceneFP->Read(&materialDef.hasOpacity, sizeof(materialDef.hasOpacity));

    NMaterial* mat = new NMaterial();
    mat->SetMaterialName(FastName(materialDef.name));
    mat->SetFXName(NMaterialName::TEXTURED_OPAQUE);

    if (strlen(materialDef.diffuseTexture))
    {
        mat->AddTexture(NMaterialTextureName::TEXTURE_ALBEDO, ScopedPtr<Texture>(Texture::CreateFromFile(scenePath + String(materialDef.diffuseTexture))));
    }
	
    // retain object when we put it to array
    materials.push_back(SafeRetain(mat));
    
	SafeRelease(mat);
	return true;
}
	
bool SceneFile::ReadStaticMesh()
{

    bool rebuildTangentSpace = false;
    #ifdef REBUILD_TANGENT_SPACE_ON_IMPORT
    rebuildTangentSpace = true;
    #endif

	uint32 polyGroupCount;
	sceneFP->Read(&polyGroupCount, sizeof(uint32));
	if (debugLogEnabled)Logger::FrameworkDebug("- Static Mesh: %d\n", polyGroupCount);
	
	StaticMesh * mesh = new StaticMesh(scene);
    
	for (uint32 polyGroupIndex = 0; polyGroupIndex < polyGroupCount; polyGroupIndex++)
	{
        PolygonGroup * polygonGroup = new PolygonGroup();
        mesh->AddNode(polygonGroup);
        
		uint32 vertexCount, indexCount, vertexFormat;
        sceneFP->Read(&vertexFormat, sizeof(uint32));
		sceneFP->Read(&vertexCount, sizeof(uint32));
		sceneFP->Read(&indexCount, sizeof(uint32));
        if (debugLogEnabled)Logger::FrameworkDebug("--- vertex format: %x\n", vertexFormat); 
		if (debugLogEnabled)Logger::FrameworkDebug("--- vertex count: %d\n", vertexCount);
		if (debugLogEnabled)Logger::FrameworkDebug("--- index count: %d\n", indexCount);
		
		polygonGroup->AllocateData(vertexFormat, vertexCount, indexCount);
		
		for (uint32 v = 0; v < vertexCount; ++v)
		{
			Vector3 position, normal, tangent, binormal; 
			Vector2 texCoords0, texCoords1;
            
            if (polygonGroup->GetFormat() & EVF_VERTEX)
            {
                sceneFP->Read(&position, sizeof(Vector3));
                polygonGroup->SetCoord(v, position);
            }
            
            if (polygonGroup->GetFormat() & EVF_NORMAL)
            {
                sceneFP->Read(&normal, sizeof(Vector3));
                polygonGroup->SetNormal(v, normal);
                //Logger::FrameworkDebug("loadnorm: %f %f %f", normal.x, normal.y, normal.z);
            }

            if (polygonGroup->GetFormat() & EVF_TANGENT)
            {
                sceneFP->Read(&tangent, sizeof(Vector3));
                polygonGroup->SetTangent(v, tangent);
                //Logger::FrameworkDebug("loadnorm: %f %f %f", normal.x, normal.y, normal.z);
            }
            if (polygonGroup->GetFormat() & EVF_BINORMAL)
            {
                sceneFP->Read(&binormal, sizeof(Vector3));
                polygonGroup->SetBinormal(v, binormal);
                //Logger::FrameworkDebug("loadnorm: %f %f %f", normal.x, normal.y, normal.z);
            }
            
			if (polygonGroup->GetFormat() & EVF_TEXCOORD0)
            {
                sceneFP->Read(&texCoords0, sizeof(Vector2));
                polygonGroup->SetTexcoord(0, v, texCoords0);
            }
            
            if (polygonGroup->GetFormat() & EVF_TEXCOORD1)
            {
                sceneFP->Read(&texCoords1, sizeof(Vector2));
                polygonGroup->SetTexcoord(1, v, texCoords1);
            }
		}        
		
		int * indices = new int[indexCount];
		sceneFP->Read(indices, sizeof(int) * indexCount);
		for (uint32 i = 0; i < indexCount; ++i)
        {
			polygonGroup->SetIndex(i, indices[i]);
        }
        delete [] indices;

        const int32 prerequiredFormat = EVF_TANGENT | EVF_BINORMAL | EVF_NORMAL;
        if (rebuildTangentSpace&&((polygonGroup->GetFormat()&prerequiredFormat) == prerequiredFormat))
            MeshUtils::RebuildMeshTangentSpace(polygonGroup, true);
        else
            polygonGroup->BuildBuffers();
        
        SafeRelease(polygonGroup);
	}
	
	//scene->AddStaticMesh(mesh);
    staticMeshes.push_back(SafeRetain(mesh));
	SafeRelease(mesh);
	
	return true;
}
	
bool SceneFile::ReadAnimatedMesh()
{
	int polyGroupCount;
	sceneFP->Read(&polyGroupCount, sizeof(int));
	if (debugLogEnabled)Logger::FrameworkDebug("- Animated Mesh: %d\n", polyGroupCount);
	
	AnimatedMesh * mesh = new AnimatedMesh(scene);
	//mesh->Create(polyGroupCount);
    
	for (int polyGroupIndex = 0; polyGroupIndex < polyGroupCount; polyGroupIndex++)
	{
        PolygonGroup * polygonGroup = new PolygonGroup();
        mesh->AddNode(polygonGroup);
        
		int vertexCount, indexCount;
		sceneFP->Read(&vertexCount, sizeof(int));
		sceneFP->Read(&indexCount, sizeof(int));
		if (debugLogEnabled)Logger::FrameworkDebug("--- vertex count: %d\n", vertexCount);
		if (debugLogEnabled)Logger::FrameworkDebug("--- index count: %d\n", indexCount);
		
		
		polygonGroup->AllocateData(EVF_VERTEX | EVF_NORMAL | EVF_COLOR | EVF_TEXCOORD0 | EVF_JOINTINDEX | EVF_JOINTWEIGHT, vertexCount, indexCount);
		
		for (int v = 0; v < vertexCount; ++v)
		{
			Vector3 position, normal; 
			Vector2 texCoords0, texCoords1;
			int32 jointIdx;
			float32 weight;
			int32 jointCount;
			
			sceneFP->Read(&position, sizeof(Vector3));
			sceneFP->Read(&normal, sizeof(Vector3));
			sceneFP->Read(&texCoords0, sizeof(Vector2));
			sceneFP->Read(&texCoords1, sizeof(Vector2));
			
			sceneFP->Read(&jointCount, sizeof(int32));
			for (int k = 0; k < jointCount; ++k)
			{
				sceneFP->Read(&jointIdx, sizeof(int32));
				sceneFP->Read(&weight, sizeof(float32));
				polygonGroup->SetJointIndex(v, k, jointIdx);
				polygonGroup->SetJointWeight(v, k, weight);
			}
			
			polygonGroup->SetCoord(v, position);
			polygonGroup->SetNormal(v, normal);
			polygonGroup->SetTexcoord(0, v, texCoords0);
			polygonGroup->SetJointCount(v, jointCount);
		}
        
		int32 * indices = new int32[indexCount];
		sceneFP->Read(indices, indexCount *  sizeof(int32));
		for (int i = 0; i < indexCount; ++i)
        {
			polygonGroup->SetIndex(i, indices[i]);
        }
        delete [] indices;
		
        polygonGroup->BuildBuffers();
		
		polygonGroup->CreateBaseVertexArray();
        
        SafeRelease(polygonGroup);
	}
	Matrix4 bindShapeMatrix;
	sceneFP->Read(&bindShapeMatrix, sizeof(Matrix4));
	mesh->bindShapeMatrix = bindShapeMatrix;
	scene->AddAnimatedMesh(mesh);
	
	int32  boneCount = 0;
	sceneFP->Read(&boneCount, sizeof(int32));
	for (int32 bone = 0; bone < boneCount; ++bone)
	{
		char8 boneNodeName[512];
		sceneFP->ReadString(boneNodeName, 512);
		mesh->boneNames.push_back(FastName(boneNodeName));
	}
	
	SafeRelease(mesh);
	return true;
}


bool SceneFile::ReadSceneNode(Entity * parentNode, int level)
{
	if (sceneFP->IsEof())return false;

	int32 id;
	char8 name[512];
	sceneFP->Read(&id, sizeof(id));
	sceneFP->ReadString(name, 512);

	SceneFile::SceneNodeDef def;
    sceneFP->Read(&def, sizeof(def));		

	char8 nodeType[64];
	nodeType[0] = 0;
	
	
	Entity * node = 0;
	if (def.nodeType == SceneNodeDef::SCENE_NODE_BASE)
	{
		node = new Entity();
		node->SetLocalTransform(def.localTransform);
		node->SetName(name);
        if (parentNode != scene) 
        {
            parentNode->AddNode(node);
        }
		strcpy(nodeType, "scene node");
	}else if (def.nodeType == SceneNodeDef::SCENE_NODE_SKELETON)
	{
		Matrix4 inverse0;
		sceneFP->Read(&inverse0, sizeof(Matrix4));
		
		
		currentSkeletonNode = new SkeletonNode();
		node = currentSkeletonNode;
		node->SetLocalTransform(def.localTransform);
		currentSkeletonNode->inverse0Matrix = inverse0;
		node->SetName(name);
        if (parentNode != scene) 
        {
            parentNode->AddNode(node);
        }
		strcpy(nodeType, "skeleton node");
	}else if (def.nodeType == SceneNodeDef::SCENE_NODE_BONE)
	{
		Matrix4 inverse0;
		sceneFP->Read(&inverse0, sizeof(Matrix4));

		BoneNode * boneNode = new BoneNode(currentSkeletonNode);
		node = boneNode;
		node->SetLocalTransform(def.localTransform);
		node->SetName(name);

		boneNode->inverse0Matrix = inverse0;    // TODO: make inverse0Matrix protected
        if (parentNode != scene) 
        {
            parentNode->AddNode(node);
        }
		strcpy(nodeType, "bone node");
	}else if (def.nodeType == SceneNodeDef::SCENE_NODE_CAMERA)
	{
		int32 camIndex = -1;
		sceneFP->Read(&camIndex, sizeof(int32));
		
		//Camera * cam = SafeRetain(scene->GetCamera(camIndex + cameraIndexOffset));
		//node = cam;//new Camera(scene);
        
        //        node->SetDefaultLocalTransform(def.localTransform);
        //		node->SetLocalTransform(def.localTransform);
        //		node->SetName(name);
        //
        //        if (parentNode != scene) 
        //        {
        //            parentNode->AddNode(node);
        //        }
        //		cam->RestoreOriginalSceneTransform();
		strcpy(nodeType, "camera node");
	}
	else if (def.nodeType == SceneNodeDef::SCENE_NODE_MESH || def.nodeType == SceneNodeDef::SCENE_NODE_ANIMATED_MESH)
	{
		if (def.nodeType == SceneNodeDef::SCENE_NODE_MESH)strcpy(nodeType, "static mesh node");
		if (def.nodeType == SceneNodeDef::SCENE_NODE_ANIMATED_MESH)strcpy(nodeType, "animated mesh node");
		
		MeshInstanceNode* meshNode = new MeshInstanceNode();
		node = meshNode;
        
		node->SetLocalTransform(def.localTransform);
		node->SetName(name);
        
		int pgInstancesCount = 0;
		sceneFP->Read(&pgInstancesCount, sizeof(int));

		for (int k = 0; k < pgInstancesCount; ++k)
		{
			int meshIndex, polyGroupIndex, materialIndex;
			sceneFP->Read(&meshIndex, sizeof(int32));
			sceneFP->Read(&polyGroupIndex, sizeof(int32));
			sceneFP->Read(&materialIndex, sizeof(int32));
            
            DVASSERT(materialIndex < (int)materials.size());
            DVASSERT(0 <= materialIndex);

            NMaterial* material = (0 <= materialIndex) ? materials[materialIndex] : NULL;
            if (debugLogEnabled)
                Logger::FrameworkDebug("%s polygon group: meshIndex:%d polyGroupIndex:%d materialIndex:%d\n", GetIndentString('-', level + 1).c_str(), meshIndex, polyGroupIndex, materialIndex);

            if (def.nodeType == SceneNodeDef::SCENE_NODE_MESH)
            {
                StaticMesh* staticMesh = staticMeshes[meshIndex]; // staticMeshIndexOffset);
                meshNode->AddPolygonGroup(staticMesh, polyGroupIndex, material);
            }
            else
            {
                // add animated mesh
				AnimatedMesh * animatedMesh = scene->GetAnimatedMesh(meshIndex + animatedMeshIndexOffset);
				meshNode->AddPolygonGroup(animatedMesh, polyGroupIndex, material);
			}
		}
        if (parentNode != scene) 
        {
            parentNode->AddNode(node);
        }
	}
	if (debugLogEnabled)Logger::FrameworkDebug("%s node: %s typeId: %d childCount: %d type: %s\n", GetIndentString('-', level).c_str(), name, def.nodeType, def.childCount, nodeType);

    if (parentNode == scene)
    {
        scene->AddNode(node);
        rootNode = node;
    }

	for (int k = 0; k < def.childCount; ++k)
	{
		if (!ReadSceneNode(node, level + 1))return false;
	}

    if (parentNode == scene) 
    {//we should process lod loading when all scene graph is ready
        ProcessLOD(node);
    }
    
	SafeRelease(node);
	
	// clear skeleton node 
	if (def.nodeType == SceneNodeDef::SCENE_NODE_SKELETON)
		currentSkeletonNode = 0;
	return true;
}
	
bool SceneFile::ReadSceneGraph()
{
	currentSkeletonNode = 0;
	ReadSceneNode(scene, 0);
	return true;
}

bool SceneFile::ReadCamera()
{
	Camera * cam = new Camera();
	scene->AddCamera(cam);
	
	// read camera options
	
	CameraDef cd;
	sceneFP->Read(&cd, sizeof(CameraDef));
	
	cam->SetupPerspective(cd.fovy, 320.0f / 480.0f, cd.znear, cd.zfar);
	SafeRelease(cam);
	return true;
}

bool SceneFile::ReadAnimation()
{
	SceneNodeAnimationList * animationList = new SceneNodeAnimationList();
	
	char name[512];
	sceneFP->ReadString(name, 512);
	animationList->SetName(name);

	
	int nodeCount;
	sceneFP->Read(&nodeCount, sizeof(int32));
	
	if (debugLogEnabled)Logger::FrameworkDebug("- scene node anim list: %s nodeCount: %d\n", name, nodeCount); 

	for (int nodeIndex = 0; nodeIndex < nodeCount; ++nodeIndex)
	{
		sceneFP->ReadString(name, 512);
		
		float32 duration = 0;
		sceneFP->Read(&duration, sizeof(float32));
		
		int32 keyCount = 0;
		sceneFP->Read(&keyCount, sizeof(int32));

		SceneNodeAnimation * anim = new SceneNodeAnimation(keyCount);
		anim->SetBindName(FastName(name));	
		anim->SetDuration(duration); 
		if (debugLogEnabled)Logger::FrameworkDebug("-- scene node %d anim: %s keyCount: %d duration: %f seconds\n", nodeIndex, name, keyCount, duration); 

		sceneFP->Read(anim->invPose.data, sizeof(anim->invPose.data));
		for (int k = 0; k < keyCount; ++k)
		{
			SceneNodeAnimationKey key;
			sceneFP->Read(&key.time, sizeof(float32));
			sceneFP->Read(&key.translation, sizeof(Vector3));
			sceneFP->Read(&key.rotation, sizeof(Quaternion));
			sceneFP->Read(&key.scale, sizeof(Vector3));
			anim->SetKey(k, key);
			//printf("---- key: %f tr: %f %f %f q: %f %f %f %f\n", key.time, key.translation.x, key.translation.y, key.translation.z
			//	   , key.rotation.x, key.rotation.y, key.rotation.z, key.rotation.w); 
		}
		animationList->AddAnimation(anim);
		SafeRelease(anim);
	}
	animations.push_back(animationList);
	return true;
}
	
bool SceneFile::ReadLight()
{
	// read light options
	
	LightDef ld;
	sceneFP->Read(&ld, sizeof(LightDef));
	
	// TODO add light to light array
	
	return true;
}

void SceneFile::ProcessLOD(Entity *forRootNode)
{
//    if (scene->GetLodLayersCount() <= 0) 
//    {
//        return;
//    }
//    

    const int maxLodCount = 10;
    List<Entity*> lodNodes;
    forRootNode->FindNodesByNamePart("_lod0", lodNodes);
    if (debugLogEnabled) 
    {
        Logger::FrameworkDebug("Find %d nodes with LOD", lodNodes.size());
    }
    for (List<Entity*>::iterator it = lodNodes.begin(); it != lodNodes.end(); it++)
    {
        String nodeName(String((*it)->GetName().c_str()), 0, (*it)->GetName().find("_lod0"));
        if (debugLogEnabled) 
        {
            Logger::FrameworkDebug("Processing LODs for %s", nodeName.c_str());
        }

        Entity *oldParent = (*it)->GetParent();
        LodNode *lodNode = new LodNode();
        lodNode->SetName(nodeName.c_str());
        for (int i = maxLodCount; i >= 0; i--) 
        {
            Entity *ln = (*it)->GetParent()->FindByName(Format("%s_lod%d", nodeName.c_str(), i).c_str());
            if (ln) 
            {//if layer is not a dummy
                if (debugLogEnabled) 
                {
                    Logger::FrameworkDebug("      Add LOD layer %d", i);
                }
                ln->Retain();
                ln->GetParent()->RemoveNode(ln);

                lodNode->AddNodeInLayer(ln, i);
                ln->Release();
            }
            else 
            {//if layer is dummy
                Entity *ln = (*it)->GetParent()->FindByName(Format("%s_lod%ddummy", nodeName.c_str(), i).c_str());
                if (ln) 
                {
                    if (debugLogEnabled) 
                    {
                        Logger::FrameworkDebug("      Add Dummy LOD layer %d", i);
                    }
                    ln->Retain();
                    ln->SetVisible(false);
                    ln->RemoveAllChildren();
                    ln->GetParent()->RemoveNode(ln);
                    
                    lodNode->AddNodeInLayer(ln, i);
                    ln->Release();
                }
            }
        }
        
        oldParent->AddNode(lodNode);
        SafeRelease(lodNode);
        
    }
    
    
//    List<Entity*> lodNodes;
//    forRootNode->FindNodesByNamePart("_lod0", lodNodes);
//    if (debugLogEnabled) 
//    {
//        Logger::FrameworkDebug("Find %d nodes with LOD", lodNodes.size());
//    }
//    for (List<Entity*>::iterator it = lodNodes.begin(); it != lodNodes.end(); it++)
//    {
//        String nodeName((*it)->GetName(), 0, (*it)->GetName().find("_lod0"));
//        if (debugLogEnabled) 
//        {
//            Logger::FrameworkDebug("Processing LODs for %s", nodeName.c_str());
//        }
//        bool isNeedInit = true;
//        Entity *newNode = new Entity(scene);
//        newNode->SetName(nodeName);
//        MeshInstanceNode *meshToAdd = new MeshInstanceNode(scene);
//        meshToAdd->SetName("instance_0");
//        newNode->AddNode(meshToAdd);
//        (*it)->GetParent()->AddNode(newNode);
//        for (int i = scene->GetLodLayersCount(); i >= 0; i--) 
//        {
//            Entity *ln = (*it)->GetParent()->FindByName(Format("%s_lod%d", nodeName.c_str(), i));
//            if (ln) 
//            {//if layer is not a dummy
//                MeshInstanceNode *mn = (MeshInstanceNode *)ln->FindByName("instance_0");
//                if (mn) 
//                {
//                    if (debugLogEnabled) 
//                    {
//                        Logger::FrameworkDebug("      Add LOD layer %d", i);
//                    }
//                    if (isNeedInit)
//                    {//we should init our new node from the first appeared real(not a dummy) node
//                        isNeedInit = false;
//                        newNode->SetLocalTransform(ln->GetLocalTransform());
//                        Entity *hnode = mn;
//                        while (true)
//                        {
//                            if (hnode->GetParent() == ln) 
//                            {
//                                break;
//                            }
//                            hnode = hnode->GetParent();
//                        }
//                        meshToAdd->SetLocalTransform(mn->AccamulateLocalTransform(hnode));
//                    }
//                    for (int32 n = 0; n < (int32)mn->GetMeshes().size(); n++) 
//                    {
//                        meshToAdd->AddPolygonGroupForLayer(i, mn->GetMeshes()[n], mn->GetPolygonGroupIndexes()[n], mn->GetMaterials()[n]);
//                    }
//                }
//
//                ln->GetParent()->RemoveNode(ln);
//            }
//            else 
//            {//if layer is dummy
//                Entity *ln = (*it)->GetParent()->FindByName(Format("%s_lod%ddummy", nodeName.c_str(), i));
//                if (ln) 
//                {
//                    if (debugLogEnabled) 
//                    {
//                        Logger::FrameworkDebug("      Add Dummy LOD layer %d", i);
//                    }
//                    meshToAdd->AddDummyLODLayer(i);
//
//                    ln->GetParent()->RemoveNode(ln);
//                }
//            }
//        }
//        
//        SafeRelease(newNode);
//        SafeRelease(meshToAdd);
//        
//    }
}

    
    
	
};



