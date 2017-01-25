#include "TArc/Controls/PropertyPanel/DefaultEditorDrawers.h"

#include <QStyle>
#include <QStyleOption>
#include <QPainter>

namespace DAVA
{
namespace TArc
{
namespace EditorDrawerDetails
{
QString trueString("true");
QString falseString("false");
QString multipleString("<multiple values>");
}

void EmptyEditorDrawer::InitStyleOptions(Params& params) const
{
}

uint32 EmptyEditorDrawer::GetHeight(Params params) const
{
    return params.style->sizeFromContents(QStyle::CT_ItemViewItem, &params.options, QSize(), params.options.widget).height();
}

void EmptyEditorDrawer::Draw(QPainter* painter, Params params) const
{
    painter->fillRect(params.options.rect, Qt::red);
}

void TextEditorDrawer::InitStyleOptions(Params& params) const
{
    params.options.text = params.value.Cast<QString>(QString());
}

uint32 TextEditorDrawer::GetHeight(Params params) const
{
    InitStyleOptions(params);
    return params.style->sizeFromContents(QStyle::CT_ItemViewItem, &params.options, QSize(), params.options.widget).height();
}

void TextEditorDrawer::Draw(QPainter* painter, Params params) const
{
    InitStyleOptions(params);
    params.style->drawControl(QStyle::CE_ItemViewItem, &params.options, painter, params.options.widget);
}

void BoolEditorDrawer::InitStyleOptions(Params& params) const
{
    params.options.checkState = params.value.Cast<Qt::CheckState>();
    params.options.features |= QStyleOptionViewItem::HasCheckIndicator;
    params.options.features |= QStyleOptionViewItem::HasDisplay;
    params.options.text = GetTextHint(params.value, params.nodes);
}

uint32 BoolEditorDrawer::GetHeight(Params params) const
{
    InitStyleOptions(params);
    return params.style->sizeFromContents(QStyle::CT_ItemViewItem, &params.options, QSize(), params.options.widget).height();
}

void BoolEditorDrawer::Draw(QPainter* painter, Params params) const
{
    InitStyleOptions(params);
    params.style->drawControl(QStyle::CE_ItemViewItem, &params.options, painter, params.options.widget);
}

QString BoolEditorDrawer::GetTextHint(const Any& value, const Vector<std::shared_ptr<PropertyNode>>* nodes) const
{
    QString result;
    Qt::CheckState state = value.Cast<Qt::CheckState>();
    if (state == Qt::PartiallyChecked)
    {
        result = EditorDrawerDetails::multipleString;
    }
    else
    {
        const M::ValueDescription* description = nodes->front()->field.ref.GetMeta<M::ValueDescription>();
        if (description != nullptr)
        {
            result = QString::fromStdString(description->GetDescription(value));
        }
        else
        {
            result = (state == Qt::Checked) ? EditorDrawerDetails::trueString : EditorDrawerDetails::falseString;
        }
    }

    return result;
}

} // namespace TArc
} // namespace DAVA
