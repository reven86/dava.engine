#include "TArc/Controls/PropertyPanel/TextEditorDrawer.h"

#include <QStyle>
#include <QStyleOption>

namespace DAVA
{
namespace TArc
{
void TextEditorDrawer::Draw(QStyle* style, QPainter* painter, const QStyleOptionViewItem& options, const Any& value)
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
