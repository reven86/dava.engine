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
	: curFlags(0)
	, parent(NULL)
	, optionalWidgetViewport(NULL)
{ }

QtPropertyData::QtPropertyData(const QVariant &value)
	: curValue(value)
	, curFlags(0)
	, parent(NULL)
	, optionalWidgetViewport(NULL)
{ }

QtPropertyData::~QtPropertyData() 
{
	QHashIterator<QString, QtPropertyData*> i = QHashIterator<QString, QtPropertyData*>(children);
	while(i.hasNext())
	{
		i.next();

		QtPropertyData *data = i.value();
		if(NULL != data)
		{
			delete data;
		}
	}

	for (int i = 0; i < optionalWidgets.size(); i++)
	{
		if(NULL != optionalWidgets.at(i).widget)
		{
			delete optionalWidgets.at(i).widget;
		}
	}
}

QVariant QtPropertyData::GetValue()
{
	QVariant internalValue = GetValueInternal();

	if(internalValue != curValue)
	{
		curValue = internalValue;
		ChildNeedUpdate();
	}

	return curValue;
}

QVariant QtPropertyData::GetAlias()
{
	return GetValueAlias();
}

void QtPropertyData::SetValue(const QVariant &value)
{
	QVariant oldValue = curValue;

	curValue = value;
	SetValueInternal(curValue);

	if(curValue != oldValue)
	{
		emit ValueChanged();
	}

	// change signal going down to childs
	ChildNeedUpdate();

	// change signal going up to parents
	ParentUpdate();
}

void QtPropertyData::SetIcon(const QIcon &icon)
{
	curIcon = icon;
}

QIcon QtPropertyData::GetIcon()
{
	return curIcon;
}

int QtPropertyData::GetFlags()
{
	return curFlags;
}

void QtPropertyData::SetFlags(int flags)
{
	if(curFlags != flags)
	{
		curFlags = flags;
		emit FlagsChanged();
	}
}

QWidget* QtPropertyData::CreateEditor(QWidget *parent, const QStyleOptionViewItem& option) 
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

void QtPropertyData::ParentUpdate()
{
	if(NULL != parent)
	{
		parent->ChildChanged(parent->children.key(this), this);
		parent->ParentUpdate();
	}
}

void QtPropertyData::ChildAdd(const QString &key, QtPropertyData *data)
{
	if(NULL != data && !key.isEmpty())
	{
		int sz = children.size();
        
        children.insert(key, data);
        childrenOrder.insert(key, sz);

		data->parent = this;

		emit ChildAdded(key, data);
	}
}

void QtPropertyData::ChildAdd(const QString &key, const QVariant &value)
{
	ChildAdd(key, new QtPropertyData(value));
}

int QtPropertyData::ChildCount()
{
	return children.size();
}

QPair<QString, QtPropertyData*> QtPropertyData::ChildGet(int i)
{
	QPair<QString, QtPropertyData*> p("", NULL);

	if(i >= 0 && i < children.size())
	{
		p.first = childrenOrder.key(i);
		p.second = children[p.first];
	}

	return p;
}

QtPropertyData * QtPropertyData::ChildGet(const QString &key)
{
	QtPropertyData *data = NULL;

	if(children.contains(key))
	{
		data = children[key];
	}

	return data;
}

void QtPropertyData::ChildRemove(const QString &key)
{
	QtPropertyData *data = ChildGet(key);

	if(NULL != data)
	{
		emit ChildRemoving(key, data);

		children.remove(key);
		childrenOrder.remove(key);

		delete data;
	}
}

void QtPropertyData::ChildRemove(int i)
{
	QPair<QString, QtPropertyData*> pair = ChildGet(i);

	if(!pair.first.isEmpty() && NULL != pair.second)
	{
		emit ChildRemoving(pair.first, pair.second);

		children.remove(pair.first);
		childrenOrder.remove(pair.first);

		delete pair.second;
	}
}

void QtPropertyData::ChildRemove(QtPropertyData *data)
{
	QString key = children.key(data, "");
	if(!key.isEmpty())
	{
		ChildRemove(key);
	}
}

int QtPropertyData::GetOWCount()
{
	return optionalWidgets.size();
}

const QtPropertyOW* QtPropertyData::GetOW(int index)
{
	const QtPropertyOW *ret = NULL;

	if(index >= 0 && index < optionalWidgets.size())
	{
		ret = &optionalWidgets.at(index);
	}

	return ret;
}

void QtPropertyData::AddOW(const QtPropertyOW &ow)
{
	optionalWidgets.append(ow);

	if(NULL != ow.widget)
	{
		ow.widget->setParent(optionalWidgetViewport);
	}
}

void QtPropertyData::RemOW(int index)
{
	if(index >= 0 && index < optionalWidgets.size())
	{
		if(NULL != optionalWidgets.at(index).widget)
		{
			delete optionalWidgets.at(index).widget;
		}

		optionalWidgets.remove(index);
	}
}

QWidget* QtPropertyData::GetOWViewport()
{
	return optionalWidgetViewport;
}

void QtPropertyData::SetOWViewport(QWidget *viewport)
{
	optionalWidgetViewport = viewport;

	for(int i = 0; i < optionalWidgets.size(); ++i)
	{
		if(NULL != optionalWidgets.at(i).widget)
		{
			optionalWidgets.at(i).widget->setParent(viewport);
		}
	}

	QHashIterator<QString, QtPropertyData*> i(children);
	while (i.hasNext()) 
	{
		i.next();
		i.value()->SetOWViewport(viewport);
	}
}

QVariant QtPropertyData::GetValueInternal()
{
	// should be re-implemented by sub-class

	return curValue;
}

QVariant QtPropertyData::GetValueAlias()
{
	// should be re-implemented by sub-class

	return QVariant();
}

void QtPropertyData::SetValueInternal(const QVariant &value)
{
	// should be re-implemented by sub-class

	curValue = value;
}

QWidget* QtPropertyData::CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option)
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

void QtPropertyData::ChildChanged(const QString &key, QtPropertyData *data)
{
	// should be re-implemented by sub-class
}

void QtPropertyData::ChildNeedUpdate()
{
	// should be re-implemented by sub-class
}

