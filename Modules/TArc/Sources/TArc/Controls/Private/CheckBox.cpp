#include "TArc/Controls/CheckBox.h"
#include "Base/FastName.h"

namespace DAVA
{
namespace TArc
{
CheckBox::CheckBox(const FieldsDescriptor& fields_, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxy<QCheckBox>(wrappersProcessor, model, parent)
    , fieldsDescr(fields_)
{
    SetupControl();
}

CheckBox::CheckBox(const FieldsDescriptor& fields_, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxy<QCheckBox>(accessor, model, parent)
    , fieldsDescr(fields_)
{
    SetupControl();
}

void CheckBox::SetupControl()
{
    setTristate(true);
    connections.AddConnection(static_cast<QCheckBox*>(this), &QCheckBox::stateChanged, MakeFunction(this, &CheckBox::StateChanged));
}

void CheckBox::OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields)
{
    if (wrapper.HasData() == false)
    {
        setCheckState(Qt::Unchecked);
        return;
    }

    bool shouldUpdateState = fields.empty();
    for (const Any& fieldName : fields)
    {
        if (fieldName == fieldsDescr.valueFieldName)
        {
            shouldUpdateState = true;
            break;
        }
    }

    if (shouldUpdateState == true)
    {
        DAVA::Reflection fieldValue = model.GetField(Any(fieldsDescr.valueFieldName.Get<FastName>().c_str()));
        DVASSERT(fieldValue.IsValid());

        setCheckState(fieldValue.GetValue().Cast<Qt::CheckState>());
    }
}

void CheckBox::StateChanged(int newState)
{
    if (newState != Qt::PartiallyChecked)
    {
        setTristate(false);
        wrapper.SetFieldValue(Any(fieldsDescr.valueFieldName.Get<FastName>().c_str()), Any(checkState()));
    }
}

} // namespace TArc
} // namespace DAVA
