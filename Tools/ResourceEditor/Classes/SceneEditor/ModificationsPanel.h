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



#ifndef __MODIFICATIONS_PANEL_H__
#define __MODIFICATIONS_PANEL_H__

#include "DAVAEngine.h"
#include "ComboBox.h"

using namespace DAVA;

class ModificationsPanelDelegate
{
public: 
    
    virtual void OnPlaceOnLandscape() = 0;
    
};

class ModificationPopUp;
class EditorScene;
class ModificationsPanel: public UIControl, public ComboBoxDelegate
{
public:

	enum eModState
    {
        MOD_MOVE = 0, 
        MOD_ROTATE,
        MOD_SCALE
	};

	enum eModAxis
    {
        AXIS_X = 0, 
        AXIS_Y,
        AXIS_Z,
        AXIS_XY,
        AXIS_YZ,
        AXIS_XZ,
		AXIS_COUNT
	};

    
protected:
    virtual ~ModificationsPanel();
public:
    ModificationsPanel(ModificationsPanelDelegate *newDelegate, const Rect &rect = Rect(), bool rectInAbsoluteCoordinates = false);
    
	virtual void Update(float32 timeElapsed);
    virtual void Input(UIEvent * touch);

    void SetScene(EditorScene *scene);
    void NodeSelected(Entity *newNode);
    
    Vector3 GetPlaneNormal();
    
    eModState GetModState();
    eModAxis GetModAxis();
    
    bool IsModificationMode();
    void IsModificationMode(bool value);

    bool IsLandscapeRelative();
    void IsLandscapeRelative(bool value);

	void OnReloadScene();
	void OnItemSelected(ComboBox *forComboBox, const String &itemKey, int itemIndex);

    void UpdateCollisionTypes(void);
protected:
    
    void PlaceOnLandscape();

	void OnModificationPressed(BaseObject * object, void * userData, void * callerData);
    void OnModificationPopUpPressed(BaseObject * object, void * userData, void * callerData);
	void OnModePressed(BaseObject * object, void * userData, void * callerData);
	
	void OnLandscapeRelative(BaseObject * object, void * userData, void * callerData);

	void UpdateModState(void);
	void PrepareModMatrix(const Vector2 & point);
	void ChangeCollisionModeShow(Entity * node);

	UIButton *btnMod[3];
	UIButton *btnAxis[3];
	UIButton *btnPopUp;
	UIButton *btnModeSelection;
	UIButton *btnModeModification;
	ComboBox *btnModeCollision;
	UIButton *btnPlaceOn;
	UIButton *btnLandscape;
	
	eModState modState;
	eModAxis modAxis;

	bool isModeModification;
	bool isLandscapeRelative;
	int32 modeCollision;

    UIControl * modificationPanel;
	ModificationPopUp * modificationPopUp;
	
    ModificationsPanelDelegate *delegate;
    EditorScene *workingScene;
};



#endif // __MODIFICATIONS_PANEL_H__
