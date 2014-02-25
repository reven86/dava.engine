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

#ifndef __MATERIAL_EDITOR_H__
#define __MATERIAL_EDITOR_H__

#include <QDialog>
#include <QtGui>
#include "DAVAEngine.h"

#include "Scene/SceneSignals.h"
#include "Tools/QtPosSaver/QtPosSaver.h"
#include "DockProperties/PropertyEditorStateHelper.h"

namespace Ui {
	class MaterialEditor;
}

class MaterialEditor : public QDialog, public DAVA::Singleton<MaterialEditor>
{
	Q_OBJECT

private:
    typedef QMap< int, bool > ExpandMap;

public:
	MaterialEditor(QWidget *parent = 0);
	~MaterialEditor();

	void SelectMaterial(DAVA::NMaterial *material);
	void SelectEntities(DAVA::NMaterial *material);

public slots:
	void sceneActivated(SceneEditor2 *scene);
	void sceneDeactivated(SceneEditor2 *scene);
	void commandExecuted(SceneEditor2 *scene, const Command2 *command, bool redo);
	void materialSelected(const QItemSelection & selected, const QItemSelection & deselected);

protected slots:
	void OnAddProperty();
	void OnRemProperty();
	void OnAddTexture();
	void OnRemTexture();
	void OnTemplateChanged(int index);
	void OnPropertyEdited(const QModelIndex &);
    void OnSwitchQuality(bool checked);
    void OnMaterialReload(bool checked);
    void OnMaterialSetFog(bool checked);

protected:
	virtual void showEvent(QShowEvent * event);

	void SetCurMaterial(DAVA::NMaterial *material);
	void FillMaterialProperties(DAVA::NMaterial *material);
    void FillMaterialTemplates(DAVA::NMaterial *material);

    QVariant CheckForTextureDescriptor(const QVariant& value);

private slots:
    void onFilterChanged();
    void onCurrentExpandModeChange( bool mode );
    void autoExpand();

private:
    void initActions();

	Ui::MaterialEditor *ui;
	QtPosSaver posSaver;

	DAVA::NMaterial *curMaterial;

	PropertyEditorStateHelper *treeStateHelper;
    ExpandMap expandMap;
};

class MaterialEditorFogDialog : public QDialog
{
    Q_OBJECT

public:
    enum FogType
    {
        FOG_DISABLED,
        FOG_EXPONENTIAL,
        FOG_LINEAR
    };

    struct FogParams
    {
        FogType type;
        DAVA::Color color;
        DAVA::float32 density;
        DAVA::float32 start;
        DAVA::float32 end;

        FogParams() : type(FOG_DISABLED), density(0), start(0), end(0) {}
    };

    MaterialEditorFogDialog();

    void SetFogParams(const FogParams &params);
    FogParams GetFogParams() const;

public slots:
    void OnColorPick();
    void OnModeSwitch(bool state);

protected:
    QRadioButton *disabled;
    QRadioButton *exponential;
    QRadioButton *linear;
    QPushButton *fogColor;
    QDoubleSpinBox *fogDensity;
    QDoubleSpinBox *fogStart;
    QDoubleSpinBox *fogEnd;
    QLabel *labelColor;
    QLabel *labelDensity;
    QLabel *labelStart;
    QLabel *labelEnd;

};

#endif
