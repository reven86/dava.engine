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
#include "DAVAEngine.h"

#include "MaterialModel.h"
#include "Scene/SceneSignals.h"
#include "Tools/QtPosSaver/QtPosSaver.h"

namespace Ui {
	class MaterialEditor;
}

class MaterialEditor : public QDialog, public DAVA::Singleton<MaterialEditor>
{
	Q_OBJECT

public:
	MaterialEditor(QWidget *parent = 0);
	~MaterialEditor();

public slots:
	void sceneActivated(SceneEditor2 *scene);
	void sceneDeactivated(SceneEditor2 *scene);
	void sceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected);
	void materialClicked(const QModelIndex &index);

protected:
	virtual void showEvent(QShowEvent * event);

private:
	Ui::MaterialEditor *ui;
	QtPosSaver posSaver;
};

#endif
