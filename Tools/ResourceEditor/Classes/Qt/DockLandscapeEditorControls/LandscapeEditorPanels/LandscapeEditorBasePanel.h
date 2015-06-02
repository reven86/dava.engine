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


#ifndef __RESOURCEEDITORQT__LANDSCAPEEDITORBASEPANEL__
#define __RESOURCEEDITORQT__LANDSCAPEEDITORBASEPANEL__

#include <QWidget>
#include "DAVAEngine.h"

class SceneEditor2;

class LandscapeEditorBasePanel: public QWidget
{
	Q_OBJECT
	
public:
	explicit LandscapeEditorBasePanel(QWidget* parent = 0);
	virtual ~LandscapeEditorBasePanel();
	
	void InitPanel(SceneEditor2* scene);
	void DeinitPanel();

protected slots:
	virtual void EditorToggled(SceneEditor2* scene);

protected:
	virtual void OnEditorEnabled();
	virtual void OnEditorDisabled();

	SceneEditor2* GetActiveScene();
	virtual bool GetEditorEnabled() = 0;

	virtual void SetWidgetsState(bool enabled) = 0;
	virtual void BlockAllSignals(bool block) = 0;

	virtual void InitUI() = 0;
	virtual void ConnectToSignals() = 0;

	virtual void StoreState() = 0;
	virtual void RestoreState() = 0;

	virtual void ConnectToShortcuts();
	virtual void DisconnectFromShortcuts();

private:
	SceneEditor2* activeScene;
};

#endif /* defined(__RESOURCEEDITORQT__LANDSCAPEEDITORBASEPANEL__) */
