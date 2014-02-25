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


#ifndef __SCENE_DATA_MANAGER_H__
#define __SCENE_DATA_MANAGER_H__

#include "DAVAEngine.h"
#include "EditorScene.h"
#include "Scene/SceneData.h"

class SceneDataManager: public QObject, public DAVA::Singleton<SceneDataManager>
{
	Q_OBJECT

public:
    SceneDataManager();
    virtual ~SceneDataManager();

	// TODO:
	// this part need refactor -->
    void SetActiveScene(EditorScene *scene);

	EditorScene * RegisterNewScene();
    void ReleaseScene(EditorScene *scene);

 	// <--

public:
	// Create the new scene.
	SceneData* CreateNewScene();

	DAVA::Entity*	SceneGetSelectedNode(SceneData *scene);
	DAVA::Entity*	SceneGetRootNode(SceneData *scene);
	SceneData*			SceneGetActive();
	SceneData*			SceneGetLevel();
	SceneData*			SceneGet(DAVA::int32 index);
	SceneData*			SceneGet(DAVA::Scene *scene);
	DAVA::int32			SceneCount();
    
	//void				TextureCompressAllNotCompressed();
	//void				TextureReloadAll(DAVA::eGPUFamily forGPU);
	//DAVA::Texture*		TextureReload(const TextureDescriptor *descriptor, DAVA::Texture *prevTexture, DAVA::eGPUFamily forGPU);

	//static void EnumerateTextures(DAVA::Entity *forNode, DAVA::Map<DAVA::String, DAVA::Texture *> &textures);
	//static void EnumerateDescriptors(DAVA::Entity *forNode, DAVA::Set<DAVA::FilePath> &descriptors);
	//static void EnumerateMaterials(DAVA::Entity *forNode, Vector<Material *> &materials);

	// These methods are called by Scene Graph Tree View.
	void SceneNodeSelectedInSceneGraph(Entity* node);

	// Refresh the information regarding the particular Particles Editor nods.
	void RefreshParticlesLayer(DAVA::ParticleLayer* layer);

signals:
	void SceneCreated(SceneData *scene);
	void SceneActivated(SceneData *scene);
	void SceneDeactivated(SceneData *scene);
	void SceneChanged(SceneData *scene);
	void SceneReleased(SceneData *scene);
	void SceneNodeSelected(SceneData *scene, DAVA::Entity *node);
	
	// Signals needed for Scene Graph Tree View.
	void SceneGraphNeedRebuildNode(DAVA::Entity* node);
	void SceneGraphNeedRebuild();
	
	// Signals related to Particles Editor.
	void SceneGraphNeedRefreshLayer(DAVA::ParticleLayer* layer);

	void SceneGraphNeedSetScene(SceneData *sceneData, EditorScene *scene);
	void SceneGraphNeedSelectNode(SceneData *sceneData, DAVA::Entity* node);

protected slots:
	void InSceneData_SceneChanged(EditorScene *scene);
	void InSceneData_SceneNodeSelected(DAVA::Entity *node);

	// Rebuild the Scene Graph for particular node and for the whole graph.
	void InSceneData_SceneGraphModelNeedsRebuildNode(DAVA::Entity *node);
	void InSceneData_SceneGraphModelNeedsRebuild();
	
	void InSceneData_SceneGraphModelNeedSetScene(EditorScene* scene);
	void InSceneData_SceneGraphModelNeedsSelectNode(DAVA::Entity* node);

protected:

    SceneData * FindDataForScene(EditorScene *scene);
    
	static void CollectLandscapeTextures(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, DAVA::Landscape *forNode);
	static void CollectTexture(DAVA::Map<DAVA::String, DAVA::Texture *> &textures, const DAVA::String &name, DAVA::Texture *tex);

	static void CollectLandscapeDescriptors(DAVA::Set<DAVA::FilePath> &descriptors, DAVA::Landscape *forNode);
	static void CollectDescriptors(DAVA::Set<DAVA::FilePath> &descriptors, const DAVA::FilePath &pathname);

	void RestoreTexture(const DAVA::FilePath &descriptorPathname, DAVA::Texture *texture);
	void CompressTextures(const List<Texture *> texturesForCompression, DAVA::eGPUFamily forGPU);

	// Update the Particle Editor sprites.
	void UpdateParticleSprites();

	// Apply the default fog settings for the new entity.
	void ApplyDefaultFogSettings(Landscape* landscape, DAVA::Entity *entity);
    
protected:
    SceneData *currentScene;
    DAVA::List<SceneData *>scenes;
	
	// This structure is needed to reload scene.
    struct AddedNode
    {
        DAVA::Entity *nodeToAdd;
        DAVA::Entity *nodeToRemove;
        DAVA::Entity *parent;
    };
	
    DAVA::Vector<AddedNode> nodesToAdd;
};

#endif // __SCENE_DATA_MANAGER_H__
