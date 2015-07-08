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


#include <QMouseEvent>
#include <QHeaderView>
#include <QPainter>
#include <QApplication>
#include <QScrollBar>

#include "QtPropertyEditor.h"
#include "QtPropertyModel.h"
#include "QtPropertyItemDelegate.h"

QtPropertyEditor::QtPropertyEditor(QWidget *parent /* = 0 */)
: QTreeView(parent)
, updateTimeout(0)
, doUpdateOnPaintEvent(false)
{
	curModel = new QtPropertyModel(viewport());
	setModel(curModel);

	curItemDelegate = new QtPropertyItemDelegate(this, curModel);
	setItemDelegate(curItemDelegate);

	QObject::connect(this, SIGNAL(clicked(const QModelIndex &)), this, SLOT(OnItemClicked(const QModelIndex &)));
	QObject::connect(this, SIGNAL(expanded(const QModelIndex &)), this, SLOT(OnExpanded(const QModelIndex &)));
	QObject::connect(this, SIGNAL(collapsed(const QModelIndex &)), this, SLOT(OnCollapsed(const QModelIndex &)));
	QObject::connect(curModel, SIGNAL(PropertyEdited(const QModelIndex &)), this, SLOT(OnItemEdited(const QModelIndex &)));
	QObject::connect(curModel, SIGNAL(rowsAboutToBeInserted(const QModelIndex &, int, int)), this, SLOT(rowsAboutToBeInserted(const QModelIndex &, int, int)));
	QObject::connect(curModel, SIGNAL(rowsAboutToBeRemoved(const QModelIndex &, int, int)), this, SLOT(rowsAboutToBeRemoved(const QModelIndex &, int, int)));
	QObject::connect(curModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(rowsOp(const QModelIndex &, int, int)));
	QObject::connect(curModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(rowsOp(const QModelIndex &, int, int)));
	QObject::connect(&updateTimer, SIGNAL(timeout()), this, SLOT(OnUpdateTimeout()));

	setMouseTracking(true);
}

QtPropertyEditor::~QtPropertyEditor()
{ }

QModelIndex QtPropertyEditor::AppendProperty(const QString &name, QtPropertyData* data, const QModelIndex &parent)
{
	return curModel->AppendProperty(name, data, parent);
}

void QtPropertyEditor::MergeProperty(QtPropertyData* data, QModelIndex const& parent)
{
    curModel->MergeProperty(data, parent);
}

QModelIndex QtPropertyEditor::InsertProperty(const QString &name, QtPropertyData* data, int row, const QModelIndex &parent)
{
	return curModel->InsertProperty(name, data, row, parent);
}

QModelIndex QtPropertyEditor::AppendHeader(const QString &text)
{
	QModelIndex propHeader = AppendProperty(text, new QtPropertyData(""));

	ApplyStyle(GetProperty(propHeader), HEADER_STYLE);
	return propHeader;
}

QModelIndex QtPropertyEditor::InsertHeader(const QString &text, int row)
{
	QModelIndex propHeader = InsertProperty(text, new QtPropertyData(""), row);

	ApplyStyle(GetProperty(propHeader), HEADER_STYLE);
	return propHeader;
}

QtPropertyData* QtPropertyEditor::GetProperty(const QModelIndex &index) const
{
	return curModel->itemFromIndex(index);
}

QtPropertyData * QtPropertyEditor::GetRootProperty() const
{
	return curModel->rootItem();
}

void QtPropertyEditor::RemoveProperty(const QModelIndex &index)
{
	curModel->RemoveProperty(index);
}

void QtPropertyEditor::RemoveProperty(QtPropertyData *data)
{
	curModel->RemoveProperty(curModel->indexFromItem(data));
}

void QtPropertyEditor::RemovePropertyAll()
{
	curModel->RemovePropertyAll();
}

void QtPropertyEditor::SetEditTracking(bool enabled)
{
	curModel->SetEditTracking(enabled);
}

bool QtPropertyEditor::GetEditTracking() const
{
	return curModel->GetEditTracking();
}

void QtPropertyEditor::SetUpdateTimeout(int ms)
{
	updateTimeout = ms;

	if(0 != updateTimeout)
	{
		updateTimer.start(updateTimeout);
	}
	else
	{
		updateTimer.stop();
	}
}

int QtPropertyEditor::GetUpdateTimeout()
{
	return updateTimeout;
}

void QtPropertyEditor::Update()
{
	OnUpdateTimeout();
}

void QtPropertyEditor::OnUpdateTimeout()
{
	if(state() != QAbstractItemView::EditingState)
	{
		doUpdateOnPaintEvent = false;
		curModel->UpdateStructure();
	}

	SetUpdateTimeout(updateTimeout);
}

void QtPropertyEditor::OnExpanded(const QModelIndex & index)
{
	ShowButtonsUnderCursor();
}

void QtPropertyEditor::OnCollapsed(const QModelIndex & index)
{
	ShowButtonsUnderCursor();
}

void QtPropertyEditor::drawRow(QPainter * painter, const QStyleOptionViewItem &option, const QModelIndex & index) const
{
	QColor gridColor = option.palette.color(QPalette::Normal, QPalette::Window);

	// draw horizontal bottom line
	painter->setPen(gridColor);
	painter->drawLine(option.rect.bottomLeft(), option.rect.bottomRight());

	// adjust rect, so that grid line wont be overdrawn
	QStyleOptionViewItemV4 opt = option;
	opt.rect.adjust(0, 0, 0, -1);

	// draw row
	QTreeView::drawRow(painter, opt, index);

	// draw vertical line
	if(!(option.state & QStyle::State_Selected))
	{
		QHeaderView *hdr = header();
		if(NULL != hdr && hdr->count() > 1)
		{
            int sz = hdr->sectionSize(0);
            QScrollBar *scroll = horizontalScrollBar();
            if (scroll != NULL)
            {
                sz -= scroll->value();
            }

			QPoint p1 = option.rect.topLeft();
			QPoint p2 = option.rect.bottomLeft();

			p1.setX(p1.x() + sz - 1);
			p2.setX(p2.x() + sz - 1);

			painter->setPen(gridColor);
			painter->drawLine(p1, p2);
		}
	}
}

void QtPropertyEditor::ApplyStyle(QtPropertyData *data, int style)
{
	if(NULL != data)
	{
		switch(style)
		{
		case DEFAULT_STYLE:
			data->ResetStyle();
			break;

		case HEADER_STYLE:
			{
				QFont boldFont = data->GetFont();
				boldFont.setBold(true);
				data->SetFont(boldFont);
				data->SetBackground(QBrush(QColor(Qt::lightGray)));
                                data->SetEditable(false);
			}
			break;

		default:
			break;
		}
	}
}

void QtPropertyEditor::leaveEvent(QEvent * event)
{
	curItemDelegate->invalidateButtons();
}

void QtPropertyEditor::OnItemClicked(const QModelIndex &index)
{
	edit(index, QAbstractItemView::DoubleClicked, NULL);
}

void QtPropertyEditor::OnItemEdited(const QModelIndex &index)
{
	emit PropertyEdited(index);
}

void QtPropertyEditor::rowsAboutToBeInserted(const QModelIndex & parent, int start, int end)
{
	curItemDelegate->invalidateButtons();
}

void QtPropertyEditor::rowsAboutToBeRemoved(const QModelIndex & parent, int start, int end)
{
	curItemDelegate->invalidateButtons();
}

void QtPropertyEditor::rowsOp(const QModelIndex & parent, int start, int end)
{
	ShowButtonsUnderCursor();
}

void QtPropertyEditor::ShowButtonsUnderCursor()
{
	QPoint pos = viewport()->mapFromGlobal(QCursor::pos());
	curItemDelegate->showButtons(GetProperty(indexAt(pos)));
}
