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

protected:
    Any GetMultipleValue() const override;
    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override;
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) override;

private:
    Any GetNumberValue() const;
    void SetNumberValue(const Any& v);

    DAVA_VIRTUAL_REFLECTION(NumberComponentValue<T>, BaseComponentValue);
};
} // namespace TArc
} // namespace DAVA