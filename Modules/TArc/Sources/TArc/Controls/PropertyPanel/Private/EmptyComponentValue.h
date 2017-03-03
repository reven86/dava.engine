#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include <Reflection/Reflection.h>

namespace DAVA
{
namespace TArc
{
class EmptyComponentValue : public BaseComponentValue
{
protected:
    Any GetMultipleValue() const override;
    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override;
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const override;

private:
    DAVA_VIRTUAL_REFLECTION(EmptyComponentValue, BaseComponentValue);
};
}
} // namespace DAVA