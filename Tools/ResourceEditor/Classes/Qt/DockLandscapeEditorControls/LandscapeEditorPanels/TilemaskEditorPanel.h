#ifndef __RESOURCEEDITORQT__TILEMASKEDITORPANEL__
#define __RESOURCEEDITORQT__TILEMASKEDITORPANEL__

#include "LandscapeEditorBasePanel.h"
#include "DAVAEngine.h"
#include "Commands2/Command2.h"

#include "Render/UniqueStateSet.h"

using namespace DAVA;

class QComboBox;
class QRadioButton;
class SliderWidget;
class TileTexturePreviewWidget;
class QFrame;

class TilemaskEditorPanel: public LandscapeEditorBasePanel
{
	Q_OBJECT

public:
	static const int DEF_BRUSH_MIN_SIZE = 1;
	static const int DEF_BRUSH_MAX_SIZE = 40;
	static const int DEF_STRENGTH_MIN_VALUE = 0;
	static const int DEF_STRENGTH_MAX_VALUE = 60;
	static const int STRENGTH_MAX_BOUNDARY = 999;

	explicit TilemaskEditorPanel(QWidget* parent = 0);
	virtual ~TilemaskEditorPanel();

private slots:
	void SetBrushSize(int brushSize);
	void SetToolImage(int imageIndex);
	void SetStrength(int strength);
	void SetDrawTexture(int textureIndex);
	void OnTileColorChanged(int32 tileNumber, Color color);
	void SetNormalDrawing();
	void SetCopyPaste();

	void IncreaseBrushSize();
	void DecreaseBrushSize();
	void IncreaseBrushSizeLarge();
	void DecreaseBrushSizeLarge();

	void IncreaseStrength();
	void DecreaseStrength();
	void IncreaseStrengthLarge();
	void DecreaseStrengthLarge();

	void PrevTexture();
	void NextTexture();

	void PrevTool();
	void NextTool();

	void OnCommandExecuted(SceneEditor2* scene, const Command2* command, bool redo);

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

private:
	SliderWidget* sliderWidgetBrushSize;
	SliderWidget* sliderWidgetStrength;
	QComboBox* comboBrushImage;
	TileTexturePreviewWidget* tileTexturePreviewWidget;
	QRadioButton* radioDraw;
	QRadioButton* radioCopyPaste;
	QFrame* frameStrength;
	QFrame* frameTileTexturesPreview;

	void InitBrushImages();
	void UpdateTileTextures();

	void UpdateControls();

	int32 BrushSizeUIToSystem(int32 uiValue);
	int32 BrushSizeSystemToUI(int32 systemValue);

	float32 StrengthUIToSystem(int32 uiValue);
	int32 StrengthSystemToUI(float32 systemValue);

	void SplitImageToChannels(Image* image, Image*& r, Image*& g, Image*& b, Image*& a);
};

#endif /* defined(__RESOURCEEDITORQT__TILEMASKEDITORPANEL__) */
