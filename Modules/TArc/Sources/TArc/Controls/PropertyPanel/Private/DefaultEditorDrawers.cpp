#include "TArc/Controls/PropertyPanel/DefaultEditorDrawers.h"

#include <QStyle>
#include <QStyleOption>
#include <QPainter>

namespace DAVA
{
namespace TArc
{
uint32 EmptyEditorDrawer::GetHeight(QStyle* style, const QStyleOptionViewItem& options, const Any& value) const
{
    return style->sizeFromContents(QStyle::CT_ItemViewItem, &options, QSize(), options.widget).height();
}

void EmptyEditorDrawer::Draw(QStyle* style, QPainter* painter, const QStyleOptionViewItem& options, const Any& value) const
{
    painter->fillRect(options.rect, Qt::red);
}

uint32 TextEditorDrawer::GetHeight(QStyle* style, const QStyleOptionViewItem& options, const Any& value) const
{
    QStyleOptionViewItem opt = options;
    opt.text = value.Cast<QString>(QString());
    if (opt.text.isEmpty())
    {
        opt.text = QString(value.Cast<String>().c_str());
    }
    return style->sizeFromContents(QStyle::CT_ItemViewItem, &opt, QSize(), opt.widget).height();
}

void TextEditorDrawer::Draw(QStyle* style, QPainter* painter, const QStyleOptionViewItem& options, const Any& value) const
{
    QStyleOptionViewItem opt = options;
    opt.text = value.Cast<QString>(QString());
    if (opt.text.isEmpty())
    {
        opt.text = QString(value.Cast<String>().c_str());
    }
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);
}

} // namespace TArc
} // namespace DAVA
