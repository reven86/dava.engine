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



#ifndef __QT_PROPERTY_WIDGET_H__
#define __QT_PROPERTY_WIDGET_H__

#include "PropertyEditorStateHelper.h"
#include "Tools/QtPosSaver/QtPosSaver.h"
#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "Scene/SceneData.h"
#include "Scene/SceneSignals.h"

class DAVA::Entity;

class PropertyEditor : public QtPropertyEditor
{
	Q_OBJECT

public:
	PropertyEditor(QWidget *parent = 0, bool connectToSceneSignals = true);
	~PropertyEditor();

	void SetNode(DAVA::Entity *node);
	void SetAdvancedMode(bool set);

protected:
    QtPropertyData* AppendIntrospectionInfo(void *object, const DAVA::InspInfo * info);
    
public slots:
	void sceneActivated(SceneEditor2 *scene);
	void sceneDeactivated(SceneEditor2 *scene);
	void EntitySelected(SceneEditor2 *scene, DAVA::Entity *entity);
	void EntityDeselected(SceneEditor2 *scene, DAVA::Entity *entity);
	
	void actionShowAdvanced();
	
	void EditActionComponent();

protected:
	bool advancedMode;
	QtPosSaver posSaver;

	DAVA::Entity *curNode;
	PropertyEditorStateHelper treeStateHelper;
};

#endif // __QT_PROPERTY_WIDGET_H__
