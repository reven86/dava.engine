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


#ifndef __QT_PROPERTY_VIEW_H__
#define __QT_PROPERTY_VIEW_H__

#include <QTreeView>
#include <QTimer>
#include "QtPropertyData.h"
#include "QtPropertyModel.h"

class QtPropertyItemDelegate;

class QtPropertyEditor : public QTreeView
{
	Q_OBJECT

public:
	enum Style
	{
		DEFAULT_STYLE = 0,
		HEADER_STYLE,

		USER_STYLE
	};

	QtPropertyEditor(QWidget *parent = 0);
	~QtPropertyEditor();

	QModelIndex AppendProperty(const QString &name, QtPropertyData* data, const QModelIndex &parent = QModelIndex());
    void MergeProperty(QtPropertyData* data, const QModelIndex &parent = QModelIndex());
	QModelIndex InsertProperty(const QString &name, QtPropertyData* data, int row, const QModelIndex &parent = QModelIndex());
	QModelIndex AppendHeader(const QString &text);
	QModelIndex InsertHeader(const QString &text, int row);

	QtPropertyData * GetProperty(const QModelIndex &index) const;
	QtPropertyData * GetRootProperty() const;
	
	bool GetEditTracking() const;
	void SetEditTracking(bool enabled);

	void RemoveProperty(const QModelIndex &index);
	void RemoveProperty(QtPropertyData* data);
	void RemovePropertyAll();

	void SetUpdateTimeout(int ms);
	int GetUpdateTimeout();

	virtual void ApplyStyle(QtPropertyData *data, int style);

public slots:
	void Update();

	void OnExpanded(const QModelIndex & index);
	void OnCollapsed(const QModelIndex & index);

signals:
	void PropertyEdited(const QModelIndex &index);

protected:
	QtPropertyModel *curModel;
	QtPropertyItemDelegate *curItemDelegate;
	
	int updateTimeout;
	QTimer updateTimer;
	bool doUpdateOnPaintEvent;

	void leaveEvent(QEvent * event) override;
    void drawRow(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const override;

protected slots:
	virtual void OnItemClicked(const QModelIndex &);
	virtual void OnItemEdited(const QModelIndex &);
	virtual void OnUpdateTimeout();

	virtual void rowsAboutToBeInserted(const QModelIndex & parent, int start, int end);
	virtual void rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end);
	virtual void rowsOp(const QModelIndex & parent, int start, int end);

private:
	void ShowButtonsUnderCursor();
};

#endif // __QT_PROPERTY_VIEW_H__
