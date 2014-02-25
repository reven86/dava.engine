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



#ifndef __EDITOR_BODY_CONTROL_H__
#define __EDITOR_BODY_CONTROL_H__

#include "DAVAEngine.h"
#include "CameraController.h"
#include "EditMatrixControl.h"
#include "../EditorScene.h"

#include "../Constants.h"
#include "NodesPropertyControl.h"

#include "ModificationsPanel.h"

#include "LandscapeEditorBase.h"

#include "GraphBase.h"



using namespace DAVA;

class RulerTool;
class SceneGraph;
class BeastManager;
class LandscapeEditorColor;
class LandscapeEditorHeightmap;
class LandscapeToolsSelection;
class LandscapeEditorCustomColors;
class LandscapeEditorVisibilityCheckTool;
class Command;
class ArrowsNode;
class EditorBodyControl: 
        public UIControl, 
        public GraphBaseDelegate,
        public LandscapeEditorDelegate,
        public ModificationsPanelDelegate
{
    static const int32 SCENE_OFFSET = 10;
    
    enum ePropertyShowState
    {
        EPSS_HIDDEN = 0,
        EPSS_ONSCREEN,
    };
protected:
    virtual ~EditorBodyControl();
public:
    EditorBodyControl(const Rect & rect);
    
    virtual void WillAppear();
	virtual void Update(float32 timeElapsed);
    virtual void Input(UIEvent * event);
	virtual void Draw(const UIGeometricData &geometricData);

    virtual void SetSize(const Vector2 &newSize);

    void BeastProcessScene();
    virtual void DrawAfterChilds(const UIGeometricData &geometricData);
	    
    EditorScene * GetScene();
    void AddNode(Entity *node);
    
    void RemoveSelectedSGNode();
    Entity *GetSelectedSGNode(); //Scene Graph node
    
    void Refresh();
    
    
    void SetViewportSize(ResourceEditor::eViewportType viewportType);
    bool ControlsAreLocked();

    void GetCursorVectors(Vector3 * from, Vector3 * dir, const Vector2 &point);
    
    bool ToggleLandscapeEditor(int32 landscapeEditorMode);
	LandscapeEditorBase* GetLandscapeEditor(int32 landscapeEditorMode);
	LandscapeEditorBase* GetCurrentLandscapeEditor();
    
    void RecreteFullTilingTexture();

    //LandscapeEditorDelegate
    virtual void LandscapeEditorStarted();  //Show LE Controls
    virtual void LandscapeEditorFinished(); //Hide LE Controls
    
    //ModificationsPanelDelegate
    virtual void OnPlaceOnLandscape();
	void RestoreOriginalTransform();
	void ApplyTransform(float32 x, float32 y, float32 z);

    //GraphBaseDelegate
    virtual bool LandscapeEditorActive();
    virtual NodesPropertyControl *GetPropertyControl(const Rect &rect);
    
    bool TileMaskEditorEnabled();
    
    void SetScene(EditorScene *newScene);
    void SetCameraController(CameraController *newCameraController);
    
    void SelectNodeQt(Entity *node);
    void OnReloadRootNodesQt();
    
    
	SceneGraph * GetSceneGraph() { return sceneGraph; }


    bool RulerToolIsActive();
    bool RulerToolTriggered();

	bool CustomColorIsActive();
	bool VisibilityToolIsActive();
	bool ColorIsActive();
	bool HightMapIsActive();
    
	void UpdateModificationPanel(void);

	//custom color editor elements
	
	void SetBrushRadius(uint32 size);
	void SetColorIndex(uint32 indexInSet);
	void SaveTexture(const FilePath &path);
	void CustomColorsLoadTexture(const FilePath& path);
	FilePath CustomColorsGetCurrentSaveFileName();
	
	//visibility check tool
	void VisibilityToolSetPoint();
	void VisibilityToolSetArea();
	void VisibilityToolSetAreaSize(uint32 size);

    void ProcessIsSolidChanging();

	void RemoveNode(Entity* node);
	void SelectNode(Entity* node);

	ResourceEditor::eModificationActions GetModificationMode();
	void SetModificationMode(ResourceEditor::eModificationActions mode);
	bool IsLandscapeRelative();
	void SetLandscapeRelative(bool isLandscapeRelative);

	Matrix4 GetLandscapeOffset(const Matrix4& transform);

protected:

    void InitControls();
    
	void CreateModificationPanel();
    void ReleaseModificationPanel();
	void PrepareModMatrix(const Vector2 & point);

	void PlaceOnLandscape();
	void PlaceOnLandscape(Entity *node);
	
	Vector3 GetIntersection(const Vector3 & start, const Vector3 & dir, const Vector3 & planeN, const Vector3 & planePos);
	void InitMoving(const Vector2 & point);
	
    bool LandscapeEditorInput(UIEvent *event);
    bool RulerToolInput(UIEvent *event);
    bool ProcessKeyboard(UIEvent *event);
    bool ProcessMouse(UIEvent *event);
	
    //scene controls
    EditorScene * scene;
	Camera * activeCamera;
    UI3DView * scene3dView;
    CameraController * cameraController;

    // touch
    float32 currentTankAngle;
	bool inTouch;
	Vector2 touchStart;
	
	float32 startRotationInSec;

	//beast
	BeastManager * beastManager;

	Matrix4 startTransform;
	Matrix4 currTransform;
	Vector3 rotationCenter;
	bool isDrag;
    ModificationsPanel *modificationPanel;
	
	float32 axisSign[3];
	
	float32 moveKf;
    
    FilePath mainFilePath;
    
    
    void SelectNodeAtTree(Entity *node);

	//for moving object
	Vector3 startDragPoint;
	Vector3 planeNormal;
	
	Matrix4 translate1, translate2;

	Vector<Entity *> poppedEditorEntitiesForSave;
    
    struct AddedNode
    {
        Entity *nodeToAdd;
        Entity *nodeToRemove;
        Entity *parent;
    };
    Vector<AddedNode> nodesToAdd;
	
	
    ResourceEditor::eViewportType currentViewportType;
    
	void PackLightmaps();

	//modification options
	ResourceEditor::eModificationActions modificationMode;
	bool landscapeRelative;
	ArrowsNode* GetArrowsNode(bool createIfNotExist);
	void UpdateArrowsNode(Entity* node);
	bool InModificationMode();

    //Landscape Editor
    bool savedModificatioMode;
    void CreateLandscapeEditor();
    void ReleaseLandscapeEditor();
    
    LandscapeEditorColor *landscapeEditorColor;
    LandscapeEditorHeightmap *landscapeEditorHeightmap;
    LandscapeEditorCustomColors *landscapeEditorCustomColors;
	LandscapeEditorVisibilityCheckTool* landscapeEditorVisibilityTool;
    LandscapeEditorBase *currentLandscapeEditor;
    LandscapeToolsSelection *landscapeToolsSelection;
    
    //graps
    SceneGraph *sceneGraph;
    GraphBase *currentGraph;
    ePropertyShowState propertyShowState;
    
    RulerTool *landscapeRulerTool;

	Entity* originalNode;
	Entity* modifiedNode;
	Matrix4 transformBeforeModification;
};



#endif // __EDITOR_BODY_CONTROL_H__