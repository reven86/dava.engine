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


#include "stdafx.h"
#include "ColladaDocument.h"
#include "Scene3D/SceneFile.h"
#include "CommandLine/CommandLineParser.h"

///*
// INCLUDE DevIL
// */
//#ifdef _UNICODE
//#undef _UNICODE
//#undef UNICODE
//#define UNICODE_DISABLED
//#endif
//
//#include <IL/il.h>
//#include <IL/ilu.h>
//
//#ifdef UNICODE_DISABLED
//#define _UNICODE
//#endif

namespace DAVA
{

eColladaErrorCodes ColladaDocument::Open( const char * filename )
{
	document = FCollada::NewTopDocument();
	bool val = FCollada::LoadDocumentFromFile( document, FUStringConversion::ToFString(filename));
	if (!val)return COLLADA_ERROR;
	
    FCDSceneNode *loadedRootNode = document->GetVisualSceneInstance();
    if(!loadedRootNode)
    {
        return COLLADA_ERROR_OF_ROOT_NODE;
    }
    
	colladaScene = new ColladaScene(loadedRootNode);

	FCDGeometryLibrary * geometryLibrary = document->GetGeometryLibrary();
	
	DAVA::Logger::FrameworkDebug("* Export geometry: %d\n", (int)geometryLibrary->GetEntityCount());
	for (int entityIndex = 0; entityIndex < (int)geometryLibrary->GetEntityCount(); ++entityIndex)
	{
		FCDGeometry * geometry = geometryLibrary->GetEntity(entityIndex);
		if (geometry->IsMesh())
		{
			FCDGeometryMesh * geometryMesh = geometry->GetMesh();
			colladaScene->colladaMeshes.push_back(new ColladaMesh(geometryMesh, 0));
		}
	}

//	ilInit();
//	iluInit();
//
//	ilEnable(IL_ORIGIN_SET);
//	ilOriginFunc(IL_ORIGIN_LOWER_LEFT); 

	FCDImageLibrary * imageLibrary = document->GetImageLibrary();
	for (int entityIndex = 0; entityIndex < (int)imageLibrary->GetEntityCount(); ++entityIndex)
	{
		FCDImage * texture = imageLibrary->GetEntity(entityIndex);
		colladaScene->colladaTextures.push_back(new ColladaTexture(texture));
	}
//	ilShutDown();
	
	FCDLightLibrary * lightLibrary = document->GetLightLibrary();

	for (int entityIndex = 0; entityIndex < (int)lightLibrary->GetEntityCount(); ++entityIndex)
	{
		FCDLight * light = lightLibrary->GetEntity(entityIndex);
		colladaScene->colladaLights.push_back(new ColladaLight(light));
	}

	FCDMaterialLibrary * materialLibrary = document->GetMaterialLibrary();

	for (int entityIndex = 0; entityIndex < (int)materialLibrary->GetEntityCount(); ++entityIndex)
	{
		FCDMaterial * material = materialLibrary->GetEntity(entityIndex);
		colladaScene->colladaMaterials.push_back(new ColladaMaterial(colladaScene, material));
	}

	FCDCameraLibrary * cameraLibrary = document->GetCameraLibrary();
	DAVA::Logger::FrameworkDebug("Cameras:%d\n", cameraLibrary->GetEntityCount());
	
	for (int entityIndex = 0; entityIndex < (int)cameraLibrary->GetEntityCount(); ++entityIndex)
	{
		FCDCamera * cam = cameraLibrary->GetEntity(entityIndex);
		colladaScene->colladaCameras.push_back(new ColladaCamera(cam));
	}
	
	FCDAnimationLibrary * animationLibrary = document->GetAnimationLibrary();
	FCDAnimationClipLibrary * animationClipLibrary = document->GetAnimationClipLibrary();
	DAVA::Logger::FrameworkDebug("[A] Animations:%d Clips:%d\n", animationLibrary->GetEntityCount(), animationClipLibrary->GetEntityCount());


	FCDControllerLibrary * controllerLibrary = document->GetControllerLibrary();
	
	DAVA::Logger::FrameworkDebug("* Export animation controllers: %d\n", controllerLibrary->GetEntityCount());
	for (int entityIndex = 0; entityIndex < (int)controllerLibrary->GetEntityCount(); ++entityIndex)
	{
		FCDController * controller = controllerLibrary->GetEntity(entityIndex);
		colladaScene->colladaAnimatedMeshes.push_back(new ColladaAnimatedMesh(controller));
	}
	
	colladaScene->ExportScene();
	
	for (int k = 0; k < (int)colladaScene->colladaAnimatedMeshes.size();  ++k)
	{
		colladaScene->colladaAnimatedMeshes[k]->MarkJoints(colladaScene->rootNode);
	}
	
	ExportNodeAnimations(document, document->GetVisualSceneInstance());
	colladaScene->SetExclusiveAnimation(0);
	return COLLADA_OK;
}
	
bool ColladaDocument::ExportNodeAnimations(FCDocument * exportDoc, FCDSceneNode * exportNode)
{
	FCDAnimationLibrary * animationLibrary = exportDoc->GetAnimationLibrary();
	if (animationLibrary->GetEntityCount() == 0)
	{
		DAVA::Logger::Error("*** Can't find any animations in this file: %s\n", exportDoc->GetFileUrl().c_str());
		return false;
	}
	
	float32 timeStart, timeEnd;
	GetAnimationTimeInfo(exportDoc, timeStart, timeEnd);
	DAVA::Logger::FrameworkDebug("== Additional animation: %s start: %0.3f end: %0.3f\n ", exportDoc->GetFileUrl().c_str(), timeStart, timeEnd);
	
	FilePath fullPathName(exportDoc->GetFileUrl().c_str());
	String name = fullPathName.GetBasename();
	
	ColladaAnimation * anim = new ColladaAnimation();
	anim->name = name;//Format("animation:%d", colladaScene->colladaAnimations.size());
	anim->duration = timeEnd;
	colladaScene->ExportAnimations(anim, exportNode, timeStart, timeEnd);
	colladaScene->colladaAnimations.push_back(anim);
	return true;
}

bool ColladaDocument::ExportAnimations(const char * filename)
{
	FCDocument * exportDoc = FCollada::NewTopDocument();
	bool val = FCollada::LoadDocumentFromFile( exportDoc, FUStringConversion::ToFString(filename));
	if (!val)
	{
		DAVA::Logger::Error("*** Can't find file: %s\n", filename);
		return false;
	}
	FCDSceneNode * exportNode =  exportDoc->GetVisualSceneInstance();
	return ExportNodeAnimations(exportDoc, exportNode);;
}

void ColladaDocument::GetAnimationTimeInfo(FCDocument * document, float32 & retTimeStart, float32 & retTimeEnd)
{
	FCDAnimationLibrary * animationLibrary = document->GetAnimationLibrary();
	
	colladaScene->animationStartTime = 0.0f;
	colladaScene->animationEndTime = 0.0f;
	
	float32 timeMin, timeMax;
	for (int entityIndex = 0; entityIndex < (int)animationLibrary->GetEntityCount(); ++entityIndex)
	{
		FCDAnimation * anim = animationLibrary->GetEntity(entityIndex);
		// DAVA::Logger::FrameworkDebug("* Export animation: %d channelCount:%d\n", entityIndex, anim->GetChannelCount());
		
		timeMin = 10000000.0f;
		timeMax = 0.0f;
		
		for (int channelIndex = 0; channelIndex < (int)anim->GetChannelCount(); ++channelIndex)
		{
			FCDAnimationChannel * channel = anim->GetChannel(channelIndex);
			// DAVA::Logger::FrameworkDebug("- channel: %d curveCount: %d\n", channelIndex, channel->GetCurveCount());
			for (int curveIndex = 0; curveIndex < (int)channel->GetCurveCount(); ++curveIndex)
			{
				FCDAnimationCurve * curve = channel->GetCurve(curveIndex);
				// DAVA::Logger::FrameworkDebug("-- curve: %d target:%s\n", curveIndex, curve->GetTargetQualifier().c_str());
				for (int keyIndex = 0; keyIndex < (int)curve->GetKeyCount(); ++keyIndex)
				{
					FCDAnimationKey * key = curve->GetKey(keyIndex);
					if (key->input < timeMin)timeMin = key->input;
					if (key->input > timeMax)timeMax = key->input;
					
				}
			}
		}
		colladaScene->animationStartTime = Min(timeMin, colladaScene->animationStartTime);
		colladaScene->animationEndTime = Max(timeMax, colladaScene->animationEndTime);
		//DAVA::Logger::FrameworkDebug("- timeMin: %f timeMax: %f\n", timeMin, timeMax);  
	}
	
	retTimeStart = 0.0f;
	retTimeEnd = timeMax;
}
	
void ColladaDocument::Close()
{
	SAFE_RELEASE(document);
}

void ColladaDocument::Render()
{
	colladaScene->Render();
}

static const char* DEFAULT_COLLADA_EXTENSION = ".png";

String ColladaDocument::GetTextureName(const FilePath & scenePath, ColladaTexture * texture)
{
    FilePath texPathname(texture->texturePathName.c_str());
    DAVA::Logger::FrameworkDebug("+ get texture name: %s", texPathname.GetAbsolutePathname().c_str());
    
    auto extension = texPathname.GetExtension();
    if(!extension.empty())
    {
        if(!TextureDescriptor::IsSourceTextureExtension(extension) && !TextureDescriptor::IsCompressedTextureExtension(extension))
        {
            texPathname.ReplaceExtension(DEFAULT_COLLADA_EXTENSION);
        }
    }
    return texPathname.GetRelativePathname(scenePath);
}

void ColladaDocument::SaveScene( const FilePath & scenePath, const String & sceneName )
{
    DVASSERT(scenePath.IsDirectoryPathname());
    
	DAVA::Logger::FrameworkDebug("* Write begin: %s/%s\n", scenePath.GetAbsolutePathname().c_str(), sceneName.c_str());
	
	FilePath scenePathName = scenePath + sceneName;
	
	sceneFP = fopen(scenePathName.GetAbsolutePathname().c_str(), "wb");
	if (sceneFP == 0)return;
	
    
    
	// write empty header and later write data.
	// at the end of not collection we will rewrite header again.
	fwrite(&header, sizeof(SceneFile::Header), 1, sceneFP);

    //header.version = 
	header.materialCount = (uint32)colladaScene->colladaMaterials.size();
	header.staticMeshCount = (uint32)colladaScene->colladaMeshes.size();
	header.animatedMeshCount = (uint32)colladaScene->colladaAnimatedMeshes.size();
	header.cameraCount = (uint32)colladaScene->colladaCameras.size();
	header.nodeAnimationsCount = (uint32)colladaScene->colladaAnimations.size();
	header.lightCount = (uint32)colladaScene->colladaLights.size();
	
	/*for (uint32 textureIndex = 0; textureIndex < header.textureCount; ++textureIndex)
	{
		SceneFile::TextureDef texture;
	
		String textureRelativePathName = String(colladaScene->colladaTextures[textureIndex]->texturePathName.c_str());
		CommandLineParser::RemoveFromPath(textureRelativePathName, scenePath);
		
		if (textureRelativePathName.c_str()[0] == '/')
			textureRelativePathName.erase(0, 1);

		if (textureRelativePathName.substr(0, 2) == "./")
			textureRelativePathName.erase(0,2);
		
		texture.id = textureIndex;
		strcpy(texture.name, textureRelativePathName.c_str());
		
		// HACK for .jpg into .png
        // work for all extensions
		std::string texname(texture.name);
		int32 pos = texname.find(".");
		if(-1 != pos)
		{
			texname.replace(pos, 4, ".png");
		}
		
		
		strcpy(texture.name, texname.c_str());
        texture.hasOpacity = colladaScene->colladaTextures[textureIndex]->hasOpacity;
		////
		
		DAVA::Logger::FrameworkDebug("- texture: %s %d\n", texture.name, texture.id);
		WriteTexture(&texture);
	}*/
	
	for (uint32 materialIndex = 0; materialIndex < header.materialCount; ++materialIndex)
	{
		SceneFile::MaterialDef material;
		ColladaMaterial * colladaMaterial = colladaScene->colladaMaterials[materialIndex];
		
		
		strcpy(material.name, colladaMaterial->material->GetDaeId().c_str());
		
		// 
		material.ambient.x = colladaMaterial->ambient.x;
		material.ambient.y = colladaMaterial->ambient.y;
		material.ambient.z = colladaMaterial->ambient.z;
		material.ambient.w = colladaMaterial->ambient.w;

		//
		material.diffuse.x = colladaMaterial->diffuse.x;
		material.diffuse.y = colladaMaterial->diffuse.y;
		material.diffuse.z = colladaMaterial->diffuse.z;
		material.diffuse.w = colladaMaterial->diffuse.w;
		
		//
		material.specular.x = colladaMaterial->diffuse.x;
		material.specular.y = colladaMaterial->diffuse.y;
		material.specular.z = colladaMaterial->diffuse.z;
		material.specular.w = colladaMaterial->diffuse.w;
        
        material.shininess = colladaMaterial->shininess;
		
        material.hasOpacity = false;
		
//		for (uint32 textureIndex = 0; textureIndex < colladaScene->colladaTextures.size(); ++textureIndex)
//			if (colladaMaterial->diffuseTexture == colladaScene->colladaTextures[textureIndex])
//			{
//				material.diffuseTextureId = textureIndex;
//                material.hasOpacity = colladaMaterial->diffuseTexture->hasOpacity;
//                break;
//			}
        
        if (colladaMaterial->diffuseTexture)
        {
            String diffuseTextureName = GetTextureName(scenePath, colladaMaterial->diffuseTexture);
            strcpy(material.diffuseTexture, diffuseTextureName.c_str());
            material.hasOpacity = colladaMaterial->diffuseTexture->hasOpacity;
        }
        
        if ((colladaMaterial->lightmapTexture) && (colladaMaterial->hasLightmapTexture))
        {
//            String textureRelativePathName = String(colladaMaterial->lightmapTexture->texturePathName.c_str());
//            CommandLineParser::RemoveFromPath(textureRelativePathName, scenePath);
//            
//            if (textureRelativePathName.c_str()[0] == '/')
//                textureRelativePathName.erase(0, 1);
//            
//            if (textureRelativePathName.substr(0, 2) == "./")
//                textureRelativePathName.erase(0,2);
            
            strcpy(material.lightmapTexture, GetTextureName(scenePath, colladaMaterial->lightmapTexture).c_str()); 
        }else
        {
            strcpy(material.lightmapTexture,"");
        }
        
		DAVA::Logger::FrameworkDebug("- material: %s diffuse texture: %s idx:%d\n", material.name, material.diffuseTexture, materialIndex); 
		WriteMaterial(&material);
	}
	
	for (uint32 staticMeshIndex = 0; staticMeshIndex < header.staticMeshCount; ++staticMeshIndex)
	{
		ColladaMesh * mesh = colladaScene->colladaMeshes[staticMeshIndex];
		WriteStaticMesh(mesh, staticMeshIndex);
	}
	
	for (uint32 animatedMeshIndex = 0; animatedMeshIndex < header.animatedMeshCount; ++animatedMeshIndex)
	{
		ColladaAnimatedMesh * animatedMesh = colladaScene->colladaAnimatedMeshes[animatedMeshIndex];
		WriteAnimatedMesh(animatedMesh, animatedMeshIndex);
	}
	
	for (uint32 lightIndex = 0; lightIndex < header.lightCount; ++lightIndex)
	{
		ColladaLight * l = colladaScene->colladaLights[lightIndex];
		WriteLight(l, lightIndex);
	}
	
	for (uint32 camIndex = 0; camIndex < header.cameraCount; ++camIndex)
	{
		ColladaCamera * cam = colladaScene->colladaCameras[camIndex];
		WriteCamera(cam, camIndex);
	}
	
	for (uint32 nodeAnimIndex = 0; nodeAnimIndex < header.nodeAnimationsCount; ++nodeAnimIndex)
	{
		ColladaAnimation * animation = colladaScene->colladaAnimations[nodeAnimIndex];
		WriteNodeAnimationList(animation);
	}
	
	
	
	DAVA::Logger::FrameworkDebug("* Write scene graph:\n");
	// write scene graph
	int nodeId = 0;
	WriteSceneNode(colladaScene->rootNode, nodeId, -1, 0);
	
	fseek(sceneFP, 0, SEEK_SET);
	fwrite(&header, sizeof(SceneFile::Header), 1, sceneFP);

	fclose(sceneFP);
	DAVA::Logger::FrameworkDebug("* Write end\n");
	DAVA::Logger::FrameworkDebug("===============\n");

//	DAVA::Logger::FrameworkDebug("* Verify start: %s\n", scenePathName.c_str());
//	SceneFile file;
//	Scene * scene = new Scene();
//	file.LoadScene(scenePathName.c_str(), scene, false);
//	DAVA::Logger::FrameworkDebug("* Verify end\n");
}
	
void ColladaDocument::WriteTexture(SceneFile::TextureDef * texture)
{
	fwrite(texture->name, strlen(texture->name) + 1, 1, sceneFP);
    if (header.version >= 101)
    {
        fwrite(&texture->hasOpacity, sizeof(texture->hasOpacity), 1, sceneFP);
    }
}
	
void ColladaDocument::WriteMaterial(SceneFile::MaterialDef * material)
{
	fwrite(material->name, strlen(material->name) + 1, 1, sceneFP);

	fwrite(&material->ambient, sizeof(material->ambient), 1, sceneFP);
	fwrite(&material->diffuse, sizeof(material->diffuse), 1, sceneFP);
	fwrite(&material->emission, sizeof(material->emission), 1, sceneFP);
	fwrite(&material->indexOfRefraction, sizeof(material->indexOfRefraction), 1, sceneFP);
	fwrite(&material->reflective, sizeof(material->reflective), 1, sceneFP);
	fwrite(&material->reflectivity, sizeof(material->reflectivity), 1, sceneFP);
	fwrite(&material->shininess, sizeof(material->shininess), 1, sceneFP);
	fwrite(&material->specular, sizeof(material->specular), 1, sceneFP);
	fwrite(&material->transparency, sizeof(material->transparency), 1, sceneFP);
	fwrite(&material->transparent, sizeof(material->transparent), 1, sceneFP);
    
    fwrite(material->diffuseTexture, strlen(material->diffuseTexture) + 1, 1, sceneFP);
    fwrite(material->lightmapTexture, strlen(material->lightmapTexture) + 1, 1, sceneFP);
    fwrite(material->reflectiveTexture, strlen(material->reflectiveTexture) + 1, 1, sceneFP);
    fwrite(material->specularTexture, strlen(material->specularTexture) + 1, 1, sceneFP);
    fwrite(material->normalMapTexture, strlen(material->normalMapTexture) + 1, 1, sceneFP);
    
    fwrite(&material->hasOpacity, sizeof(material->hasOpacity), 1, sceneFP);
}


static void FlipTexCoords(Vector2 & v)
{
    v.y = 1.0f - v.y;
}
	
void ColladaDocument::WriteStaticMesh(ColladaMesh * mesh, int meshIndex)
{
	//fwrite(&mesh->id, sizeof(int), 1, sceneFP);
	uint32 groupCount = mesh->GetPolygonGroupCount();
	fwrite(&groupCount, sizeof(uint32), 1, sceneFP);
	
	DAVA::Logger::FrameworkDebug("- static mesh: %s idx: %d groupCount: %d\n", mesh->name.c_str(), meshIndex, groupCount); 

	for (int k = 0; k < (int)groupCount; ++k)
	{
		ColladaPolygonGroup * group = mesh->GetPolygonGroup(k);
		std::vector<ColladaVertex> & vertices = group->GetVertices();
		std::vector<int> & indices = group->GetIndices();
        uint32 vertexFormat = group->GetVertexFormat();
		uint32 vertexCount = vertices.size();
		uint32 indexCount = group->GetTriangleCount() * 3;
		
        fwrite(&vertexFormat, sizeof(uint32), 1, sceneFP);
		fwrite(&vertexCount, sizeof(uint32), 1, sceneFP);
		fwrite(&indexCount, sizeof(uint32), 1, sceneFP);

		DAVA::Logger::FrameworkDebug("    group: %d vertexCount: %d indexCount:%d\n", k, vertexCount, indexCount); 
			
		for (int vi = 0; vi < (int)vertexCount; ++vi)
		{
//			ColladaVertex
//			Vector3 position;	
//			Vector3 normal;
//			Vector3 tangent;
//		    Vector3 binormal;
//			Vector2 texCoords[4];
//			int		jointCount;
//			int		joint[20];
//			float	weight[20];
			ColladaVertex & v = vertices[vi];
            if (vertexFormat & EVF_VERTEX)
                fwrite(&v.position, sizeof(Vector3), 1, sceneFP);
			if (vertexFormat & EVF_NORMAL)
            {
                v.normal.Normalize();
                fwrite(&v.normal, sizeof(Vector3), 1, sceneFP);
                //Logger::FrameworkDebug("normal: %f %f %f", v.normal.x, v.normal.y, v.normal.z);
			}            
            if (vertexFormat & EVF_TANGENT)
            {
                v.tangent.Normalize();
                fwrite(&v.tangent, sizeof(Vector3), 1, sceneFP);
                //Logger::FrameworkDebug("normal: %f %f %f", v.normal.x, v.normal.y, v.normal.z);
			}
            if (vertexFormat & EVF_BINORMAL)
            {
                v.binormal.Normalize();
                fwrite(&v.binormal, sizeof(Vector3), 1, sceneFP);
                //Logger::FrameworkDebug("normal: %f %f %f", v.normal.x, v.normal.y, v.normal.z);
            }
            
            if (vertexFormat & EVF_TEXCOORD0)
            {
                FlipTexCoords(v.texCoords[0]);
                fwrite(&v.texCoords[0], sizeof(Vector2), 1, sceneFP);
			}
            if (vertexFormat & EVF_TEXCOORD1)
            {
                FlipTexCoords(v.texCoords[1]);
                fwrite(&v.texCoords[1], sizeof(Vector2), 1, sceneFP);
            }
		}
		
		fwrite(&indices.front(), sizeof(int), indexCount, sceneFP);
	}
}
	
void ColladaDocument::WriteAnimatedMesh(ColladaAnimatedMesh * animMesh, int meshIndex)
{
	ColladaMesh * mesh = animMesh->mesh;
	int groupCount = mesh->GetPolygonGroupCount();
	fwrite(&groupCount, sizeof(int32), 1, sceneFP);
	
	DAVA::Logger::FrameworkDebug("- animated mesh: %s idx: %d groupCount: %d\n", mesh->name.c_str(), meshIndex, groupCount); 
	
	for (int k = 0; k < groupCount; ++k)
	{
		ColladaPolygonGroup * group = mesh->GetPolygonGroup(k);
		std::vector<ColladaVertex> & vertices = group->GetVertices();
		std::vector<int> & indices = group->GetIndices();
		int32 vertexCount = (int32)vertices.size();
		int32 indexCount = group->GetTriangleCount() * 3;
		
		fwrite(&vertexCount, sizeof(int32), 1, sceneFP);
		fwrite(&indexCount, sizeof(int32), 1, sceneFP);
		
		DAVA::Logger::FrameworkDebug("    group: %d vertexCount: %d indexCount:%d\n", k, vertexCount, indexCount); 
		
		for (int vi = 0; vi < vertexCount; ++vi)
		{
			//			ColladaVertex
			//			Vector3 position;	
			//			Vector3 normal;
			//			Vector3 tangent;
			//		    Vector3 binormal;
			//			Vector2 texCoords[4];
			//			int		jointCount;
			//			int		joint[20];
			//			float	weight[20];
			//
			ColladaVertex & v = vertices[vi];
			fwrite(&v.position, sizeof(Vector3), 1, sceneFP);
			fwrite(&v.normal, sizeof(Vector3), 1, sceneFP);
			
			FlipTexCoords(v.texCoords[0]);
			FlipTexCoords(v.texCoords[1]);
			
			fwrite(&v.texCoords[0], sizeof(Vector2), 1, sceneFP);
			fwrite(&v.texCoords[1], sizeof(Vector2), 1, sceneFP);
			fwrite(&v.jointCount, sizeof(int32), 1, sceneFP);
			
			if (v.jointCount > 4)
			{
				DAVA::Logger::FrameworkDebug("-- WARNING: JOINT COUNT MORE THAN 4 : %d", v.jointCount);
			}
			
			
			for (int k = 0; k < v.jointCount; ++k)
			{
				fwrite(&v.joint[k], sizeof(int32), 1, sceneFP);
				fwrite(&v.weight[k], sizeof(float32), 1, sceneFP);
			}	
		}
		
		fwrite(&indices.front(), indexCount, sizeof(int32), sceneFP);
	}
	
	fwrite(&animMesh->bindShapeMatrix, sizeof(Matrix4), 1, sceneFP);
	
	int32 boneCount = animMesh->joints.size();
	fwrite(&boneCount, sizeof(int32), 1, sceneFP);
	for (int jointIndex = 0; jointIndex < (int)animMesh->joints.size(); ++jointIndex)
	{
		ColladaSceneNode * node =  animMesh->joints[jointIndex].node; 
		char name[512];
		strcpy(name, node->originalNode->GetDaeId().c_str());
		fwrite(name, strlen(name) + 1, 1, sceneFP);
	}
}

bool ColladaDocument::IsEmptyNode(ColladaSceneNode * node)
{
	if ((node->childs.size() == 0) && (node->meshInstances.size() == 0) && (node->lights.size() == 0) && (!node->isJoint))
		return true;
	return false;
}

/*
	node - original collada node
	globalNodeId - counter to make different node ids accross all nodes
	parentId - id of parent to store it in file
	level = value from 0 (hierarhy level of this particular node)
 */
void ColladaDocument::WriteSceneNode(ColladaSceneNode * node, int &globalNodeId, int32 parentId, int32 level)
{	
	 // skip empty nodes
	//if (IsEmptyNode(node))return;
																		   
	int nodeId = globalNodeId;
    std::string name(node->originalNode->GetDaeId());
    if (name.find("node-") == 0)
    {//if node name begins from "node-"
        name = name.substr(strlen("node-"));
    }
//	strcpy(name, node->originalNode->GetDaeId().c_str());
		
	
	fwrite(&nodeId, sizeof(int32), 1, sceneFP);
	fwrite(name.c_str(), name.length() + 1, 1, sceneFP);
	// write node information
	SceneFile::SceneNodeDef def;
	def.parentId = parentId;
	def.localTransform = node->localTransform;
	def.nodeType = SceneFile::SceneNodeDef::SCENE_NODE_BASE;
	def.customDataSize = 0;
	def.childCount = node->childs.size() + node->meshInstances.size() + node->cameras.size();
	
	// mark skeleton nodes for easy skeleton calc
	if (node->isJoint)
	{
		def.nodeType = SceneFile::SceneNodeDef::SCENE_NODE_SKELETON;
		if ((node->parent) && (node->parent->isJoint))
		{
			def.nodeType = SceneFile::SceneNodeDef::SCENE_NODE_BONE;
		}		
	}

	DAVA::Logger::FrameworkDebug("%s Write scene node: %s childCount: %d isJoint: %d\n", GetIndentString('-', level + 1).c_str(), name.c_str(), def.childCount, (int)node->isJoint);
	
	fwrite(&def, sizeof(def), 1, sceneFP);
	
	if (node->isJoint)	// for all joints write inverse0 matrix
	{
		fwrite(&node->inverse0, sizeof(Matrix4), 1, sceneFP);
	}
	
	if (node->meshInstances.size() != 0)
	{
		for (int i = 0; i < (int)node->meshInstances.size(); ++i)
			WriteMeshNode(node->meshInstances[i], ++globalNodeId, nodeId, level + 1, i);	
	}
	
	if (node->cameras.size() != 0)
	{
		for (int i = 0; i < (int)node->cameras.size(); ++i)
			WriteCameraNode(node->cameras[i], ++globalNodeId, nodeId, level + 1);	
	}
	
	for (int i = 0; i < (int)node->childs.size(); ++i)
	{
		ColladaSceneNode * childNode = node->childs[i];
		WriteSceneNode(childNode, ++globalNodeId, nodeId, level + 1);
	}
	
	
}
					  
void ColladaDocument::WriteMeshNode(ColladaMeshInstance * node, int32 & globalNodeId, int32 parentId, int32 level, int32 instanceIdx)
{
	int nodeId = globalNodeId;
	char name[512];
	strcpy(name, Format("instance_%d", instanceIdx).c_str());
	
	fwrite(&nodeId, sizeof(int), 1, sceneFP);
	fwrite(name, strlen(name) + 1, 1, sceneFP);
	
	
	DAVA::Logger::FrameworkDebug("%s Write mesh instance node: %s\n", GetIndentString('-', level + 1).c_str(),  name);
	
	// write node information
	SceneFile::SceneNodeDef def;
	def.parentId = parentId;
	def.localTransform.Identity();
	
	def.nodeType = SceneFile::SceneNodeDef::SCENE_NODE_MESH;
	if (node->IsAnimated())
		def.nodeType = SceneFile::SceneNodeDef::SCENE_NODE_ANIMATED_MESH;

	def.customDataSize = node->polyGroupInstances.size() * 8 + 4; 
	def.childCount = 0;
	
	fwrite(&def, sizeof(def), 1, sceneFP);
	
	int pgInstancesCount = node->polyGroupInstances.size();
	fwrite(&pgInstancesCount, sizeof(int), 1, sceneFP);
	for (int pgi = 0; pgi < pgInstancesCount; ++pgi)
	{
		ColladaPolygonGroupInstance * polyGroupInstance = node->polyGroupInstances[pgi];
        int32 materialIndex = colladaScene->FindMaterialIndex(polyGroupInstance->material);
		if (materialIndex == -1)
		{
			DAVA::Logger::Error("*** Error: failed to find material index\n");
		}
		
		int32 meshIndex = 0;
		int32 polyGroupIndex = 0;
		
		if (!colladaScene->FindPolyGroupIndex(polyGroupInstance->polyGroup, meshIndex, polyGroupIndex))
		{
			DAVA::Logger::Error("- search : 0x%p\n", polyGroupInstance->polyGroup);
			DAVA::Logger::Error("*** Error: failed to find poly group index\n");
		}
		
				
		DAVA::Logger::FrameworkDebug("%s Write poly instance node: %d materialIdx: %d meshIndex: %d polygroupIndex: %d\n",GetIndentString('-', level + 1).c_str(), pgi, materialIndex, meshIndex, polyGroupIndex);
		fwrite(&meshIndex, sizeof(int32), 1, sceneFP);
		fwrite(&polyGroupIndex, sizeof(int32), 1, sceneFP);
		fwrite(&materialIndex, sizeof(int32), 1, sceneFP);
	}
}
	
//int ColladaDocument::WriteGeometryNodes( ColladaSceneNode * rootNode )
// {
// 			
// 
// 
// }
	
void ColladaDocument::WriteCameraNode(ColladaCamera * node, int32 & globalNodeId, int32 parentId, int32 level)
{
	int nodeId = globalNodeId;
	char name[64];
	strcpy(name, "camera_instance");
	
	fwrite(&nodeId, sizeof(int), 1, sceneFP);
	fwrite(name, strlen(name) + 1, 1, sceneFP);
		
	DAVA::Logger::FrameworkDebug("%s Write camera node: %s\n", GetIndentString('-', level + 1).c_str(), name);
	
	// write node information
	SceneFile::SceneNodeDef def;
	def.parentId = parentId;
	def.localTransform.Identity();
	// TODO set transform to camera position, rotation ???
	
	
	
	def.nodeType = SceneFile::SceneNodeDef::SCENE_NODE_CAMERA;
	
	def.customDataSize = sizeof(int);
	// camera id
	
	def.childCount = 0;
	
	fwrite(&def, sizeof(def), 1, sceneFP);		
	
	int camId = colladaScene->FindCameraIndex(node);
	
	fwrite(&camId, sizeof(int), 1, sceneFP);
}

void ColladaDocument::WriteNodeAnimationList(ColladaAnimation * animation)
{
	char name[512];
	strcpy(name, animation->name.c_str());
	fwrite(name, strlen(name) + 1, 1, sceneFP);
	
	int32 cnt = animation->animations.size();
	fwrite(&cnt, sizeof(int32), 1, sceneFP);
	
	DAVA::Logger::FrameworkDebug("- scene node anim list: %s\n", name); 
	for (Map<ColladaSceneNode*, SceneNodeAnimation*>::iterator it = animation->animations.begin(); it != animation->animations.end(); ++it)
	{
		ColladaSceneNode * node = it->first;
		SceneNodeAnimation * anim = it->second;
		
		std::string name(node->originalNode->GetDaeId());
		if (name.find("node-") == 0)
		{//if node name begins from "node-"
			name = name.substr(strlen("node-"));
		}
		fwrite(name.c_str(), name.length() + 1, 1, sceneFP);
		
		float32 duration = anim->GetDuration();
		fwrite(&duration, sizeof(float32), 1, sceneFP);
		
		int32 keyCount = anim->GetKeyCount();
		fwrite(&keyCount, sizeof(int32), 1, sceneFP);
		
		DAVA::Logger::FrameworkDebug("-- scene node anim: %s keyCount: %d\n", name.c_str(), keyCount);

		Matrix4 invPose;
		node->localTransform.GetInverse(invPose);
		fwrite(invPose.data, sizeof(invPose.data), 1, sceneFP);
		SceneNodeAnimationKey * keys = anim->GetKeys();
		for (int k = 0; k < anim->GetKeyCount(); ++k)
		{
			SceneNodeAnimationKey & key = keys[k];
			fwrite(&key.time, sizeof(float32), 1, sceneFP);
			fwrite(&key.translation, sizeof(Vector3), 1, sceneFP);
			fwrite(&key.rotation, sizeof(Quaternion), 1, sceneFP);
			fwrite(&key.scale, sizeof(Vector3), 1, sceneFP);
			
			//DAVA::Logger::FrameworkDebug("---- key: %f tr: %f %f %f q: %f %f %f %f\n", key.time, key.translation.x, key.translation.y, key.translation.z
			//, key.rotation.x, key.rotation.y, key.rotation.z, key.rotation.w); 
		}
	}
}
	
void ColladaDocument::WriteCamera(ColladaCamera * cam, int32 i)
{
	// write fov/ortho...
	DAVA::Logger::FrameworkDebug("write camera %i\n", i);
	SceneFile::CameraDef cd;
	
	if (cam->camera->HasHorizontalFov())
		cd.fovy = cam->camera->GetFovX();
	
	if (cam->camera->HasVerticalFov())
		cd.fovy = cam->camera->GetFovY();
	
	cd.znear = cam->camera->GetNearZ();
	cd.zfar = cam->camera->GetFarZ();
	
	cd.ortho = cam->camera->GetProjectionType() == FCDCamera::ORTHOGRAPHIC;
	fwrite(&cd, sizeof(SceneFile::CameraDef), 1, sceneFP);
}
	
void ColladaDocument::WriteLight(ColladaLight * light, int32 i)
{
	DAVA::Logger::FrameworkDebug("write light %i\n", i);
	SceneFile::LightDef ldef;
	
	memset(&ldef, 0, sizeof(ldef));
	
	ldef.ambient.Set(0.f,0.f,0.f,1.0f);
	ldef.diffuse.Set(light->color.x, light->color.y, light->color.z, 1.f);
	ldef.specular.Set(1.f, 1.f, 1.f, 1.f);
	
	ldef.constanAttenuation = light->light->GetConstantAttenuationFactor();
	ldef.linearAttenuation = light->light->GetLinearAttenuationFactor();
	ldef.quadraticAttenuation = light->light->GetQuadraticAttenuationFactor();
	
	ldef.spotCutOff = light->light->GetFallOffAngle();
	ldef.spotExponent = light->light->GetFallOffExponent();
	
	switch (light->light->GetType()) {
		case FCDLight::AMBIENT:
			ldef.type = SceneFile::LightDef::AMBIENT;
			ldef.ambient.Set(light->color.x, light->color.y, light->color.z, 1.f);
			ldef.diffuse = ldef.ambient;
			break;
			
		case FCDLight::DIRECTIONAL:
			ldef.type = SceneFile::LightDef::DIRECTIONAL;
			ldef.position.Set(0, 0, 1.f, 0);
			break;

		case FCDLight::POINT:
			ldef.type = SceneFile::LightDef::POINT;
			ldef.position.Set(0, 0, 0, 1.f);
			ldef.spotCutOff = 180.0f;
			break;
			
		case FCDLight::SPOT:
			ldef.type = SceneFile::LightDef::SPOT;
			ldef.position.Set(0, 0, 0.f, 1.f);
			ldef.spotDirection.Set(0, 0, -1.f, 0);
			break;
            
        default:
            break;
	}
	
	
	fwrite(&ldef, sizeof(SceneFile::LightDef), 1, sceneFP);
}


};