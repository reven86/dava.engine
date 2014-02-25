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



#include <QtGui>

#include "QtPropertyItemDelegate.h"
#include "QtPropertyModel.h"
#include "QtPropertyItem.h"
#include "QtPropertyWidgets/QtColorLineEdit.h"

QtPropertyItemDelegate::QtPropertyItemDelegate(QWidget *parent /* = 0 */)
	: QStyledItemDelegate(parent)
{
	childWidgetsStyle = new QWindowsStyle();
}

QtPropertyItemDelegate::~QtPropertyItemDelegate()
{
	delete childWidgetsStyle;
}

void QtPropertyItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItemV4 opt = option;
	initStyleOption(&opt, index);

	if(index.column() == 1)
	{
		opt.textElideMode = Qt::ElideLeft;
	}

	recalcOptionalWidgets(index, &opt);

	QStyledItemDelegate::paint(painter, opt, index);
}

QSize QtPropertyItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QSize s = QStyledItemDelegate::sizeHint(option, index);
    return QSize(s.width(), s.height() + 5);
}

QWidget* QtPropertyItemDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QWidget* editWidget = NULL;
	QtPropertyData* data = index.data(QtPropertyItem::PropertyDataRole).value<QtPropertyData*>();
	
	recalcOptionalWidgets(index, (QStyleOptionViewItem *) &option);

	if(NULL != data)
	{
		editWidget = data->CreateEditor(parent, option);
	}

	if(NULL == editWidget)
	{
		editWidget = QStyledItemDelegate::createEditor(parent, option, index);
	}

    return editWidget;
}

bool QtPropertyItemDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index)
{
	bool ret = QStyledItemDelegate::editorEvent(event, model, option, index);

	const QtPropertyModel *propertyModel = dynamic_cast<const QtPropertyModel *>(index.model());
	if(NULL != propertyModel)
	{
		QtPropertyItem* item = (QtPropertyItem*) propertyModel->itemFromIndex(index);
		QtPropertyData* data = item->GetPropertyData();

		if(NULL != data && NULL != item && item->isCheckable())
		{
			data->SetValue((item->checkState() == Qt::Checked), QtPropertyData::VALUE_EDITED);
		}
	}

	return ret;
}

void QtPropertyItemDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
	bool doneByInternalEditor = false;

    const QtPropertyModel *propertyModel = dynamic_cast<const QtPropertyModel *>(index.model());
	if(NULL != propertyModel)
	{
		QtPropertyItem* item = (QtPropertyItem*) propertyModel->itemFromIndex(index);
		QtPropertyData* data = item->GetPropertyData();
		if(NULL != data)
		{
            doneByInternalEditor = data->SetEditorData(editor);
		}
	}

	if(!doneByInternalEditor)
	{
		QStyledItemDelegate::setEditorData(editor, index);
	}
}

void QtPropertyItemDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
	bool doneByInternalEditor = false;

    const QtPropertyModel *propertyModel = dynamic_cast<const QtPropertyModel *>(index.model());
	if(NULL != propertyModel)
	{
		QtPropertyItem* item = (QtPropertyItem*) propertyModel->itemFromIndex(index);
		QtPropertyData* data = item->GetPropertyData();
		if(NULL != data)
		{
            doneByInternalEditor = data->EditorDone(editor);
		}
	}

	if(!doneByInternalEditor)
	{
	    QStyledItemDelegate::setModelData(editor, model, index);
	}
}

void QtPropertyItemDelegate::updateEditorGeometry(QWidget * editor, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
	QStyledItemDelegate::updateEditorGeometry(editor, option, index);

	// tune widget border and geometry
	if(NULL != editor)
	{
		editor->setObjectName("customPropertyEditor");
		editor->setStyleSheet("#customPropertyEditor{ border: 1px solid gray; }");
		QRect r = option.rect;
		//r.adjust(2, -1, 0, 1);

		if(!index.model()->data(index, Qt::DecorationRole).isNull())
		{
			r.adjust(17, 0, 0, 0);
		}

		editor->setGeometry(r);
	}
}

bool QtPropertyItemDelegate::helpEvent(QHelpEvent * event, QAbstractItemView * view, const QStyleOptionViewItem & option, const QModelIndex & index)
{
	bool showToolTip = true;

	if(NULL != event && NULL != view && event->type() == QEvent::ToolTip)
	{
		QRect rect = view->visualRect(index);
		QSize size = sizeHint(option, index);
		if(rect.width() >= size.width()) 
		{
			showToolTip = false;
		}
	}

	if(showToolTip)
	{
		return QStyledItemDelegate::helpEvent(event, view, option, index);
	}
	else
	{
		QToolTip::hideText();
	}

	return false;
}

void QtPropertyItemDelegate::recalcOptionalWidgets(const QModelIndex &index, QStyleOptionViewItem *option) const
{
	QtPropertyData* data = index.data(QtPropertyItem::PropertyDataRole).value<QtPropertyData*>();

	if(NULL != data)
	{
		QWidget *owViewport = data->GetOWViewport();

		int prevOWSpace = 0;
		int owSpacing = 1;
		int optionRectRight = option->rect.right();

		int owCount = data->GetOWCount();
		for (int i = (owCount - 1); i >= 0; --i)
		{
			const QtPropertyOW *ow = data->GetOW(i);
			if(NULL != ow && NULL != owViewport && NULL != ow->widget)
			{
				QWidget *owWidget = ow->widget;
				int owWidth = ow->size.width();

				if(0 != owWidth)
				{
					QRect owRect = option->rect;
					owRect.setLeft(optionRectRight - owWidth - prevOWSpace);
					owRect.setRight(owRect.left() + owWidth);

					if(owWidget->style() != childWidgetsStyle)
					{
						owWidget->setStyle(childWidgetsStyle);
					}

					owWidget->setGeometry(owRect);
					owWidget->show();

					// if this widget isn't overlayed we should modify rect for tree view cell to be drawn in.
					if(!ow->overlay)
					{
						option->rect.setRight(owRect.left());
					}

					prevOWSpace += (owWidth + owSpacing);
				}
				else
				{
					owWidget->hide();
				}
			}
		}
	}
}

void QtPropertyItemDelegate::collapse(const QModelIndex & collapse_index)
{
	// hide all optional widgets from child
	// they will be shown after expanding them and on first paint call
	if(collapse_index.isValid())
	{
		const QAbstractItemModel *model = collapse_index.model();

		if(NULL != model)
		{
			// go thought columns and hide OW in each cell PropertyData
			for (int c = 0; c < model->columnCount(); c++)
			{
				QModelIndex cellIndex = model->index(collapse_index.row(), c, collapse_index.parent());
				QtPropertyData* cellData = cellIndex.data(QtPropertyItem::PropertyDataRole).value<QtPropertyData*>();
				if(NULL != cellData)
				{
					hideAllChildOptionalWidgets(cellData);
				}
			}
		}
	}
}

void QtPropertyItemDelegate::expand(const QModelIndex & index)
{ }

void QtPropertyItemDelegate::hideAllChildOptionalWidgets(QtPropertyData* data)
{
	for(int i = 0; i < data->ChildCount(); i++)
	{
		QPair<QString, QtPropertyData *> childPair = data->ChildGet(i);
		QtPropertyData *childData = childPair.second;

		for (int j = 0; j < childData->GetOWCount(); j++)
		{
			childData->GetOW(j)->widget->hide();
		}

		hideAllChildOptionalWidgets(childData);
	}
}