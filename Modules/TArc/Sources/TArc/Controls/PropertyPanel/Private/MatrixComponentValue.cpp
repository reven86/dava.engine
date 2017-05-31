#include "TArc/Controls/PropertyPanel/Private/MatrixComponentValue.h"
#include "TArc/Controls/CommonStrings.h"
#include "TArc/Controls/Label.h"

namespace DAVA
{
namespace TArc
{
Any MatrixComponentValue::GetMultipleValue() const
{
    return String(MultipleValuesString);
}

bool MatrixComponentValue::IsValidValueToSet(const Any& newValue, const Any& currentValue) const
{
    return false;
}

ControlProxy* MatrixComponentValue::CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor)
{
    Label::Params params(GetAccessor(), GetUI(), GetWindowKey());
    params.fields[Label::Fields::Text] = "value";
    return new Label(params, wrappersProcessor, model, parent);
}

String MatrixComponentValue::GetTextValue() const
{
    return GetValue().Cast<String>();
}

DAVA_VIRTUAL_REFLECTION_IMPL(MatrixComponentValue)
{
    ReflectionRegistrator<MatrixComponentValue>::Begin()
    .Field("value", &MatrixComponentValue::GetTextValue, nullptr)
    .End();
}

} // namespace TArc
} // namespace DAVA
