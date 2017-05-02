#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"

#include <Reflection/Reflection.h>
#include <Base/BaseTypes.h>

namespace DAVA
{
namespace TArc
{
class TextComponentValue : public BaseComponentValue
{
public:
    TextComponentValue() = default;
    ~TextComponentValue() override = default;

protected:
    Any GetMultipleValue() const override;
    bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const override;
    ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const override;

private:
    String GetText() const;
    void SetText(const String& text);

private:
    DAVA_VIRTUAL_REFLECTION(TextComponentValue, BaseComponentValue);
};
}
}
