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



#ifndef __RESOURCEEDITORQT__BASEADDENTITYDIALOG__
#define __RESOURCEEDITORQT__BASEADDENTITYDIALOG__

#include <QDialog.h>
#include "DAVAEngine.h"
#include "Scene3D/Entity.h"
#include "DockProperties/PropertyEditorDialog.h"
#include <QDialogButtonBox>

#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataMetaObject.h"

class DAVA::Entity;

namespace Ui
{
	class BaseAddEntityDialog;
}

class BaseAddEntityDialog: public QDialog
{
	Q_OBJECT

public:
	
	enum eButtonAlign
	{
		BUTTON_ALIGN_LEFT = 0,
		BUTTON_ALIGN_RIGHT
	};
	
	explicit BaseAddEntityDialog( QWidget* parent = 0, QDialogButtonBox::StandardButtons buttons = QDialogButtonBox::Close);
	virtual ~BaseAddEntityDialog();
	
	void GetIncludedControls(QList<QWidget*>& includedWidgets);

	virtual DAVA::Entity* GetEntity();
	void virtual SetEntity(DAVA::Entity* );
	
	void AddButton( QWidget* widget, eButtonAlign orientation = BUTTON_ALIGN_LEFT);

protected slots:
	virtual void OnItemEdited(const QString &name, QtPropertyData *data);
    virtual void CommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo);
    

protected:
	virtual void FillPropertyEditorWithContent() = 0;

	virtual QtPropertyData* AddInspMemberToEditor(void *object, const DAVA::InspMember *);
	virtual QtPropertyData* AddKeyedArchiveMember(DAVA::KeyedArchive* _archive, const DAVA::String& _key, const DAVA::String& rowName);
	virtual QtPropertyData* AddMetaObject(void *_object, const DAVA::MetaInfo *_meta, const String& rowName);

	void AddControlToUserContainer(QWidget* widget);
	void AddControlToUserContainer(QWidget* widget, const DAVA::String& labelString);
	void RemoveControlFromUserContainer(QWidget* widget);
	void RemoveAllControlsFromUserContainer();

	void showEvent(QShowEvent * event);
	
	void PerformResize();
	
	DAVA::Entity* entity;
	QtPropertyEditor *propEditor;
	Ui::BaseAddEntityDialog *ui;
	
	DAVA::Map<QWidget*, QWidget*> additionalWidgetMap;
};

#endif /* defined(__RESOURCEEDITORQT__BASEADDENTITYDIALOG__) */