#include <QSortFilterProxyModel>

#include "DockSceneTree/SceneTreeDelegate.h"
#include "DockSceneTree/SceneTreeModel.h"
#include "DockSceneTree/SceneTreeItem.h"

SceneTreeDelegate::SceneTreeDelegate(QWidget* parent /* = 0 */)
    : QStyledItemDelegate(parent)
{
}

void SceneTreeDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItemV4 opt = option;

    initStyleOption(&opt, index);
    opt.state = opt.state & ~QStyle::State_HasFocus;
    customDraw(painter, &opt, index);

    QStyledItemDelegate::paint(painter, opt, index);
}

void SceneTreeDelegate::customDraw(QPainter* painter, QStyleOptionViewItem* option, const QModelIndex& index) const
{
    QSortFilterProxyModel* proxyModel = (QSortFilterProxyModel*)index.model();
    if (NULL != proxyModel)
    {
        SceneTreeModel* model = (SceneTreeModel*)proxyModel->sourceModel();

        if (NULL != model)
        {
            QModelIndex realIndex = proxyModel->mapToSource(index);
            QVector<QIcon> icons = model->GetCustomIcons(realIndex);

            if (icons.size() > 0)
            {
                QRect owRect = option->rect;
                owRect.setLeft(owRect.right() - 1);

                for (int i = 0; i < icons.size(); ++i)
                {
                    owRect.setLeft(owRect.left() - 16);
                    owRect.setRight(owRect.left() + 16);
                    icons[i].paint(painter, owRect);
                }

                option->rect.setRight(owRect.left());
            }

            int flags = model->GetCustomFlags(realIndex);
            if (SceneTreeModel::CF_Invisible & flags || SceneTreeModel::CF_Disabled & flags)
            {
                // change text color
                QColor c = option->palette.text().color();
                c.setAlpha(100);
                option->palette.setColor(QPalette::Text, c);
            }
        }
    }
}
