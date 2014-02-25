#ifndef __RESOURCEEDITORQT__CUSTOMCOLORSPANEL__
#define __RESOURCEEDITORQT__CUSTOMCOLORSPANEL__

#include "LandscapeEditorBasePanel.h"
#include "DAVAEngine.h"
#include "Main/Request.h"

using namespace DAVA;

class QComboBox;
class QPushButton;
class SliderWidget;

class CustomColorsPanel: public LandscapeEditorBasePanel
{
	Q_OBJECT
	
public:
	static const int DEF_BRUSH_MIN_SIZE = 3;
	static const int DEF_BRUSH_MAX_SIZE = 40;
	
	explicit CustomColorsPanel(QWidget* parent = 0);
	~CustomColorsPanel();

private slots:
	void ProjectOpened(const QString &path);

	void SetBrushSize(int brushSize);
	void SetColor(int color);
	bool SaveTexture();
	void LoadTexture();
	void SaveTextureIfNeeded(SceneEditor2* scene);

	void IncreaseBrushSize();
	void DecreaseBrushSize();
	void IncreaseBrushSizeLarge();
	void DecreaseBrushSizeLarge();

	void PrevTexture();
	void NextTexture();

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
	QComboBox* comboColor;
	SliderWidget* sliderWidgetBrushSize;
	QPushButton* buttonSaveTexture;
	QPushButton* buttonLoadTexture;
	
	void InitColors();
	
	int32 BrushSizeUIToSystem(int32 uiValue);
	int32 BrushSizeSystemToUI(int32 systemValue);
};

#endif /* defined(__RESOURCEEDITORQT__CUSTOMCOLORSPANEL__) */
