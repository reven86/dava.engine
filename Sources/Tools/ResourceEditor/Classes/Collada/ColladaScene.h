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


#ifndef __COLLADALOADER_COLLADASCENE_H__
#define __COLLADALOADER_COLLADASCENE_H__

#include "ColladaIncludes.h"
#include "ColladaSceneNode.h"
#include "ColladaMesh.h"
#include "ColladaLight.h"
#include "ColladaMaterial.h"
#include "ColladaMeshInstance.h"
#include "ColladaAnimatedMesh.h"
#include "ColladaCamera.h"
#include "ColladaAnimation.h"

namespace DAVA
{

Matrix4 ConvertMatrix(FMMatrix44 & matrix);



class ColladaScene
{
public:	
	ColladaScene(FCDSceneNode * rootNode);
	~ColladaScene();

	void	ExportAnimations(ColladaAnimation * anim, FCDSceneNode * currentNode, float32 anStart, float32 anEnd);
	void	ExportScene(FCDSceneNode * fcdNode = 0,  ColladaSceneNode * node = 0);
	void	Render();
	
	void	RenderAxes();
	void	RenderGrid();

	void	SetupDefaultLights();

	ColladaMeshInstance * CreateMeshInstance(ColladaMesh * mesh, FCDGeometryInstance  * geometryInstance, bool animated);

	ColladaMesh * FindMeshWithName(const fm::string & name);
	ColladaTexture * FindTextureWithName(const fm::string & name); 
	ColladaMaterial * FindMaterialWithName(const fm::string & name); 
	ColladaLight * FindLightWithName(const fm::string & name); 
	ColladaAnimatedMesh * FindAnimatedMeshWithName(const fm::string & name); 
	
	int	FindMaterialIndex(ColladaMaterial * material);
	int	FindMeshIndex(ColladaMesh * mesh);
	bool FindPolyGroupIndex(ColladaPolygonGroup * group, int & meshIndex, int & polygroupIndex);
	
	ColladaCamera * FindCameraWithName( const fm::string & name );
	int	FindCameraIndex(ColladaCamera * cam);

	std::vector<ColladaMesh*> colladaMeshes;
	std::vector<ColladaLight*> colladaLights;
	std::vector<ColladaMaterial*> colladaMaterials;
	std::vector<ColladaTexture*> colladaTextures;
	std::vector<ColladaAnimatedMesh*> colladaAnimatedMeshes;

	std::vector<ColladaLight*> colladaActiveSceneLights;
	std::vector<ColladaCamera*> colladaCameras;
	
	std::vector<ColladaAnimation*> colladaAnimations;
	
	void SetExclusiveAnimation(int32 index);
	
	
	float32 animationStartTime;
	float32 animationEndTime;
	
	int					exportSceneLevel;
	ColladaSceneNode *	rootNode;
	FCDSceneNode *		rootFCDNode;
	float				currentTime;
};

};

#endif // __COLLADALOADER_COLLADASCENE_H__


