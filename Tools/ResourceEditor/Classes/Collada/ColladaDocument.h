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


#ifndef __COLLADALOADER_COLLADADOCUMENT_H__
#define __COLLADALOADER_COLLADADOCUMENT_H__

#include "ColladaIncludes.h"
#include "ColladaMesh.h"
#include "ColladaScene.h"
#include "ColladaErrorCodes.h"

#include "DAVAEngine.h"


namespace DAVA
{

class ColladaDocument
{
public:
    
	eColladaErrorCodes	Open(const char * filename);
	bool	ExportAnimations(const char * filename);
	bool	ExportNodeAnimations(FCDocument * exportDoc, FCDSceneNode * exportNode);
	
	void	Close();
	
	void	Render();
	void	LoadTextures();
	
	bool	IsEmptyNode(ColladaSceneNode * node);
	
    SceneFileV2::eError SaveSC2( const FilePath & scenePath, const String & sceneName );
	void	SaveScene(const FilePath & scenePath, const String & sceneName);
    String  GetTextureName(const FilePath & scenePath, ColladaTexture * texture);

	void	WriteTexture(SceneFile::TextureDef * texture);
	void	WriteMaterial(SceneFile::MaterialDef * material);
	void	WriteLight(SceneFile::LightDef * light);
	
	//StaticMesh * ConvertMesh(ColladaMesh * mesh);
	void	WriteStaticMesh(ColladaMesh * mesh, int32 meshIndex);
	void	WriteAnimatedMesh(ColladaAnimatedMesh * mesh, int32 animatedMeshIndex);
	
	void	WriteSceneNode(ColladaSceneNode * node, int32 & nodeId, int32 parentId, int32 level);
	void	WriteMeshNode(ColladaMeshInstance * node, int32 & nodeId, int32 parentId, int32 level, int32 instanceIdx);
	void	WriteCameraNode(ColladaCamera * node, int32 & globalNodeId, int32 parentId, int32 level);
	
	void	WriteCamera(ColladaCamera * cam, int32 i);
	void	WriteLight(ColladaLight * light, int32 i);
	
	void	WriteNodeAnimationList(ColladaAnimation * animation);
	
	void	GetAnimationTimeInfo(FCDocument * document, float32 & timeStart, float32 & timeEnd);
    

	FILE	* sceneFP;
	ColladaScene *				colladaScene;

private:
	FCDocument *				document; 
    SceneFile::Header           header;
};
};

#endif 