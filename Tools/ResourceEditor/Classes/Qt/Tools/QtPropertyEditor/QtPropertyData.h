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



#ifndef __QT_PROPERTY_DATA_H__
#define __QT_PROPERTY_DATA_H__

#include <QStyledItemDelegate>
#include <QHash>
#include <QIcon>

// Optional widget
struct QtPropertyOW
{
	QtPropertyOW() : widget(NULL), overlay(false), size(0, 0)
	{ }

	QtPropertyOW(QWidget *_widget, bool _overlay = false, QSize _size = QSize(16, 16))
		: widget(_widget), overlay(_overlay), size(_size)
	{ }

	QWidget *widget;
	QSize size;
	bool overlay;
};

// PropertyData class
class QtPropertyData : public QObject
{
	Q_OBJECT

	friend class QtPropertyItem;

public:
	enum
	{
		FLAG_EMPTY				= 0x0,

		FLAG_IS_DISABLED		= 0x1,
		FLAG_IS_CHECKABLE		= 0x2,
		FLAG_IS_NOT_EDITABLE	= 0x4,
	};

	enum ValueChangeReason
	{
		VALUE_SOURCE_CHANGED,
		VALUE_SET,
		VALUE_EDITED
	};

	QtPropertyData();
	QtPropertyData(const QVariant &value);
	virtual ~QtPropertyData() ;

	QVariant GetValue();
	void SetValue(const QVariant &value, ValueChangeReason reason = QtPropertyData::VALUE_SET);
	bool UpdateValue();

	QVariant GetAlias();

	virtual QIcon GetIcon();
	virtual void SetIcon(const QIcon &icon);

	int GetFlags();
	void SetFlags(int flags);

	QWidget* CreateEditor(QWidget *parent, const QStyleOptionViewItem& option);
	bool EditorDone(QWidget *editor);
	bool SetEditorData(QWidget *editor);

	void ChildAdd(const QString &key, QtPropertyData *data);
	void ChildAdd(const QString &key, const QVariant &value);
	int ChildCount();
	QtPropertyData* ChildGet(const QString &key);
	QPair<QString, QtPropertyData*> ChildGet(int i);
	void ChildRemove(const QString &key);
	void ChildRemove(QtPropertyData *data);
	void ChildRemove(int i);

	virtual void* CreateLastCommand() const;

signals:
	void ValueChanged(QtPropertyData::ValueChangeReason reason);
	void FlagsChanged();
	void ChildAdded(const QString &key, QtPropertyData *data);
	void ChildRemoving(const QString &key, QtPropertyData *data);

protected:
	QVariant curValue;
	QIcon curIcon;
	int curFlags;
	bool updatingValue;
	
	QtPropertyData *parent;

	QList<QString> childrenNames;
	QList<QtPropertyData*> childrenData;

	virtual void UpdateUp();
	virtual void UpdateDown();

	// Functions should be re-implemented by sub-class
	virtual QVariant GetValueInternal();
	virtual QVariant GetValueAlias();
	virtual void SetValueInternal(const QVariant &value);
	virtual bool UpdateValueInternal();
	virtual QWidget* CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option);
	virtual bool EditorDoneInternal(QWidget *editor);
	virtual bool SetEditorDataInternal(QWidget *editor);

public:
	// Option widgets
	int GetOWCount();
	const QtPropertyOW* GetOW(int index = 0);
	void AddOW(const QtPropertyOW &ow);
	void RemOW(int index);
	void RemOW(QWidget *widget);

	QWidget* GetOWViewport();
	void SetOWViewport(QWidget *viewport);

private:
	// Optional widgets data struct and memebers
	QVector<QtPropertyOW> optionalWidgets;
	QWidget *optionalWidgetViewport;
};

#endif // __QT_PROPERTY_DATA_H__
