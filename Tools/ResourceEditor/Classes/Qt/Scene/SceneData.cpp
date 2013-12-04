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



#include "Scene/SceneData.h"
#include "DockSceneGraph/SceneGraphModel.h"
#include "DockSceneGraph/PointerHolder.h"

#include "../EditorScene.h"
#include "../SceneEditor/EditorSettings.h"
#include "../SceneEditor/SceneValidator.h"

#include "../SceneEditor/SceneEditorScreenMain.h"

#include "../Commands/SceneGraphCommands.h"
#include "../Commands/LibraryCommands.h"
#include "../Commands/CommandsManager.h"
#include "../StringConstants.h"

#include "../LandscapeEditor/LandscapesController.h"

#include "Scene3D/Components/CameraComponent.h"

#include "ParticlesEditorQT/Helpers/ParticlesEditorNodeNameHelper.h"

#include "Main/QtMainWindowHandler.h"

#include "Main/QtUtils.h"
#include "DockSceneGraph/PointerHolder.h"

#include "DockLibrary/LibraryModel.h"

#include <QTreeView>
#include <QFileSystemModel>
#include <QDir>
#include <QFileInfo>
#include <QMenu>
#include <QAction>

using namespace DAVA;

SceneData::SceneData()
    :   QObject(NULL)
    ,   scene(NULL)
	,	selectedNode(NULL)
{
    landscapesController = new LandscapesController();
    cameraController = new WASDCameraController(EditorSettings::Instance()->GetCameraSpeed());
}

SceneData::~SceneData()
{
    SafeRelease(scene);
    SafeRelease(landscapesController);
    SafeRelease(cameraController);
}

void SceneData::RebuildSceneGraphNode(DAVA::Entity* node)
{
	emit SceneGraphModelNeedsRebuildNode(node);
}

void SceneData::RebuildSceneGraph()
{
	emit SceneGraphModelNeedsRebuild();
}

EditorScene * SceneData::GetScene()
{
    return scene;
}

void SceneData::AddSceneNode(DAVA::Entity *node)
{
    // Firstly ask Particle Editor to add this node.
    if (particlesEditorSceneDataHelper.AddSceneNode(node) == false)
    {
		String newName = ParticlesEditorNodeNameHelper::GetNewNodeName(node->GetName(), scene);
		node->SetName(newName);
        scene->AddNode(node);
    }
    
    Landscape *landscape = dynamic_cast<Landscape *>(node);
    if(landscape)
    {
        landscapesController->SaveLandscape(landscape);
    }
    
    RebuildSceneGraph();
}

void SceneData::RemoveSceneNode(DAVA::Entity *node)
{
    Entity * parent = node->GetParent();
    if (parent)
    {
        Landscape *landscape = dynamic_cast<Landscape *>(node);
        if(landscape)
        {
            landscapesController->SaveLandscape(NULL);
        }

		particlesEditorSceneDataHelper.RemoveSceneNode(node);
        scene->ReleaseUserData(node);
		SelectNode(NULL);
        scene->SetSelection(NULL);
        
        parent->RemoveNode(node);

//        SceneValidator::Instance()->EnumerateSceneTextures();
    }
    
    RebuildSceneGraph();
}

void SceneData::SelectNode(DAVA::Entity *node)
{
	this->selectedNode = node;
	emit SceneGraphModelNeedsSelectNode(node);
}

void SceneData::SceneNodeSelectedInGraph(Entity *node)
{
    if(scene)   scene->SetSelection(node);
    
    cameraController->SetSelection(node);

    //TODO: remove code at full-qt version
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(screen)
    {
        screen->SelectNodeQt(node);
    }
    //EndOfTODO
    
    
    Camera * cam = GetCamera(node);
    if (cam)
    {
        if (IsKeyModificatorPressed(DVKEY_ALT))
        {
            scene->SetClipCamera(cam);
        }
        else
        {
            scene->SetCurrentCamera(cam);
        }
    }

	this->selectedNode = node;
	emit SceneNodeSelected(node);
}

DAVA::Entity * SceneData::GetSelectedNode()
{
    return this->selectedNode;
}

void SceneData::LockAtSelectedNode()
{
    if(cameraController)
    {
        cameraController->LookAtSelection();
    }
}

CameraController * SceneData::GetCameraController()
{
    return cameraController;
}

void SceneData::SetScene(EditorScene *newScene)
{
    ReleaseScene();
    
    scene = SafeRetain(newScene);
	emit SceneGraphModelNeedSetScene(scene);
    cameraController->SetScene(scene);
    landscapesController->SetScene(scene);
}

void SceneData::ReleaseScene()
{
    cameraController->SetScene(NULL);
	emit SceneGraphModelNeedSetScene(NULL);
    landscapesController->SetScene(NULL);
    
    SafeRelease(scene);
}

void SceneData::CreateScene(bool createEditorCameras)
{
    ReleaseScene();

    cameraController->SetSpeed(EditorSettings::Instance()->GetCameraSpeed());
    
    EditorScene *createdScene = new EditorScene();


    // Camera setup
    if(createEditorCameras)
    {
        Camera * cam = new Camera();
        cam->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        cam->SetPosition(Vector3(0.0f, 0.0f, 0.0f));
        cam->SetTarget(Vector3(0.0f, 1.0f, 0.0f));
        
        cam->SetupPerspective(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f);
        
        ScopedPtr<Entity> node(new Entity());
        node->SetName(ResourceEditor::EDITOR_MAIN_CAMERA);
        node->AddComponent(ScopedPtr<CameraComponent> (new CameraComponent(cam)));
        createdScene->AddNode(node);
        createdScene->AddCamera(cam);
        createdScene->SetCurrentCamera(cam);
        
        SafeRelease(cam);
        
        Camera * cam2 = new Camera();
        cam2->SetUp(Vector3(0.0f, 0.0f, 1.0f));
        cam2->SetPosition(Vector3(0.0f, 0.0f, 200.0f));
        cam2->SetTarget(Vector3(0.0f, 250.0f, 0.0f));
        
        cam2->SetupPerspective(70.0f, 320.0f / 480.0f, 1.0f, 5000.0f);
        
        ScopedPtr<Entity> node2(new Entity());
        node2->SetName(ResourceEditor::EDITOR_DEBUG_CAMERA);
        node2->AddComponent(ScopedPtr<CameraComponent> (new CameraComponent(cam2)));
        createdScene->AddNode(node2);
        createdScene->AddCamera(cam2);
        
        SafeRelease(cam2);
    }
    
    SetScene(createdScene);
    SafeRelease(createdScene);
}

void SceneData::SetScenePathname(const FilePath &newPathname)
{
    sceneFilePathname = newPathname;
    if(scene)
    {
        scene->SetName(sceneFilePathname.GetFilename());
    }
}

const FilePath & SceneData::GetScenePathname() const
{
    return sceneFilePathname;
}

void SceneData::BakeNode(DAVA::Entity *node)
{
    if(node->GetSolid())
    {
        node->BakeTransforms();
        return;
    }
    
    for(int32 i = 0; i < node->GetChildrenCount(); ++i)
    {
        BakeNode(node->GetChild(i));
    }
}

void SceneData::RemoveIdentityNodes(DAVA::Entity *node)
{
    for(int32 i = 0; i < node->GetChildrenCount(); ++i)
    {
        Entity *removedChild = node->GetChild(i);
        
        if(
           (removedChild->GetFlags() & Entity::NODE_LOCAL_MATRIX_IDENTITY)
           &&   (typeid(Entity) == typeid(*removedChild))
           &&   (removedChild->GetChildrenCount() == 1))
        {
            //Entity *child = SafeRetain(removedChild->GetChild(0));
            //removedChild->RemoveNode(child);
            node->AddNode(removedChild->GetChild(0));
            //SafeRelease(child);
            
            node->RemoveNode(removedChild);
            
            i = -1;
        }
        else
        {
            RemoveIdentityNodes(removedChild);
        }
    }
}

void SceneData::FindIdentityNodes(DAVA::Entity *node)
{
    for(int32 i = 0; i < node->GetChildrenCount(); ++i)
    {
        Entity *child = node->GetChild(i);
        
        if(child->GetSolid())
        {
            RemoveIdentityNodes(child);
        }
        else
        {
            FindIdentityNodes(child);
        }
    }
}


void SceneData::ToggleNotPassableLandscape()
{
    UIScreen *screen = UIScreenManager::Instance()->GetScreen();
    if (IsPointerToExactClass<SceneEditorScreenMain>(screen))
    {
        if(((SceneEditorScreenMain *)screen)->TileMaskEditorEnabled())
        {
            return;
        }

    }
    
    landscapesController->ToggleNotPassableLandscape();
    RebuildSceneGraph();
}


bool SceneData::CanSaveScene()
{
    SceneEditorScreenMain *screen = dynamic_cast<SceneEditorScreenMain *>(UIScreenManager::Instance()->GetScreen());
    if(!screen->SaveIsAvailable())
    {
        return false;
    }
    
    if(landscapesController->EditorLandscapeIsActive())
    {
        return false;
    }
    
    
    return true;
}


LandscapesController * SceneData::GetLandscapesController()
{
    return landscapesController;
}

void SceneData::SetLandscapesControllerScene(EditorScene* scene)
{
	if (this->landscapesController)
	{
		this->landscapesController->SetScene(scene);
	}
}

void SceneData::ResetLandsacpeSelection()
{
	Landscape *selectedNode = dynamic_cast<Landscape *>	(GetSelectedNode());
	if(selectedNode)
	{
		SelectNode(NULL);
	}
}


void SceneData::RestoreTexture(const DAVA::FilePath &descriptorPathname, DAVA::Texture *texture)
{
    Vector<Entity *> nodes;
    scene->GetChildNodes(nodes);
    
    for(int32 n = 0; n < (int32)nodes.size(); ++n)
    {
        RenderComponent *rc = static_cast<RenderComponent *>(nodes[n]->GetComponent(Component::RENDER_COMPONENT));
        if(!rc) continue;
        
        RenderObject *ro = rc->GetRenderObject();
        if(!ro) continue;
        
        uint32 count = ro->GetRenderBatchCount();
        for(uint32 b = 0; b < count; ++b)
        {
            RenderBatch *renderBatch = ro->GetRenderBatch(b);
            
            Material *material = renderBatch->GetMaterial();
            if(material)
            {
                for(int32 t = 0; t < Material::TEXTURE_COUNT; ++t)
                {
                    if(material->GetTextureName((Material::eTextureLevel)t) == descriptorPathname)
                    {
                        material->SetTexture((Material::eTextureLevel)t, texture);
                    }
                }
            }
            
            InstanceMaterialState *instanceMaterial = renderBatch->GetMaterialInstance();
            if(instanceMaterial)
            {
                if(instanceMaterial->GetLightmapName() == descriptorPathname)
                {
                    instanceMaterial->SetLightmap(texture, instanceMaterial->GetLightmapName());
                }
            }
        }
        
        Landscape *landscape = dynamic_cast<Landscape *>(ro);
        if(landscape)
        {
            for(int32 t = 0; t < Landscape::TEXTURE_COUNT; ++t)
            {
                if(landscape->GetTextureName((Landscape::eTextureLevel)t) == descriptorPathname)
                {
                    landscape->SetTexture((Landscape::eTextureLevel)t, texture);
                }
            }
        }
    }
}

void SceneData::EmitSceneChanged()
{
	emit SceneChanged(this->scene);
}

void SceneData::GetAllParticleEffects(List<DAVA::Entity*> & particleEffects)
{
	Scene* scene = this->GetScene();
	uint32 childCount = scene->GetChildrenCount();
	for(uint32 i = 0 ; i < childCount; ++i)
	{
		FindAllParticleEffectsRecursive(scene->GetChild(i), particleEffects);
	}
}

void SceneData::FindAllParticleEffectsRecursive(Entity *node , List<DAVA::Entity*> & particleEffects)
{
    ParticleEffectComponent * effectComponent = cast_if_equal<ParticleEffectComponent*>(node->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
	if (effectComponent)
	{
		particleEffects.push_back(node);
	}

	uint32 childCount = node->GetChildrenCount();
	for(uint32 i = 0 ; i < childCount; ++i)
	{
		FindAllParticleEffectsRecursive(node->GetChild(i), particleEffects);
	}
}

