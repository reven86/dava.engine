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


#include <QSortFilterProxyModel>

#include "DockSceneTree/SceneTreeDelegate.h"
#include "DockSceneTree/SceneTreeModel.h"
#include "DockSceneTree/SceneTreeItem.h"


SceneTreeDelegate::SceneTreeDelegate(QWidget *parent /* = 0 */)
	: QStyledItemDelegate(parent)
{ }

void SceneTreeDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	QStyleOptionViewItemV4 opt = option;

	initStyleOption(&opt, index);
	opt.state = opt.state & ~QStyle::State_HasFocus;
	customDraw(painter, &opt, index);

	QStyledItemDelegate::paint(painter, opt, index);
}

void SceneTreeDelegate::customDraw(QPainter *painter, QStyleOptionViewItem *option, const QModelIndex &index) const
{
	QSortFilterProxyModel *proxyModel = (QSortFilterProxyModel *)index.model();
	if(NULL != proxyModel)
	{
		SceneTreeModel *model = (SceneTreeModel *) proxyModel->sourceModel();

		if(NULL != model)
		{
			QModelIndex realIndex = proxyModel->mapToSource(index);
			QVector<QIcon> icons = model->GetCustomIcons(realIndex);

			if(icons.size() > 0)
			{
				QRect owRect = option->rect;
				owRect.setLeft(owRect.right() - 1);

				for(int i = 0; i < icons.size(); ++i)
				{
					owRect.setLeft(owRect.left() - 16);
					owRect.setRight(owRect.left() + 16);
					icons[i].paint(painter, owRect);
				}

				option->rect.setRight(owRect.left());
			}

			int flags = model->GetCustomFlags(realIndex);
			if(SceneTreeModel::CF_Invisible & flags || SceneTreeModel::CF_Disabled & flags)
			{
				// change text color
				QColor c = option->palette.text().color();
				c.setAlpha(100);
				option->palette.setColor(QPalette::Text, c);
			}
		}
	}
}
