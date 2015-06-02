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


#ifndef __RESOURCEEDITORQT__HEIGHTMAPEDITORPANEL__
#define __RESOURCEEDITORQT__HEIGHTMAPEDITORPANEL__

#include "LandscapeEditorBasePanel.h"
#include "DAVAEngine.h"
#include "Qt/Scene/System/HeightmapEditorSystem.h"

using namespace DAVA;

class QComboBox;
class QRadioButton;
class QLineEdit;
class SliderWidget;
class QDoubleSpinBox;

class HeightmapEditorPanel: public LandscapeEditorBasePanel
{
	Q_OBJECT

public:
	static const int DEF_BRUSH_MIN_SIZE = 3;
	static const int DEF_BRUSH_MAX_SIZE = 40;
	static const int DEF_STRENGTH_MAX_VALUE = 30;
	static const int DEF_AVERAGE_STRENGTH_MIN_VALUE = 0;
	static const int DEF_AVERAGE_STRENGTH_MAX_VALUE = 60;
	static const int STRENGTH_MIN_BOUNDARY = -999;
	static const int STRENGTH_MAX_BOUNDARY = 999;
	static const int AVG_STRENGTH_MIN_BOUNDARY = 0;
	static const int AVG_STRENGTH_MAX_BOUNDARY = 999;

	explicit HeightmapEditorPanel(QWidget* parent = 0);
	~HeightmapEditorPanel();

private slots:
	void SetDropperHeight(SceneEditor2* scene, double height);
	void HeightUpdatedManually();

	void SetBrushSize(int brushSize);
	void SetToolImage(int toolImage);
	void SetRelativeDrawing();
	void SetAverageDrawing();
	void SetAbsoluteDrawing();
	void SetAbsDropDrawing();
	void SetDropper();
	void SetHeightmapCopyPaste();
	void SetStrength(int strength);
	void SetAverageStrength(int averageStrength);

	void IncreaseBrushSize();
	void DecreaseBrushSize();
	void IncreaseBrushSizeLarge();
	void DecreaseBrushSizeLarge();

	void IncreaseStrength();
	void DecreaseStrength();
	void IncreaseStrengthLarge();
	void DecreaseStrengthLarge();

	void IncreaseAvgStrength();
	void DecreaseAvgStrength();
	void IncreaseAvgStrengthLarge();
	void DecreaseAvgStrengthLarge();

	void PrevTool();
	void NextTool();

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
	
	bool eventFilter(QObject *o, QEvent *e);

private:
	SliderWidget* sliderWidgetBrushSize;
	SliderWidget* sliderWidgetStrength;
	SliderWidget* sliderWidgetAverageStrength;
	QComboBox* comboBrushImage;
	QRadioButton* radioCopyPaste;
	QRadioButton* radioAbsDrop;
	QRadioButton* radioAbsolute;
	QRadioButton* radioAverage;
	QRadioButton* radioDropper;
	QRadioButton* radioRelative;
	QDoubleSpinBox* editHeight;

	void InitBrushImages();
	void UpdateRadioState(HeightmapEditorSystem::eHeightmapDrawType type);
	void SetDrawingType(HeightmapEditorSystem::eHeightmapDrawType type);


	float32 GetBrushScaleCoef();
	int32 BrushSizeUIToSystem(int32 uiValue);
	int32 BrushSizeSystemToUI(int32 systemValue);

	float32 StrengthUIToSystem(int32 uiValue);
	int32 StrengthSystemToUI(float32 systemValue);

	float32 AverageStrengthUIToSystem(int32 uiValue);
	int32 AverageStrengthSystemToUI(float32 systemValue);
};

#endif /* defined(__RESOURCEEDITORQT__HEIGHTMAPEDITORPANEL__) */
