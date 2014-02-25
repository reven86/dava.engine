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



#ifndef __LOD_EDITOR_H__
#define __LOD_EDITOR_H__

#include <QWidget>
#include "Tools/QtPosSaver/QtPosSaver.h"

namespace Ui
{
	class LODEditor;
}

class QLabel;
class QDoubleSpinBox;
class QLineEdit;
class SceneEditor2;
class EditorLODData;

class LODEditor: public QWidget
{
	Q_OBJECT

public:
	explicit LODEditor(QWidget* parent = 0);
	~LODEditor();

protected slots:

    void GlobalSettingsButtonReleased();
    void ViewLODButtonReleased();
    void EditLODButtonReleased();
    
    
    void LODCorrectionChanged(double value);
    void ForceDistanceStateChanged(int checked);
    void ForceDistanceChanged(int distance);

    void SceneActivated(SceneEditor2 *scene);
	void SceneDeactivated(SceneEditor2 *scene);

    
    void LODDataChanged();
    void LODDistanceChangedBySpinbox(double value);
    void LODDistanceChangedBySlider(const QVector<int> &changedLayers, bool continuous);
    
    void CreatePlaneLODClicked();

    void ForceLayerActivated(int index);
    
protected:

    void SetupInternalUI();
    void InitCorrectionSpinBox(QDoubleSpinBox *spinbox, int index);
    void InitDistanceSpinBox(QLabel *name, QDoubleSpinBox *spinbox, int index);
    
    void SetupSceneSignals();
    
    void UpdateSpinboxColor(QDoubleSpinBox *spinbox);
    
    void AddLODRecurcive(DAVA::Entity *entity);
    void RemoveLODRecurcive(DAVA::Entity *entity);
    
    void SetSpinboxValue(QDoubleSpinBox *spinbox, double value);
    void SetForceLayerValues(int layersCount);
    
    void InvertVisibility(QWidget *widget);
    
    void UpdateWidgetVisibility();
    
private:
	Ui::LODEditor *ui;
    QtPosSaver posSaver;
    
    EditorLODData *editedLODData;
};

#endif //#ifndef __LOD_EDITOR_H__
