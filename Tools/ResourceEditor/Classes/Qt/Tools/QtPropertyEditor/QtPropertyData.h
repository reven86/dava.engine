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
#include <QEvent>
#include <QToolButton>
#include <QList>

#include "QtPropertyModel.h"

// model class
class QtPropertyModel;
class QtPropertyData;
class QtPropertyDataValidator;

class QtPropertyToolButton : public QToolButton
{
	friend class QtPropertyData;

public:
    enum StateVariant
    {
        ACTIVE_ALWAYS,
        ACTIVE_WHEN_ITEM_IS_ENABLED,
        ACTIVE_WHEN_ITEM_IS_EDITABLE,
        ACTIVE_WHEN_ITEM_IS_EDITABLE_OR_ENABLED,
        ACTIVE_WHEN_ITEM_IS_EDITABLE_AND_ENABLED
    };

	QtPropertyToolButton(QtPropertyData* data, QWidget * parent = 0);
	~QtPropertyToolButton();

	QtPropertyData* GetPropertyData() const;
	virtual bool event(QEvent * event);

    void SetStateVariant(StateVariant state);
    StateVariant GetStateVariant() const;

	bool eventsPassThrought;
	bool overlayed;

protected:
	QtPropertyData* propertyData;
    StateVariant stateVariant;

    void UpdateState(bool itemIsEnabled, bool itemIsEditable);
};

// PropertyData class
class QtPropertyData : public QObject
{
	Q_OBJECT

	friend class QtPropertyModel;
	friend class QtPropertyItemDelegate;

public:
	enum ValueChangeReason
	{
		VALUE_SOURCE_CHANGED,
		VALUE_SET,
		VALUE_EDITED,
		STATE_CHANGED,
	};

	struct UserData
	{ };

	QtPropertyData();
	QtPropertyData(const QVariant &value, Qt::ItemFlags flags = (Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable));
	virtual ~QtPropertyData();

	QVariant data(int role) const;
	bool setData(const QVariant & value, int role);

	QString GetName() const;
	void SetName(const QString &name);

	QString GetPath() const;

	QVariant GetValue() const;
	void SetValue(const QVariant &value, ValueChangeReason reason = QtPropertyData::VALUE_SET);
	bool UpdateValue(bool force = false);

	QVariant GetAlias() const;

	QIcon GetIcon() const;
	void SetIcon(const QIcon &icon);

	QFont GetFont() const;
	void SetFont(const QFont &font);

	QBrush GetBackground() const;
	void SetBackground(const QBrush &brush);

	QBrush GetForeground() const;
	void SetForeground(const QBrush &brush);

	Qt::ItemFlags GetFlags() const;
	void SetFlags(Qt::ItemFlags flags);

	virtual UserData* GetUserData() const;
	virtual void SetUserData(UserData* userdata);

    virtual void SetToolTip(const QVariant& toolTip);
    virtual QVariant GetToolTip() const;

	virtual const DAVA::MetaInfo* MetaInfo() const;

	// reset background/foreground/font settings
	void ResetStyle();

	void SetCheckable(bool checkable);
	bool IsCheckable() const;
	void SetChecked(bool checked);
	bool IsChecked() const;

	void SetEditable(bool editable);
	bool IsEditable() const;

	void SetEnabled(bool enabled);
	bool IsEnabled() const;

	QtPropertyModel* GetModel() const;
    
    QtPropertyDataValidator* GetValidator() const;
    void SetValidator(QtPropertyDataValidator*);

	// editor
	QWidget* CreateEditor(QWidget *parent, const QStyleOptionViewItem& option) const;
	bool EditorDone(QWidget *editor);
	bool SetEditorData(QWidget *editor);

	// childs
	QtPropertyData *Parent() const;
	void ChildAdd(const QString &key, QtPropertyData *data);
	void ChildAdd(const QString &key, const QVariant &value);
	void ChildInsert(const QString &key, QtPropertyData *data, int pos);
	void ChildInsert(const QString &key, const QVariant &value, int pos);
	int ChildCount() const;
	QtPropertyData* ChildGet(int i) const;
	QtPropertyData* ChildGet(const QString &key) const;
	int ChildIndex(QtPropertyData *data) const;
	void ChildExtract(QtPropertyData *data);
	void ChildRemove(QtPropertyData *data);
	void ChildRemove(const QString &key);
	void ChildRemove(int i);
	void ChildRemoveAll();

	// Optional widgets
	int GetButtonsCount() const;
	QtPropertyToolButton* GetButton(int index = 0);
	QtPropertyToolButton* AddButton(QtPropertyToolButton::StateVariant stateVariant = QtPropertyToolButton::ACTIVE_ALWAYS);
	void RemButton(int index);
	void RemButton(QtPropertyToolButton *button);

	void EmitDataChanged(ValueChangeReason reason);

	// edit command
	virtual void* CreateLastCommand() const;

    // Merging
    bool IsMergedDataEqual() const;
    QtPropertyData * GetMergedData(int idx) const;
    int GetMergedCount() const;
    void Merge(QtPropertyData *data);
    void MergeChild(QtPropertyData *data, const QString& key = QString());
    virtual bool IsMergable() const;

protected:
	mutable QVariant curValue;
    mutable bool isValuesMerged;

	QString curName;
	Qt::ItemFlags curFlags;

	QMap<int, QVariant> style;
	bool updatingValue;

	QtPropertyModel *model;
	QtPropertyData *parent;
	UserData* userData;

	QList<QString> childrenNames;
	QList<QtPropertyData*> childrenData;
    QList<QtPropertyData*> mergedData;
	
	QWidget *optionalButtonsViewport;
	QVector<QtPropertyToolButton *> optionalButtons;
    
    QtPropertyDataValidator* validator;
    
    QVariant tooltipValue;

	void SetModel(QtPropertyModel *model);
    void SetColorButtonIcon(const QIcon &icon);

    void BuildCurrentValue();
    void SetTempValue(const QVariant &value);

	virtual void UpdateUp();
	virtual void UpdateDown();

	// Functions should be re-implemented by sub-class
	virtual QVariant GetValueInternal() const;
	virtual QVariant GetValueAlias() const;
	virtual void SetValueInternal(const QVariant &value);
    virtual void SetTempValueInternal(const QVariant &value);
	virtual bool UpdateValueInternal();
	virtual QWidget* CreateEditorInternal(QWidget *parent, const QStyleOptionViewItem& option) const;
	virtual bool EditorDoneInternal(QWidget *editor);
	virtual bool SetEditorDataInternal(QWidget *editor);

	// viewport, where optional toolbuttons should be drawn
	QWidget* GetOWViewport() const;
	void SetOWViewport(QWidget *viewport);

    // optional widgets state update
    void UpdateOWState();

	void ChildRemoveInternal(int i, bool del);
};

#endif // __QT_PROPERTY_DATA_H__
