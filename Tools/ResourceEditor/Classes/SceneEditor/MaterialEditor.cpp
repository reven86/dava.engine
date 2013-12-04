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





#include "MaterialEditor.h"
#include "ControlsFactory.h"
#include "SceneValidator.h"

#include "MaterialPropertyControl.h"
#include "EditorSettings.h"
#include "../EditorScene.h"

#include "../Qt/CubemapEditor/MaterialHelper.h"
#include "Classes/Qt/Scene/SceneEditor2.h"

static const float32 materialListPart = 0.33f;
static const float32 previewHeightPart = 0.5f;


MaterialEditor::MaterialEditor(const Rect &rect)
: DraggableDialog(rect)
{//todo: create draggable dealog
    
    ControlsFactory::CustomizeDialog(this);
    displayMode = EDM_ALL;
    
    workingMaterial = NULL;
    workingSceneNode = NULL;
    workingScene = NULL;
    float32 materialListWidth = size.x * materialListPart;
    
    btnAll = ControlsFactory::CreateButton(Rect(0, ControlsFactory::BUTTON_HEIGHT, materialListWidth/2, ControlsFactory::BUTTON_HEIGHT), 
                                           LocalizedString(L"materialeditor.all"));
    btnAll->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &MaterialEditor::OnAllPressed));
    
    btnSelected = ControlsFactory::CreateButton(Rect(materialListWidth/2, ControlsFactory::BUTTON_HEIGHT, 
                                                     materialListWidth/2, ControlsFactory::BUTTON_HEIGHT), 
                                                LocalizedString(L"materialeditor.selected"));
    btnSelected->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &MaterialEditor::OnSelectedPressed));


    Rect setupFogRect(GetRect().dx - ControlsFactory::BUTTON_WIDTH, ControlsFactory::BUTTON_HEIGHT, ControlsFactory::BUTTON_WIDTH, ControlsFactory::BUTTON_HEIGHT);
    btnSetupFog = ControlsFactory::CreateButton(setupFogRect, LocalizedString(L"materialeditor.setupfog"));
    btnSetupFog->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &MaterialEditor::OnSetupFog));
    AddControl(btnSetupFog);

	Rect setupColorRect(setupFogRect);
	setupColorRect.x -= ControlsFactory::BUTTON_WIDTH;
	btnSetupColor = ControlsFactory::CreateButton(setupColorRect, L"Setup Color");
	btnSetupColor->AddEvent(UIControl::EVENT_TOUCH_UP_INSIDE, Message(this, &MaterialEditor::OnSetupColor));
	AddControl(btnSetupColor);



    line = ControlsFactory::CreateLine(Rect(GetRect().dx - ControlsFactory::BUTTON_WIDTH*3, ControlsFactory::BUTTON_HEIGHT * 2, ControlsFactory::BUTTON_WIDTH*3, 1),
                                                  Color::White());
    AddControl(line);
    
    Rect fogRect(setupFogRect.x - ControlsFactory::BUTTON_WIDTH, setupFogRect.dy + setupFogRect.y, ControlsFactory::BUTTON_WIDTH * 2, ControlsFactory::BUTTON_HEIGHT * 5);
    fogControl = new FogControl(fogRect, this);
    
	Rect colorRect(setupColorRect.x - ControlsFactory::BUTTON_WIDTH, setupColorRect.dy + setupColorRect.y, ControlsFactory::BUTTON_WIDTH * 3, ControlsFactory::BUTTON_HEIGHT * 5);
	colorControl = new ColorControl(colorRect, this);


    materialsList = new UIList(Rect(0, ControlsFactory::BUTTON_HEIGHT * 2, 
                                    materialListWidth, size.y - ControlsFactory::BUTTON_HEIGHT * 2), 
                               UIList::ORIENTATION_VERTICAL);
    materialsList->SetDelegate(this);
    ControlsFactory::SetScrollbar(materialsList);
    ControlsFactory::CusomizeListControl(materialsList);
    AddControl(materialsList);
    UIStaticText *text = new UIStaticText(Rect(0, 0, size.x * materialListPart, ControlsFactory::BUTTON_HEIGHT));
    text->SetFont(ControlsFactory::GetFont12());
    text->SetText(LocalizedString(L"materialeditor.materials"));
	text->SetTextColor(ControlsFactory::GetColorLight());
    AddControl(text);
    SafeRelease(text);
    
    float32 textY = (GetRect().dy - ControlsFactory::BUTTON_HEIGHT ) / 2.f;
    noMaterials = new UIStaticText(Rect(materialListWidth, textY, GetRect().dx - materialListWidth, (float32)ControlsFactory::BUTTON_HEIGHT));
    noMaterials->SetFont(ControlsFactory::GetFont12());
	noMaterials->SetTextColor(ControlsFactory::GetColorLight());
    noMaterials->SetText(LocalizedString(L"materialeditor.nomaterials"));
    
    selectedMaterial = -1;
    lastSelection = NULL;
    Vector<String> v;
    for (int i = 0; i < Material::MATERIAL_TYPES_COUNT; i++) 
    {
        v.push_back(Material::GetTypeName((Material::eType)i));
    }

    materialProps = new MaterialPropertyControl(Rect(size.x * materialListPart, 
                                                      size.y * previewHeightPart, 
                                                      size.x - size.x * materialListPart, 
                                                      size.y - size.y * previewHeightPart),
                                                 false);
    materialProps->SetDelegate(this);
    AddControl(materialProps);
}

MaterialEditor::~MaterialEditor()
{
	for_each(materials.begin(), materials.end(),  SafeRelease<Material>);
	materials.clear();

	for_each(workingNodeMaterials.begin(), workingNodeMaterials.end(),  SafeRelease<Material>);
	workingNodeMaterials.clear();

	SafeRelease(workingMaterial);
	SafeRelease(workingSceneNode);
	SafeRelease(workingScene);

	SafeRelease(btnAll);
	SafeRelease(btnSelected);
	SafeRelease(btnSetupFog);
	SafeRelease(btnSetupColor);
	SafeRelease(line);
	SafeRelease(fogControl);
	SafeRelease(colorControl);
	SafeRelease(materialsList);
	SafeRelease(noMaterials);
	SafeRelease(materialProps);
}

void MaterialEditor::UpdateInternalMaterialsVector()
{
	for_each(materials.begin(), materials.end(),  SafeRelease<Material>);
    materials.clear();
    
    workingScene->GetDataNodes(materials);
	//VI: remove skybox materials so they not to appear in the lists
	MaterialHelper::FilterMaterialsByType(materials, DAVA::Material::MATERIAL_SKYBOX);
    
    for (int32 k = 0; k < (int32)materials.size(); ++k)
    {
        materials[k]->Retain();
    }
}

void MaterialEditor::UpdateNodeMaterialsVector()
{
	for_each(workingNodeMaterials.begin(), workingNodeMaterials.end(),  SafeRelease<Material>);
    workingNodeMaterials.clear();
    if(workingSceneNode)
    {
        workingSceneNode->GetDataNodes(workingNodeMaterials);
		//VI: remove skybox materials so they not to appear in the lists
		MaterialHelper::FilterMaterialsByType(workingNodeMaterials, DAVA::Material::MATERIAL_SKYBOX);

    }
    else if(workingMaterial)
    {
        workingNodeMaterials.push_back(workingMaterial);
    }
    
    for (int32 k = 0; k < (int32)workingNodeMaterials.size(); ++k)
    {
        workingNodeMaterials[k]->Retain();
    }
}


void MaterialEditor::WillAppear()
{
    UIScreen *activeScreen = UIScreenManager::Instance()->GetScreen();
    if(activeScreen)
    {
        float32 height = GetSize().y;
        SetRect(Rect(GetScreenWidth()/8.f, (GetScreenHeight() - height) / 2.f, GetScreenWidth()/4.f*3.f, height));
    }
    
    UpdateInternalMaterialsVector();
    UpdateNodeMaterialsVector();
    
    OnAllPressed(NULL, NULL, NULL);
}

void MaterialEditor::DidAppear()
{
    for (int32 wnm = 0; wnm < (int32)workingNodeMaterials.size(); ++wnm)
    {
        Material *m = workingNodeMaterials[wnm];
        for(int32 i = 0; i < (int32)materials.size(); ++i)
        {
            if(m == materials[i])
            {
                materialsList->ScrollToElement(i);
                break;
            }
        }
    }
}

void MaterialEditor::WillDisappear()
{
	for_each(materials.begin(), materials.end(),  SafeRelease<Material>);
    materials.clear();
	for_each(workingNodeMaterials.begin(), workingNodeMaterials.end(),  SafeRelease<Material>);
    workingNodeMaterials.clear();
    
    SelectMaterial(-1);
    
    if (lastSelection) 
    {
        lastSelection->SetSelected(false, false);
        lastSelection = NULL;
    }
}


void MaterialEditor::EditMaterial(Scene *newWorkingScene, Material *newWorkingMaterial)
{
    if ((newWorkingScene == workingScene) && (workingMaterial == newWorkingMaterial))
    {
        return;
    }

    SafeRelease(workingMaterial);
    SafeRelease(workingSceneNode);
    SafeRelease(workingScene);
    
    workingScene = SafeRetain(newWorkingScene);
    materialProps->SetWorkingScene(workingScene);

    workingMaterial = SafeRetain(newWorkingMaterial);
    
    UdpateButtons(NULL != workingMaterial);

    SelectMaterial(0);
    RefreshList();
}


void MaterialEditor::SetWorkingScene(Scene *newWorkingScene, Entity *newWorkingSceneNode)
{
    if ((newWorkingScene == workingScene) && (workingSceneNode == newWorkingSceneNode))
    {
        return;
    }
    
    SafeRelease(workingMaterial);
    SafeRelease(workingSceneNode);
    SafeRelease(workingScene);

    workingScene = SafeRetain(newWorkingScene);
    materialProps->SetWorkingScene(workingScene);

    workingSceneNode = SafeRetain(newWorkingSceneNode);

    UdpateButtons(NULL != workingSceneNode);
    
    SelectMaterial(0);
    RefreshList();
}

void MaterialEditor::OnButton(BaseObject * , void * , void * )
{
}

void MaterialEditor::SelectMaterial(int materialIndex)
{
    Material *mat = GetMaterial(materialIndex);
    if(mat)
    {
        selectedMaterial = materialIndex;

        if(noMaterials->GetParent())
        {
            RemoveControl(noMaterials);
        }

        PreparePropertiesForMaterialType(mat->type);
    }
    else
    {
        selectedMaterial = -1;
        
        if (materialProps->GetParent())
        {
            RemoveControl(materialProps);
        }

        if(!noMaterials->GetParent())
        {
            AddControl(noMaterials);
        }
    }
}

void MaterialEditor::PreparePropertiesForMaterialType(int materialType)
{
    if(!materialProps->GetParent())
    {
        AddControl(materialProps);
    }
    
    Material *mat = GetMaterial(selectedMaterial);
    if(mat)
    {
        materialProps->ReadFrom(mat);
    }    
}


int32 MaterialEditor::ElementsCount(UIList *)
{
    if(EDM_ALL == displayMode)
    {
        if (workingScene) 
        {
            return (int32)materials.size();
        }
    }
    else
    {
        return workingNodeMaterials.size();
    }
    return 0;
}

UIListCell *MaterialEditor::CellAtIndex(UIList *forList, int32 index)
{
    UIListCell *c = forList->GetReusableCell("Material name cell");
    if (!c) 
    {
        c = new UIListCell(Rect(0, 0, forList->GetRect().dx, 20), "Material name cell");

        float32 boxSize = 16;
        float32 y = (CellHeight(forList, index) - boxSize) / 2;
        float32 x = forList->GetRect().dx - boxSize;
        
        
        //Temporary fix for loading of UI Interface to avoid reloading of texrures to different formates.
        // 1. Reset default format before loading of UI
        // 2. Restore default format after loading of UI from stored settings.
        Texture::SetDefaultGPU(GPU_UNKNOWN);
        
        Rect r = Rect(x, y, boxSize, boxSize);
        UIControl *sceneFlagBox = new UIControl(r);
        sceneFlagBox->SetName("flagBox");
        sceneFlagBox->GetBackground()->SetDrawType(UIControlBackground::DRAW_SCALE_TO_RECT);
        sceneFlagBox->SetSprite("~res:/Gfx/UI/marker", 1);
        sceneFlagBox->SetInputEnabled(false);
        c->AddControl(sceneFlagBox);
        SafeRelease(sceneFlagBox);
        
        Texture::SetDefaultGPU(EditorSettings::Instance()->GetTextureViewGPU());
    }

    Material *mat = GetMaterial(index);
    bool found = false;
    if(EDM_ALL == displayMode)
    {
        for (int32 i = 0; i < (int32)workingNodeMaterials.size(); ++i)
        {
            if(workingNodeMaterials[i] == mat)
            {
                found = true;
                break;
            }
        }
    }
    else
    {
        found = true;
    }
    
    ControlsFactory::CustomizeListCell(c, StringToWString(mat->GetName()), false);
    UIControl *sceneFlagBox = c->FindByName("flagBox");
    sceneFlagBox->SetVisible(found, false);
    
    if (index == selectedMaterial) 
    {
        c->SetSelected(true, false);
        lastSelection = c;
    }
    else
    {
        c->SetSelected(false, false);
    }
    
    return c;
}

int32 MaterialEditor::CellHeight(UIList *, int32 )
{
    return 20;
}

void MaterialEditor::OnCellSelected(UIList *, UIListCell *selectedCell)
{
    if (selectedCell->GetIndex() != selectedMaterial)
    {
        if (lastSelection && lastSelection->GetIndex() == selectedMaterial) 
        {
            lastSelection->SetSelected(false, false);
        }
        selectedCell->SetSelected(true, false);
        lastSelection = selectedCell;
        SelectMaterial(selectedCell->GetIndex());
    }    
}


void MaterialEditor::OnAllPressed(BaseObject * , void * , void * )
{
    displayMode = EDM_ALL;
    btnAll->SetSelected(true, false);
    btnSelected->SetSelected(false, false);

    if (lastSelection) 
    {
        lastSelection->SetSelected(false, false);
        lastSelection = NULL;
    }
    
    SelectMaterial(0);
    RefreshList();
}

void MaterialEditor::OnSelectedPressed(BaseObject * , void * , void * )
{
    displayMode = EDM_SELECTED;

    btnAll->SetSelected(false, false);
    btnSelected->SetSelected(true, false);

    if (lastSelection) 
    {
        lastSelection->SetSelected(false, false);
        lastSelection = NULL;
    }
    
    SelectMaterial(0);
    RefreshList();
}

void MaterialEditor::UdpateButtons(bool showButtons)
{
    displayMode = EDM_ALL;
    if(showButtons)
    {
        if(!btnAll->GetParent()) AddControl(btnAll);
        if(!btnSelected->GetParent()) AddControl(btnSelected);
        
        btnAll->SetSelected(false, false);
        btnSelected->SetSelected(false, false);
    }
    else
    {
        if(btnAll->GetParent()) RemoveControl(btnAll);
        if(btnSelected->GetParent()) RemoveControl(btnSelected);

        btnAll->SetSelected(false, false);
        btnSelected->SetSelected(false, false);
    }
}

void MaterialEditor::RefreshList()
{
    materialsList->Refresh();
    materialsList->ResetScrollPosition();
}

Material * MaterialEditor::GetMaterial(int32 index)
{
    Material *mat = NULL;
    if(EDM_ALL == displayMode)
    {
        if((0 <= index) && (index < (int32)materials.size()))
        {
            mat = materials[index];
        }
    }
    else
    {
        if((0 <= index) && (index < (int32)workingNodeMaterials.size()))
        {
            mat = workingNodeMaterials[index];
        }
    }
    return mat;
}

void MaterialEditor::OnSetupFog(BaseObject *, void *, void *)
{
    if(fogControl)
    {
        AddControl(fogControl);
    }
}

void MaterialEditor::OnSetupColor(BaseObject * object, void * userData, void * callerData)
{
	if(colorControl)
	{
		AddControl(colorControl);
	}
}

void MaterialEditor::NodesPropertyChanged(const String &)
{
	if(workingScene)
	{
		SceneEditor2 *sc = dynamic_cast<SceneEditor2 *>(workingScene);
		if(sc)
		{
			sc->MarkAsChanged();
		}
	}

    RefreshList();
}

void MaterialEditor::SetupFog(bool enabled, float32 dencity, const DAVA::Color &newColor)
{
    for(int32 i = 0; i < (int32)materials.size(); ++i)
    {
        materials[i]->SetFog(enabled);
        materials[i]->SetFogDensity(dencity);
        materials[i]->SetFogColor(newColor);
    }
    
    if(workingScene)
    {
        Landscape *landscape = FindLandscape(workingScene);
        if (landscape)
        {
            landscape->SetFog(enabled);
            landscape->SetFogDensity(dencity);
            landscape->SetFogColor(newColor);
        }
    }
}

void MaterialEditor::SetupColor(const Color &ambient, const Color &diffuse, const Color &specular)
{
	for(int32 i = 0; i < (int32)materials.size(); ++i)
	{
		materials[i]->SetAmbientColor(ambient);
		materials[i]->SetDiffuseColor(diffuse);
		materials[i]->SetSpecularColor(specular);
	}
}


void MaterialEditor::SetSize(const Vector2 &newSize)
{
    DraggableDialog::SetSize(newSize);
    
    btnSetupFog->SetPosition(Vector2(newSize.x - ControlsFactory::BUTTON_WIDTH, ControlsFactory::BUTTON_HEIGHT));
	btnSetupColor->SetPosition(Vector2(newSize.x - ControlsFactory::BUTTON_WIDTH*2, ControlsFactory::BUTTON_HEIGHT));
    line->SetPosition(Vector2(newSize.x - ControlsFactory::BUTTON_WIDTH*3, ControlsFactory::BUTTON_HEIGHT * 3));
    

    Rect setupFogRect(newSize.x - ControlsFactory::BUTTON_WIDTH, ControlsFactory::BUTTON_HEIGHT, ControlsFactory::BUTTON_WIDTH, ControlsFactory::BUTTON_HEIGHT);
    Rect fogRect(setupFogRect.x - ControlsFactory::BUTTON_WIDTH, setupFogRect.dy + setupFogRect.y, ControlsFactory::BUTTON_WIDTH * 2, ControlsFactory::BUTTON_HEIGHT * 5);
    fogControl->SetRect(fogRect);

	Rect setupColorRect(setupFogRect);
	setupColorRect.x -= ControlsFactory::BUTTON_WIDTH;
	Rect colorRect(setupColorRect.x - ControlsFactory::BUTTON_WIDTH, setupColorRect.dy + setupColorRect.y, ControlsFactory::BUTTON_WIDTH * 3, ControlsFactory::BUTTON_HEIGHT * 5);
	colorControl->SetRect(colorRect);
    
    
    float32 materialListWidth = materialsList->GetSize().x;
    Rect noMaterialsRect = noMaterials->GetRect();
    noMaterials->SetRect(Rect(materialListWidth, noMaterialsRect.y, newSize.x - materialListWidth, noMaterialsRect.dy));

    
    Rect propsRect = materialProps->GetRect();

    RemoveControl(materialProps);
    SafeRelease(materialProps);
    
    materialProps = new MaterialPropertyControl(Rect(materialListWidth,
                                                     propsRect.y,
                                                     newSize.x - materialListWidth,
                                                     propsRect.dy),
                                                false);
    materialProps->SetDelegate(this);
    AddControl(materialProps);

    
    SelectMaterial(selectedMaterial);
}
