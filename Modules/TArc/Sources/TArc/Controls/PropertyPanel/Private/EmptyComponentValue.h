#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
namespace TArc
{
class EmptyComponentValue : public BaseComponentValue
{
public:
    QWidget* AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    void ReleaseEditorWidget(QWidget* editor, const QModelIndex& index) override;
    void StaticEditorPaint(QStyle* style, QPainter* painter, const QStyleOptionViewItem& options) override;

private:
    DAVA_VIRTUAL_REFLECTION(EmptyComponentValue, BaseComponentValue);
};
}
} // namespace DAVA