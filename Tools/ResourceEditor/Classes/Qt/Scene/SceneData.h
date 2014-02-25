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



#ifndef __SCENE_DATA_H__
#define __SCENE_DATA_H__

#include "DAVAEngine.h"
#include "../SceneEditor/CameraController.h"
#include "../../ParticlesEditorQT/Helpers/ParticlesEditorSceneDataHelper.h"
#include <QObject>

class EditorScene;
class LandscapesController;
class EditorLandscape;

class SceneData: public QObject
{
    friend class SceneDataManager;
    
    Q_OBJECT
    
    
public:
    SceneData();
    virtual ~SceneData();

	// Rebuild the scene graph for particular node.
	void RebuildSceneGraphNode(DAVA::Entity* node);
	
	// Rebuild the whole scene graph.
    void RebuildSceneGraph();

    void SetScene(EditorScene *newScene);
    EditorScene * GetScene();
    
    void AddSceneNode(DAVA::Entity *node);
    void RemoveSceneNode(DAVA::Entity *node);

    void SelectNode(DAVA::Entity *node);
    DAVA::Entity * GetSelectedNode();
    
    void LockAtSelectedNode();
    
    DAVA::CameraController *GetCameraController();
    
    void CreateScene(bool createEditorCameras);

    void SetScenePathname(const DAVA::FilePath &newPathname);
    const DAVA::FilePath & GetScenePathname() const;

    void ToggleNotPassableLandscape();
    
    bool CanSaveScene();
    
    LandscapesController *GetLandscapesController();
	void SetLandscapesControllerScene(EditorScene* scene);
    
	void ResetLandsacpeSelection();

	void RestoreTexture(const DAVA::FilePath &descriptorPathname, DAVA::Texture *texture);

	// Emit the SceneChanged singal.
	void EmitSceneChanged();

	void GetAllParticleEffects(DAVA::List<DAVA::Entity*> & particleEffects);

signals:
	void SceneChanged(EditorScene *scene);
	void SceneNodeSelected(DAVA::Entity *node);
	
	// Signals are specific for Scene Graph Model.
	void SceneGraphModelNeedsRebuildNode(DAVA::Entity *node);
	void SceneGraphModelNeedsRebuild();
	
	void SceneGraphModelNeedSetScene(EditorScene* scene);
	void SceneGraphModelNeedsSelectNode(DAVA::Entity* node);

protected:
    
    void BakeNode(DAVA::Entity *node);
    void FindIdentityNodes(DAVA::Entity *node);
    void RemoveIdentityNodes(DAVA::Entity *node);
    
    void ReloadNode(DAVA::Entity *node, const DAVA::FilePath &nodePathname);

    void ReleaseScene();

	void FindAllParticleEffectsRecursive(DAVA::Entity *node , DAVA::List<DAVA::Entity*> & particleEffects);

protected slots:
    void SceneNodeSelectedInGraph(DAVA::Entity *node);

protected:
    EditorScene *scene;

	// Node currently selected.
	DAVA::Entity* selectedNode;
	
	// Controllers related to SceneData.
    DAVA::WASDCameraController *cameraController;
    LandscapesController *landscapesController;
	
    DAVA::FilePath sceneFilePathname;
    
	// Particles Editor Scene Data Helper.
	DAVA::ParticlesEditorSceneDataHelper particlesEditorSceneDataHelper;
};

#endif // __SCENE_DATA_H__
