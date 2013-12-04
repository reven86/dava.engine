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



#include "CreateNodesDialog.h"
#include "ControlsFactory.h"

#include "NodesPropertyControl.h"
#include "PropertyControlCreator.h"
#include "../EditorScene.h"
#include "SceneEditorScreenMain.h"
#include "../AppScreens.h"
#include "EditorBodyControl.h"
#include "Scene3D/Components/UserComponent.h"
#include "EditorSettings.h"

#include "Qt/Scene/SceneDataManager.h"
#include "Qt/Scene/SceneData.h"

#include "Scene3D/Components/CustomPropertiesComponent.h"
#include "../StringConstants.h"

CreateNodesDialog::CreateNodesDialog(const Rect & rect)
    :   DraggableDialog(rect)
{
    ControlsFactory::CustomizeDialog(this);

    sceneNode = NULL;
    scene = NULL;
    
    dialogDelegate = NULL;
	propertyList = 0;
    
    header = new UIStaticText(Rect(0, 0, rect.dx, ControlsFactory::BUTTON_HEIGHT));
    header->SetFont(ControlsFactory::GetFont12());
	header->SetTextColor(ControlsFactory::GetColorLight());
    header->SetAlign(ALIGN_HCENTER | ALIGN_VCENTER);
    AddControl(header);
    
    float32 buttonY = rect.dy - ControlsFactory::BUTTON_HEIGHT;
    float32 buttonX = (rect.dx - ControlsFactory::BUTTON_WIDTH * 2) / 2;
    
    UIButton *btnCancel = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), LocalizedString(L"dialog.cancel"));
    btnCancel->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodesDialog::OnCancel));
    AddControl(btnCancel);
    
    buttonX += ControlsFactory::BUTTON_WIDTH;
    UIButton *btnOk = ControlsFactory::CreateButton(Vector2(buttonX, buttonY), LocalizedString(L"dialog.ok"));
    btnOk->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &CreateNodesDialog::OnOk));
    AddControl(btnOk);
    
    propertyRect = Rect(0, ControlsFactory::BUTTON_HEIGHT, rect.dx, buttonY - ControlsFactory::BUTTON_HEIGHT);

    SafeRelease(btnCancel);
    SafeRelease(btnOk);
}
    
CreateNodesDialog::~CreateNodesDialog()
{
    SafeRelease(sceneNode);

    SafeRelease(header);
    SafeRelease(propertyList);
    dialogDelegate = NULL;
}

void CreateNodesDialog::SetDelegate(CreateNodesDialogDelegeate *delegate)
{
    dialogDelegate = delegate;
}

void CreateNodesDialog::OnCancel(BaseObject *, void *, void *)
{
    if(dialogDelegate)
    {
        dialogDelegate->DialogClosed(RCODE_CANCEL);
    }
}

void CreateNodesDialog::OnOk(BaseObject *, void *, void *)
{    
    if(dialogDelegate)
    {
        dialogDelegate->DialogClosed(RCODE_OK);
    }
}

Entity * CreateNodesDialog::GetSceneNode()
{
    return sceneNode;
}


void CreateNodesDialog::SetScene(Scene *_scene)
{
    scene = _scene;
    propertyList->SetWorkingScene(scene);
}

void CreateNodesDialog::SetHeader(const WideString &headerText)
{
    header->SetText(headerText);
}

void CreateNodesDialog::CreateNode(ResourceEditor::eNodeType nodeType)
{
    SafeRelease(sceneNode);

    SceneData *activeScene = SceneDataManager::Instance()->SceneGetActive();
    EditorScene * editorScene = activeScene->GetScene();
	scene = editorScene;

    switch (nodeType) 
    {
        case ResourceEditor::NODE_LANDSCAPE:
            SetHeader(LocalizedString(ResourceEditor::CREATE_NODE_LANDSCAPE));
            sceneNode = new Entity();
            sceneNode->AddComponent(ScopedPtr<RenderComponent> (new RenderComponent(ScopedPtr<Landscape>(new Landscape()))));
            sceneNode->SetName(ResourceEditor::LANDSCAPE_NODE_NAME);
            break;

        case ResourceEditor::NODE_LIGHT:
        {
            SetHeader(LocalizedString(ResourceEditor::CREATE_NODE_LIGHT));
            
            //sceneNode = //EditorLightNode::CreateSceneAndEditorLight();
            sceneNode = new Entity();
            sceneNode->AddComponent(ScopedPtr<LightComponent> (new LightComponent(ScopedPtr<Light>(new Light))));
            sceneNode->SetName(ResourceEditor::LIGHT_NODE_NAME);
            break;
        }

        case ResourceEditor::NODE_SERVICE_NODE:
        {
            SetHeader(LocalizedString(ResourceEditor::CREATE_NODE_SERVICE));
            sceneNode = new Entity();
            KeyedArchive *customProperties = sceneNode->GetCustomProperties();
            customProperties->SetBool(ResourceEditor::EDITOR_IS_LOCKED, true);
            sceneNode->SetName(ResourceEditor::SERVICE_NODE_NAME);
            break;
        }

        case ResourceEditor::NODE_CAMERA:
        {
            SetHeader(LocalizedString(ResourceEditor::CREATE_NODE_CAMERA));
            sceneNode = new Entity();
            
            Camera * camera = new Camera();
            camera->SetUp(Vector3(0.0f, 0.0f, 1.0f));
            sceneNode->AddComponent(ScopedPtr<CameraComponent> (new CameraComponent(camera)));
            sceneNode->SetName(ResourceEditor::CAMERA_NODE_NAME);
            SafeRelease(camera);
        }break;

		case ResourceEditor::NODE_IMPOSTER:
			SetHeader(LocalizedString(ResourceEditor::CREATE_NODE_IMPOSTER));
			sceneNode = new ImposterNode();
			sceneNode->SetName(ResourceEditor::IMPOSTER_NODE_NAME);
			break;

		case ResourceEditor::NODE_PARTICLE_EMITTER:
		{
			SetHeader(LocalizedString(ResourceEditor::CREATE_NODE_PARTICLE_EMITTER));
			sceneNode = new Entity();
			sceneNode->SetName(ResourceEditor::PARTICLE_EMITTER_NODE_NAME);

			ParticleEmitter3D* newEmitter = new ParticleEmitter3D();

			RenderComponent * renderComponent = new RenderComponent();
			renderComponent->SetRenderObject(newEmitter);
			sceneNode->AddComponent(renderComponent);
            
            newEmitter->Release();
			renderComponent->Release();

			break;
		}

		case ResourceEditor::NODE_USER_NODE:
        {
			SetHeader(LocalizedString(ResourceEditor::CREATE_NODE_USER));
			sceneNode = new Entity();
			sceneNode->SetName(ResourceEditor::USER_NODE_NAME);
			sceneNode->AddComponent(ScopedPtr<UserComponent> (new UserComponent()));
			break;
        }

		case ResourceEditor::NODE_SWITCH_NODE:
		{
			SetHeader(LocalizedString(ResourceEditor::CREATE_NODE_SWITCH));
            sceneNode = new Entity();
			sceneNode->SetName(ResourceEditor::SWITCH_NODE_NAME);
            sceneNode->AddComponent(ScopedPtr<SwitchComponent> (new SwitchComponent()));
            
			KeyedArchive *customProperties = sceneNode->GetCustomProperties();
			customProperties->SetBool(Entity::SCENE_NODE_IS_SOLID_PROPERTY_NAME, false);
		}
			break;


		case ResourceEditor::NODE_PARTICLE_EFFECT:
		{
			SetHeader(ResourceEditor::CREATE_NODE_PARTICLE_EFFECT);

			sceneNode = new Entity();
			sceneNode->AddComponent(ScopedPtr<ParticleEffectComponent> (new ParticleEffectComponent()));
			sceneNode->SetName(ResourceEditor::PARTICLE_EFFECT_NODE_NAME);

			break;
		}

		case ResourceEditor::NODE_SKYBOX:
		{
			SetHeader(L"SkyBox");
			
			//TODO: add skybox creation code here
			break;
		}

        default:
            break;
    }

	propertyList = PropertyControlCreator::Instance()->CreateControlForNode(sceneNode, propertyRect, true);
    SafeRetain(propertyList);
	AddControl(propertyList);

	SetScene(editorScene);
    
    propertyList->ReadFrom(sceneNode);
}

void CreateNodesDialog::WillDisappear()
{
	RemoveControl(propertyList);
	SafeRelease(propertyList);
}

