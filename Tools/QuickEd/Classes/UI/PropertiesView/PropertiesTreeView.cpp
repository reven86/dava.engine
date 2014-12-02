#include "PropertiesTreeView.h"
#include <QPainter>
#include <QHeaderView>

PropertiesTreeView::PropertiesTreeView( QWidget *parent /*= NULL*/ )
{

}

PropertiesTreeView::~PropertiesTreeView()
{

}

void PropertiesTreeView::drawRow(QPainter * painter, const QStyleOptionViewItem &option, const QModelIndex & index) const
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

            QPoint p1 = option.rect.topLeft();
            QPoint p2 = option.rect.bottomLeft();

            p1.setX(p1.x() + sz - 1);
            p2.setX(p2.x() + sz - 1);

            painter->setPen(gridColor);
            painter->drawLine(p1, p2);
        }
    }
}
