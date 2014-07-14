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


#ifndef ACTIONCOMPONENTEDITOR_H
#define ACTIONCOMPONENTEDITOR_H

#include "DAVAEngine.h"
#include "Scene3D/Components/ActionComponent.h"
#include <QDialog>
#include <QStyledItemDelegate>


namespace Ui {
class ActionComponentEditor;
}

class ActionComponentEditor;
class ActionItemEditDelegate : public QStyledItemDelegate
{
	Q_OBJECT
	
public:
	
	ActionItemEditDelegate(QObject *parent = 0);
	
	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
						  const QModelIndex &index) const;
	
	void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model,
					  const QModelIndex &index) const;
	
	void updateEditorGeometry(QWidget *editor,
							  const QStyleOptionViewItem &option, const QModelIndex &index) const;
	
	void SetComponent(DAVA::ActionComponent* component);
	void SetComponentEditor(ActionComponentEditor* editor);
	
private:
    QWidget *createFloatEditor(QWidget *parent) const;
	
	DAVA::ActionComponent* targetComponent;
	ActionComponentEditor* componentEditor;
    QMap< QString, int > actionTypes;
    QMap< QString, int > eventTypes;
};


class ActionComponentEditor : public QDialog
{
    Q_OBJECT
	
public:
    explicit ActionComponentEditor(QWidget *parent = 0);
    ~ActionComponentEditor();
	
	void SetComponent(DAVA::ActionComponent* component);
	void Update();
	
private:
	
	void UpdateTableFromComponent(DAVA::ActionComponent* component);
	DAVA::ActionComponent::Action GetDefaultAction();
	bool IsActionPresent(const DAVA::ActionComponent::Action action);
	
private slots:
	void OnAddAction();
	void OnRemoveAction();
	void OnSelectedItemChanged();
    
private:
    Ui::ActionComponentEditor *ui;
	
	DAVA::ActionComponent* targetComponent;
	ActionItemEditDelegate editDelegate;

    QMap< int, QString > actionTypes;
    QMap< int, QString > eventTypes;

};

#endif // ACTIONCOMPONENTEDITOR_H
