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



#include <QPushButton>
#include "QtPropertyData.h"

QtPropertyData::QtPropertyData()
	: curFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable)
	, parent(NULL)
	, updatingValue(false)
	, model(NULL)
	, userData(NULL)
	, optionalButtonsViewport(NULL)
{ }

QtPropertyData::QtPropertyData(const QVariant &value, Qt::ItemFlags flags)
	: curValue(value)
	, curFlags(flags)
	, parent(NULL)
	, updatingValue(false)
	, model(NULL)
	, userData(NULL)
	, optionalButtonsViewport(NULL)
{ }

QtPropertyData::~QtPropertyData()
{
	for(int i = 0; i < childrenData.size(); ++i)
	{
		delete childrenData.at(i);
	}

	childrenData.clear();

	for (int i = 0; i < optionalButtons.size(); i++)
	{
		optionalButtons.at(i)->deleteLater();
	}

	if(NULL != userData)
	{
		delete userData;
	}
}

QVariant QtPropertyData::data(int role) const
{
	QVariant ret;

	switch(role)
	{
	case Qt::EditRole:
	case Qt::DisplayRole:
		ret = GetAlias();
		if(!ret.isValid())
		{
			ret = GetValue();
		}
		break;
	case Qt::CheckStateRole:
		if(GetFlags() & Qt::ItemIsUserCheckable)
		{
			ret = GetValue().toBool() ? Qt::Checked : Qt::Unchecked;
		}
		break;
	case Qt::FontRole:
	case Qt::DecorationRole:
	case Qt::BackgroundRole:
	case Qt::ForegroundRole:
		ret = style.value(role);
		break;
	default:
		break;
	}

	return ret;
}

bool QtPropertyData::setData(const QVariant & value, int role)
{
	bool ret = false;

	switch(role)
	{
	case Qt::EditRole:
		SetValue(value, QtPropertyData::VALUE_EDITED);
		ret = true;
		break;
	case Qt::CheckStateRole:
		(value.toInt() == Qt::Unchecked) ? SetValue(false, QtPropertyData::VALUE_EDITED) : SetValue(true, QtPropertyData::VALUE_EDITED);
		ret = true;
		break;
	case Qt::FontRole:
	case Qt::DecorationRole:
	case Qt::BackgroundRole:
	case Qt::ForegroundRole:
		style.insert(role, value);
		ret = true;
		break;
	default:
		break;
	}

	return ret;
}

QVariant QtPropertyData::GetValue() const
{
	if(curValue.isNull() || !curValue.isValid())
	{
		curValue = GetValueInternal();
	}
	else
	{
		const_cast<QtPropertyData*>(this)->UpdateValue();
	}

	return curValue;
}

void QtPropertyData::SetValue(const QVariant &value, ValueChangeReason reason)
{
	QVariant oldValue = curValue;

	curValue = value;
	SetValueInternal(curValue);

	if(curValue != oldValue)
	{
		UpdateDown();
		UpdateUp();

		EmitDataChanged(reason);
	}
}

bool QtPropertyData::UpdateValue()
{
	bool ret = false;

	if(!updatingValue)
	{
		updatingValue = true;

		if(UpdateValueInternal())
		{
			curValue = GetValueInternal();
			EmitDataChanged(VALUE_SOURCE_CHANGED);

			ret = true;
		}

		updatingValue = false;
	}

	return ret;
}

QVariant QtPropertyData::GetAlias() const
{
	// this will force update internalValue if 
	// it source was changed 
	GetValue();

	return GetValueAlias();
}

void QtPropertyData::SetName(const QString &name)
{
	if(NULL != parent)
	{
		int i = parent->ChildIndex(this);
		if(i >= 0)
		{
			// update name in parent list
			parent->childrenNames[i] = name;
		}
	}

	curName = name;
}

QString QtPropertyData::GetName() const
{
	return curName;
}

QString QtPropertyData::GetPath() const
{
	QString path = curName;

	// search top level parent
	const QtPropertyData *parent = this;
	while(NULL != parent->Parent())
	{
		parent = parent->Parent();
		path = parent->curName + "/" + path;
	}

	return path;
}

void QtPropertyData::SetIcon(const QIcon &icon)
{
	setData(QVariant(icon), Qt::DecorationRole);
}

QIcon QtPropertyData::GetIcon() const
{
	return qvariant_cast<QIcon>(data(Qt::DecorationRole));
}

QFont QtPropertyData::GetFont() const
{
	return qvariant_cast<QFont>(data(Qt::FontRole));
}

void QtPropertyData::SetFont(const QFont &font)
{
	setData(QVariant(font), Qt::FontRole);
}

QBrush QtPropertyData::GetBackground() const
{
	return qvariant_cast<QBrush>(data(Qt::BackgroundRole));
}

void QtPropertyData::SetBackground(const QBrush &brush)
{
	setData(QVariant(brush), Qt::BackgroundRole);
}

QBrush QtPropertyData::GetForeground() const
{
	return qvariant_cast<QBrush>(data(Qt::ForegroundRole));
}

void QtPropertyData::SetForeground(const QBrush &brush)
{
	setData(QVariant(brush), Qt::ForegroundRole);
}

void QtPropertyData::ResetStyle()
{
	style.remove(Qt::ForegroundRole);
	style.remove(Qt::BackgroundRole);
	style.remove(Qt::FontRole);
}

Qt::ItemFlags QtPropertyData::GetFlags() const
{
	return curFlags;
}

void QtPropertyData::SetFlags(Qt::ItemFlags flags)
{
	curFlags = flags;
}

void QtPropertyData::SetCheckable(bool checkable)
{
	(checkable) ? (curFlags |= Qt::ItemIsUserCheckable) : (curFlags &= ~Qt::ItemIsUserCheckable);
}

bool QtPropertyData::IsCheckable() const
{
	return (curFlags & Qt::ItemIsUserCheckable);
}

void QtPropertyData::SetChecked(bool checked)
{
	setData(QVariant(checked), Qt::CheckStateRole);
}

bool QtPropertyData::IsChecked() const
{
	return data(Qt::CheckStateRole).toBool();
}

void QtPropertyData::SetEditable(bool editable)
{
	(editable) ? (curFlags |= Qt::ItemIsEditable) : (curFlags &= ~Qt::ItemIsEditable);
}

bool QtPropertyData::IsEditable() const
{
	return (curFlags & Qt::ItemIsEditable);
}

void QtPropertyData::SetEnabled(bool enabled)
{
	(enabled) ? (curFlags |= Qt::ItemIsEnabled) : (curFlags &= ~Qt::ItemIsEnabled);
}

void QtPropertyData::SetUserData(UserData *data)
{
	if(NULL != userData)
	{
		delete userData;
	}

	userData = data;

	if(NULL != model)
	{
		model->DataChanged(this, VALUE_SET);
	}
}

QtPropertyData::UserData* QtPropertyData::GetUserData() const
{
	return userData;
}

const DAVA::MetaInfo* QtPropertyData::MetaInfo() const
{
	return NULL;
}

QtPropertyData* QtPropertyData::GetProxyOriginal()
{
	return this;
}

bool QtPropertyData::IsEnabled() const
{
	return (curFlags & Qt::ItemIsEnabled);
}

QtPropertyModel* QtPropertyData::GetModel() const
{
	return model;
}

void QtPropertyData::SetModel(QtPropertyModel *_model)
{
	model = _model;

	for(int i = 0; i < childrenData.size(); ++i)
	{
		QtPropertyData *child = childrenData.at(i);
		if(NULL != child)
		{
			child->SetModel(model);
		}
	}
}

QWidget* QtPropertyData::CreateEditor(QWidget *parent, const QStyleOptionViewItem& option) const
{ 
	return CreateEditorInternal(parent, option);
}

bool QtPropertyData::EditorDone(QWidget *editor)
{
    return EditorDoneInternal(editor);
}

bool QtPropertyData::SetEditorData(QWidget *editor)
{
    return SetEditorDataInternal(editor);
}

void QtPropertyData::EmitDataChanged(ValueChangeReason reason)
{
	if(NULL != model)
	{
		model->DataChanged(this, reason);
	}
}

void QtPropertyData::UpdateUp()
{
	if(NULL != parent)
	{
		parent->UpdateValue();
		parent->UpdateUp();
	}
}

void QtPropertyData::UpdateDown()
{
	for(int i = 0; i < childrenData.size(); ++i)
	{
		QtPropertyData *child = childrenData.at(i);
		if(NULL != child)
		{
			child->UpdateValue();
			child->UpdateDown();
		}
	}
}

QtPropertyData* QtPropertyData::Parent() const
{
	return parent;
}

void QtPropertyData::ChildAdd(const QString &key, QtPropertyData *data)
{
	ChildInsert(key, data, ChildCount());
}

void QtPropertyData::ChildAdd(const QString &key, const QVariant &value)
{
	ChildInsert(key, new QtPropertyData(value), ChildCount());
}

void QtPropertyData::ChildInsert(const QString &key, QtPropertyData *data, int pos)
{
	if(NULL != data && !key.isEmpty())
	{
		if(NULL != model)
		{
			model->DataAboutToBeAdded(this, childrenData.size(), childrenData.size());
		}

		if(pos >= 0 && pos < childrenData.size())
		{
			childrenData.insert(pos, data);
			childrenNames.insert(pos, key);
		}
		else
		{
			childrenData.append(data);
			childrenNames.append(key);
		}

		data->curName = key;
		data->parent = this;
		data->SetModel(model);
		data->SetOWViewport(optionalButtonsViewport);

		if(NULL != model)
		{
			model->DataAdded();
		}
	}
}

void QtPropertyData::ChildInsert(const QString &key, const QVariant &value, int pos)
{
	ChildInsert(key, new QtPropertyData(value), pos);
}

int QtPropertyData::ChildCount() const
{
	return childrenData.size();
}

QtPropertyData* QtPropertyData::ChildGet(int i) const
{
	QtPropertyData *ret = NULL;

	if(i >= 0 && i < childrenData.size())
	{
		ret = childrenData.at(i);
	}

	return ret;
}

QtPropertyData* QtPropertyData::ChildGet(const QString &key) const
{
	QtPropertyData *data = NULL;

	int index = childrenNames.indexOf(key);
	if(-1 != index)
	{
		data = childrenData.at(index);
	}

	return data;
}

int QtPropertyData::ChildIndex(QtPropertyData *data) const
{
	return childrenData.indexOf(data);
}

void QtPropertyData::ChildExtract(QtPropertyData *data)
{
	int index = childrenData.indexOf(data);
	ChildRemoveInternal(index, false);
}

void QtPropertyData::ChildRemove(QtPropertyData *data)
{
	int index = childrenData.indexOf(data);
	ChildRemoveInternal(index, true);
}

void QtPropertyData::ChildRemove(const QString &key)
{
	int index = childrenNames.indexOf(key);
	ChildRemoveInternal(index, true);
}

void QtPropertyData::ChildRemove(int index)
{
	ChildRemoveInternal(index, true);
}

void QtPropertyData::ChildRemoveInternal(int index, bool del)
{
	if(index >= 0 && index < childrenData.size())
	{
		if(NULL != model)
		{
			model->DataAboutToBeRemoved(this, index, index);
		}

		QtPropertyData *data = childrenData.at(index);

		childrenData.removeAt(index);
		childrenNames.removeAt(index);

		if(del)
		{
			delete data;
		}

		if(NULL != model)
		{
			model->DataRemoved();
		}
	}
}

void QtPropertyData::ChildRemoveAll()
{
	if(childrenData.size() > 0)
	{
		if(NULL != model)
		{
			model->DataAboutToBeRemoved(this, 0, childrenData.size() - 1);
		}

		for(int i = 0; i < childrenData.size(); ++i)
		{
			delete childrenData.at(i);
		}

		childrenData.clear();
		childrenNames.clear();

		if(NULL != model)
		{
			model->DataRemoved();
		}
	}
}

int QtPropertyData::GetButtonsCount() const
{
	return optionalButtons.size();
}

QtPropertyToolButton* QtPropertyData::GetButton(int index)
{
	QtPropertyToolButton *ret = NULL;

	if(index >= 0 && index < optionalButtons.size())
	{
		ret = optionalButtons.at(index);
	}

	return ret;
}

QtPropertyToolButton* QtPropertyData::AddButton()
{
	QtPropertyToolButton *button = new QtPropertyToolButton(optionalButtonsViewport);

	optionalButtons.append(button);
	button->setGeometry(0, 0, 18, 18);
	button->setAttribute(Qt::WA_NoSystemBackground, true);
	button->hide();

	return button;
}

void QtPropertyData::RemButton(int index)
{
	if(index >= 0 && index < optionalButtons.size())
	{
		if(NULL != optionalButtons.at(index))
		{
			delete optionalButtons.at(index);
		}

		optionalButtons.remove(index);
	}
}

void QtPropertyData::RemButton(QtPropertyToolButton *button)
{
	for(int i = 0; i < optionalButtons.size(); ++i)
	{
		if(optionalButtons[i] == button)
		{
			RemButton(i);
			break;
		}
	}
}

QWidget* QtPropertyData::GetOWViewport() const
{
	return optionalButtonsViewport;
}

void QtPropertyData::SetOWViewport(QWidget *viewport)
 {
	optionalButtonsViewport = viewport;

	for(int i = 0; i < optionalButtons.size(); ++i)
	{
		if(NULL != optionalButtons[i])
		{
			optionalButtons[i]->setParent(viewport);
		}
	}

	for(int i = 0; i < childrenData.size(); i++)
	{
		childrenData.at(i)->SetOWViewport(viewport);
	}
}

void* QtPropertyData::CreateLastCommand() const
{
	// can be re-implemented by sub-class

	return NULL;
}

QVariant QtPropertyData::GetValueInternal() const
{
	// should be re-implemented by sub-class

	return curValue;
}

bool QtPropertyData::UpdateValueInternal()
{
	return false;
}

QVariant QtPropertyData::GetValueAlias() const
{
	// should be re-implemented by sub-class

	return QVariant();
}

void QtPropertyData::SetValueInternal(const QVariant &value)
{
	// should be re-implemented by sub-class

	curValue = value;
}

QWidget* QtPropertyData::CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option) const
{
	// should be re-implemented by sub-class

	return NULL;
}

bool QtPropertyData::EditorDoneInternal(QWidget *editor)
{
	// should be re-implemented by sub-class
	return false;
}

bool QtPropertyData::SetEditorDataInternal(QWidget *editor)
{
	// should be re-implemented by sub-class
	return false;
}
