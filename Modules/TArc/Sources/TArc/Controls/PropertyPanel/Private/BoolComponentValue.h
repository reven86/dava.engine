#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

namespace DAVA
{
namespace TArc
{
class BoolComponentValue : public BaseComponentValue
{
public:
    BoolComponentValue() = default;

protected:
    Any GetMultipleValue() const;
    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const;
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const;

private:
    Qt::CheckState GetCheckState() const;
    void SetCheckState(Qt::CheckState checkState);

    String GetTextHint() const;

private:
    DAVA_VIRTUAL_REFLECTION(BoolComponentValue, BaseComponentValue);
};
}
}
