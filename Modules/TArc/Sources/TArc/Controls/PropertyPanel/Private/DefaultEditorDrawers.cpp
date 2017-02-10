#include "TArc/Controls/PropertyPanel/DefaultEditorDrawers.h"
#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"

#include <Reflection/ReflectedMeta.h>

#include <QStyle>
#include <QStyleOption>
#include <QStyleOptionComboBox>
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

void EnumEditorDrawer::InitStyleOptions(Params& params) const
{
    params.options.text = GetTextHint(params.value, params.nodes);
}

uint32 EnumEditorDrawer::GetHeight(Params params) const
{
    InitStyleOptions(params);
    return params.style->sizeFromContents(QStyle::CT_ComboBox, &params.options, QSize(), params.options.widget).height();
}

void EnumEditorDrawer::Draw(QPainter* painter, Params params) const
{
    InitStyleOptions(params);

    QStyleOptionComboBox box;
    box.rect = params.options.rect;
    box.currentText = params.options.text;

    params.style->drawComplexControl(QStyle::CC_ComboBox, &box, painter, params.options.widget);
    params.style->drawControl(QStyle::CE_ComboBoxLabel, &box, painter, params.options.widget);
}

QString EnumEditorDrawer::GetTextHint(const Any& value, const Vector<std::shared_ptr<PropertyNode>>* nodes) const
{
    QString result;
    if (value.IsEmpty() == false)
    {
        const M::Enum* enumMeta = nodes->front()->field.ref.GetMeta<M::Enum>();
        if (enumMeta != nullptr)
        {
            int intValue = value.Cast<int>();

            const EnumMap* enumMap = enumMeta->GetEnumMap();
            result = enumMap->ToString(intValue);
        }
    }

    return result;
}

} // namespace TArc
} // namespace DAVA
