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


#ifndef __RESOURCEEDITORQT__VISIBILITYTOOLPANEL__
#define __RESOURCEEDITORQT__VISIBILITYTOOLPANEL__

#include "LandscapeEditorBasePanel.h"
#include "DAVAEngine.h"
#include "../../Scene/System/VisibilityToolSystem.h"

using namespace DAVA;

class QPushButton;
class SliderWidget;

class VisibilityToolPanel: public LandscapeEditorBasePanel
{
	Q_OBJECT

public:
	static const int DEF_AREA_MIN_SIZE = 3;
	static const int DEF_AREA_MAX_SIZE = 40;

	explicit VisibilityToolPanel(QWidget* parent = 0);
	~VisibilityToolPanel();

private slots:
    void SetVisibilityToolButtonsState(SceneEditor2* scene,
                                       VisibilityToolSystem::State state);

    void SaveTexture();
	void SetVisibilityPoint();
	void SetVisibilityArea();

protected:
	virtual bool GetEditorEnabled();

	virtual void SetWidgetsState(bool enabled);
	virtual void BlockAllSignals(bool block);

	virtual void InitUI();
	virtual void ConnectToSignals();

	virtual void StoreState();
	virtual void RestoreState();

	virtual void ConnectToShortcuts();
	virtual void DisconnectFromShortcuts();

private:
    QPushButton* buttonSetVisibilityPoint = nullptr;
    QPushButton* buttonSetVisibilityArea = nullptr;
    QPushButton* buttonSaveTexture = nullptr;

    int32 AreaSizeUIToSystem(int32 uiValue);
	int32 AreaSizeSystemToUI(int32 systemValue);
};

#endif /* defined(__RESOURCEEDITORQT__VISIBILITYTOOLPANEL__) */
