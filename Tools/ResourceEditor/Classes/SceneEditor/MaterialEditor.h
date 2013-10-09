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




#ifndef MATERIAL_EDITOR
#define MATERIAL_EDITOR

#include "DAVAEngine.h"
#include "DraggableDialog.h"
#include "MaterialPropertyControl.h"
#include "FogControl.h"
#include "ColorControl.h"

using namespace DAVA;

class ComboBox;
class MaterialEditor: public DraggableDialog, public UIListDelegate, public NodesPropertyDelegate, public FogControlDelegate, public ColorControlDelegate
{
public:
    
    MaterialEditor(const Rect &rect = Rect(GetScreenWidth()/8, GetScreenHeight()/8, GetScreenWidth()/4*3, GetScreenHeight()/4*3));
    virtual ~MaterialEditor();
    
    void SetWorkingScene(Scene *newWorkingScene, Entity *newWorkingSceneNode);
    void EditMaterial(Scene *newWorkingScene, Material *newWorkingMaterial);
    
    void OnButton(BaseObject * object, void * userData, void * callerData);
    
    virtual void WillAppear();
    virtual void DidAppear();
    virtual void WillDisappear();
    virtual void UpdateInternalMaterialsVector();
    virtual void UpdateNodeMaterialsVector();

    virtual int32 ElementsCount(UIList *forList);
	virtual UIListCell *CellAtIndex(UIList *forList, int32 index);
	virtual int32 CellWidth(UIList *forList, int32 index)//calls only for horizontal orientation
	{return 20;};
	virtual int32 CellHeight(UIList *forList, int32 index);//calls only for vertical orientation
	virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);
    
    void SelectMaterial(int materialIndex);
    void PreparePropertiesForMaterialType(int materialType);

    //NodesPropertyDelegate
    virtual void NodesPropertyChanged(const String &forKey);
    
    //Fog control delegate
    virtual void SetupFog(bool enabled, float32 dencity, const Color &newColor);
	virtual void SetupColor(const Color &ambient, const Color &diffuse, const Color &specular);
    
    virtual void SetSize(const Vector2 &newSize);
    
protected:
    
    Vector<Material*> materials;
    UIList *materialsList;

    MaterialPropertyControl *materialProps;
    
    Scene *workingScene;
    Entity *workingSceneNode;
    Material *workingMaterial;
    Vector<Material*> workingNodeMaterials;
    
    int selectedMaterial;
    UIListCell *lastSelection;
    
    UIStaticText *noMaterials;
    
    //===============
    enum eDisplayMode
    {
        EDM_ALL = 0,
        EDM_SELECTED
    };
    eDisplayMode displayMode;
    
    UIButton *btnAll;
    UIButton *btnSelected;
    
    void OnAllPressed(BaseObject * object, void * userData, void * callerData);
    void OnSelectedPressed(BaseObject * object, void * userData, void * callerData);
    
    void UdpateButtons(bool showButtons);
    void RefreshList();
    Material *GetMaterial(int32 index);
    
    void OnSetupFog(BaseObject * object, void * userData, void * callerData);
    FogControl *fogControl;

	void OnSetupColor(BaseObject * object, void * userData, void * callerData);
	ColorControl *colorControl;
	
    UIButton *btnSetupFog;
	UIButton *btnSetupColor;

    UIControl *line;
};

#endif