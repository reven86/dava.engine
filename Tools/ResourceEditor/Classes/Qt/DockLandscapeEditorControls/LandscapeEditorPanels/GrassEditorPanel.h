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
