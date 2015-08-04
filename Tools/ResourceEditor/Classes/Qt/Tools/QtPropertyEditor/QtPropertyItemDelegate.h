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


#ifndef __QT_PROPERY_ITEM_DELEGATE_H__
#define __QT_PROPERY_ITEM_DELEGATE_H__

#include <QStyledItemDelegate>
#include <QPointer>
#include <QWidget>
#include <QAbstractItemView>
#include "QtPropertyData.h"

class QtPropertyData;
class QtPropertyModel;

class QtPropertyItemDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
	QtPropertyItemDelegate(QAbstractItemView *view, QtPropertyModel *model, QWidget *parent = 0);
	virtual ~QtPropertyItemDelegate();

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
	bool editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index);
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
	void updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const;

public slots:
	bool helpEvent(QHelpEvent * event, QAbstractItemView * view, const QStyleOptionViewItem & option, const QModelIndex & index);
    void showButtons(QtPropertyData *data);
	void invalidateButtons();

private:
    bool eventFilter(QObject *obj, QEvent *event);
	void drawOptionalButtons(QPainter *painter, QStyleOptionViewItem &option, const QModelIndex &index) const;
	void showOptionalButtons(QtPropertyData *data, bool show);
    void DrawButton(QPainter* painter, QStyleOptionViewItem& opt, QtPropertyToolButton* btn) const;

	QtPropertyModel *model;
	QPointer<QtPropertyData> lastHoverData;
    QPointer<QAbstractItemView> view;
    mutable QPointer<QWidget> activeEditor;
    mutable bool editorDataWasSet;
    const int buttonSpacing = 1;
};

#endif // __QT_PROPERY_ITEM_DELEGATE_H__
