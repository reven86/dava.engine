#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include <Reflection/Reflection.h>

namespace DAVA
{
namespace TArc
{
template <typename T>
class NumberComponentValue : public BaseComponentValue
{
public:
    NumberComponentValue() = default;

    bool EditorEvent(QWidget* parent, QEvent* event, const QStyleOptionViewItem& option) override;

protected:
    Any GetMultipleValue() const override;
    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override;
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const override;

private:
    Any GetNumberValue() const;
    void SetNumberValue(Any v);

    DAVA_VIRTUAL_REFLECTION(NumberComponentValue<T>, BaseComponentValue);
};
} // namespace TArc
} // namespace DAVA