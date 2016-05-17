#include "PropertiesTreeView.h"
#include <QPainter>
#include <QHeaderView>
#include <QApplication>

PropertiesTreeView::PropertiesTreeView(QWidget* parent /*= NULL*/)
    : QTreeView(parent)
{
}

PropertiesTreeView::~PropertiesTreeView()
{
}

void PropertiesTreeView::drawRow(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyleOptionViewItemV3 opt = option;

    QTreeView::drawRow(painter, opt, index);
    QColor color = static_cast<QRgb>(QApplication::style()->styleHint(QStyle::SH_Table_GridLineColor, &opt));
    painter->save();
    painter->setPen(QPen(color));
    painter->drawLine(opt.rect.x(), opt.rect.bottom(), opt.rect.right(), opt.rect.bottom());
    painter->restore();
}
