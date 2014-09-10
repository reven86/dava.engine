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
									   VisibilityToolSystem::eVisibilityToolState state);

	void SaveTexture();
	void SetVisibilityPoint();
	void SetVisibilityArea();
	void SetVisibilityAreaSize(int areaSize);

	void IncreaseBrushSize();
	void DecreaseBrushSize();
	void IncreaseBrushSizeLarge();
	void DecreaseBrushSizeLarge();

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
	QPushButton* buttonSetVisibilityPoint;
	QPushButton* buttonSetVisibilityArea;
	QPushButton* buttonSaveTexture;
	SliderWidget* sliderWidgetAreaSize;

	int32 AreaSizeUIToSystem(int32 uiValue);
	int32 AreaSizeSystemToUI(int32 systemValue);
};

#endif /* defined(__RESOURCEEDITORQT__VISIBILITYTOOLPANEL__) */
