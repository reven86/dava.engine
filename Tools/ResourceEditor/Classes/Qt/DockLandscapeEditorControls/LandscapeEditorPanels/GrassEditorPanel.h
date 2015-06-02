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


#ifndef __RESOURCEEDITORQT__GRASSEDITORPANEL__
#define __RESOURCEEDITORQT__GRASSEDITORPANEL__

#include <QWidget>
#include <QTableWidget>
#include <QCheckBox>
#include <QToolButton>

#include "DAVAEngine.h"

#include "LandscapeEditorBasePanel.h"
#include "Scene/System/GrassEditorSystem.h"
#include "DockLODEditor/DistanceSlider.h"

#define GRASS_EDITOR_LAYERS_COUNT 4

class GrassEditorPanel: public LandscapeEditorBasePanel
{
	Q_OBJECT

public:
	explicit GrassEditorPanel(QWidget* parent = 0);
	~GrassEditorPanel();

protected:
	virtual bool GetEditorEnabled();
	virtual void OnEditorEnabled();

	virtual void SetWidgetsState(bool enabled);
	virtual void BlockAllSignals(bool block);
	
	virtual void InitUI();
	virtual void ConnectToSignals();
	
	virtual void StoreState();
	virtual void RestoreState();

	virtual void ConnectToShortcuts();
	virtual void DisconnectFromShortcuts();

protected slots:
    void OnLayerSelected(int currentRow, int currentColumn, int previousRow, int previousColumn);
    void OnLayerChecked(int state);
    void OnHeightChanged(int value);
    void OnDensityChanged(int value);
    void OnDensityAffectToggled(bool checked);
    void OnDensityAddToggled(bool checked);
    void OnHightAffectToggled(bool checked);
    void OnHightAddToggled(bool checked);

    DAVA::Rect2i MapTexCoord(const DAVA::TextureSheetCell &cell, DAVA::uint32 w, DAVA::uint32 h) const;

private:
    QTableWidget *layersList;

    QCheckBox *layerCheckBoxes[GRASS_EDITOR_LAYERS_COUNT];
    QSlider *grassHeight;
    QSlider *grassDensity;
    QToolButton *grassHeightAffect;
    QToolButton *grassDensityAffect;
    QToolButton *grassHeightAdd;
    QToolButton *grassDensityAdd;
    DistanceSlider *lodPreview;
};

#endif /* defined(__RESOURCEEDITORQT__GRASSEDITORPANEL__) */
