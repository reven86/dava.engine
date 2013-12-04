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



#ifndef __SCENE_EDITOR_SCREEN_MAIN_H__
#define __SCENE_EDITOR_SCREEN_MAIN_H__

#include "DAVAEngine.h"

#include "CreateNodesDialog.h"

#include "SettingsDialog.h"

#include "../Constants.h"



using namespace DAVA;

class EditorBodyControl;
class MaterialEditor;
class SettingsDialog;
class TextureTrianglesDialog;

class SceneEditorScreenMain: 
    public UIScreen,
    public CreateNodesDialogDelegeate,
    public SettingsDialogDelegate
{

    static const int32 LINE_HEIGHT = 1;
    static const int32 BODY_Y_OFFSET = 50;
    static const int32 TAB_BUTTONS_OFFSET = 250;

    enum DIALOG_OPERATION
    {
        DIALOG_OPERATION_NONE = 0,
        DIALOG_OPERATION_MENU_OPEN,
        DIALOG_OPERATION_MENU_SAVE,
        DIALOG_OPERATION_MENU_PROJECT,
        DIALOG_OPERATION_MENU_EXPORT,
        DIALOG_OPERATION_MENU_EXPORT_TO_FOLDER,
    };
    
    enum eMenuIDS
    {
        MENUID_OPEN = 100,
        MENUID_CREATENODE = 200,
        MENUID_VIEWPORT = 300,
        MENUID_EXPORTTOGAME = 400,
    };
    
    
    enum eOpenMenuIDS
    {
        EOMID_OPEN = 0,
        EOMID_OPENLAST_STARTINDEX,
        
        EOMID_COUNT
    };

public:
    enum eLandscapeEditorModeIDS
    {
        ELEMID_HEIGHTMAP = 0,
        ELEMID_COLOR_MAP,
        ELEMID_CUSTOM_COLORS,
		ELEMID_VISIBILITY_CHECK_TOOL,
        
        ELEMID_COUNT
    };
    

public:
    
	SceneEditorScreenMain();
	~SceneEditorScreenMain();

    struct BodyItem;


	virtual void LoadResources();
	virtual void UnloadResources();
	virtual void WillAppear();
	virtual void DidAppear();
	virtual void WillDisappear();
	
	virtual void Update(float32 timeElapsed);
	virtual void Draw(const UIGeometricData &geometricData);

    virtual void SetSize(const Vector2 &newSize);

    // create node dialog
    virtual void DialogClosed(int32 retCode);

    void EditMaterial(Material *material);

    void ShowTextureTriangles(PolygonGroup *polygonGroup);

	BodyItem * FindCurrentBody();
    
    virtual void SettingsChanged();
    virtual void Input(UIEvent * event);

	struct BodyItem
	{
		UIButton *headerButton;
		UIButton *closeButton;
		EditorBodyControl *bodyControl;
	};
    
    void RecreteFullTilingTexture();

    void SelectNodeQt(Entity *node);
    void OnReloadRootNodesQt();
    
    bool LandscapeEditorModeEnabled();
    bool TileMaskEditorEnabled();
    
    void AddBodyItem(const WideString &text, bool isCloseable);

    void UpdateModificationPanel(void);

    void ProcessIsSolidChanging();

	void ActivateLevelBodyItem();
    
private:
    
    void InitControls();
    
    void AddLineControl(Rect r);
    
    Vector<BodyItem *> bodies;
    
    void InitializeBodyList();
    void ReleaseBodyList();
    
    void OnSelectBody(BaseObject * owner, void * userData, void * callerData);
    void OnCloseBody(BaseObject * owner, void * userData, void * callerData);
	void ActivateBodyItem(BodyItem* activeItem, bool forceResetSelection);

    //create node dialog
    CreateNodesDialog *nodeDialog;
    
    MaterialEditor *materialEditor;

    void InitializeNodeDialogs();
    void ReleaseNodeDialogs();
    
    UIControl *dialogBack;

    SettingsDialog *settingsDialog;
    
    TextureTrianglesDialog *textureTrianglesDialog;
    
    // general
    Font *font12;
	Color font12Color;
    
	bool initialized;
	bool useConvertedTextures;
    
    void ReleaseResizedControl(UIControl *control);

public: //For Qt integration
    void NewScene();

    bool SaveIsAvailable();
   

    void CreateNode(ResourceEditor::eNodeType nodeType);
    void SetViewport(ResourceEditor::eViewportType viewportType);
    
    void MaterialsTriggered();
    void HeightmapTriggered();
    void TilemapTriggered();
    void RulerToolTriggered();
	
	//custom colors
    void CustomColorsTriggered();
	void CustomColorsSetRadius(uint32 newRadius);
	void CustomColorsSetColor(uint32 indexInSet);
	void CustomColorsSaveTexture(const FilePath & path);
	void CustomColorsLoadTexture(const FilePath & path);
	FilePath CustomColorsGetCurrentSaveFileName();
	
	//visibility check tool
	void VisibilityToolTriggered();
	void VisibilityToolSaveTexture(const FilePath& path);
	void VisibilityToolSetPoint();
	void VisibilityToolSetArea();
	void VisibilityToolSetAreaSize(uint32 size);
    
    void ShowSettings();
    
    void ProcessBeast();
    
    UIControl *focusedControl;
};

#endif // __SCENE_EDITOR_SCREEN_MAIN_H__
