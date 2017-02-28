#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include <Reflection/Reflection.h>

namespace DAVA
{
namespace TArc
{
class EnumComponentValue : public BaseComponentValue
{
public:
    EnumComponentValue() = default;

protected:
    Any GetMultipleValue() const override;
    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override;
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const override;

private:
    Any GetValueAny() const;
    void SetValueAny(const Any& newValue);

private:
    DAVA_VIRTUAL_REFLECTION(EnumComponentValue, BaseComponentValue);
};
}
}
