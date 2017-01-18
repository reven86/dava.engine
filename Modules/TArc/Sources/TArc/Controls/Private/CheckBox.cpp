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
    connections.AddConnection(static_cast<QCheckBox*>(this), &QCheckBox::stateChanged, MakeFunction(this, &CheckBox::StateChanged));
}

void CheckBox::OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields)
{
    DVASSERT(wrapper.HasData());

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
        DAVA::Reflection fieldValue = model.GetField(fieldsDescr.valueFieldName);
        DVASSERT(fieldValue.IsValid());

        if (fieldValue.GetValue().CanGet<Qt::CheckState>())
        {
            dataType = eContainedDataType::TYPE_CHECK_STATE;

            Qt::CheckState state = fieldValue.GetValue().Cast<Qt::CheckState>();
            setTristate(state == Qt::PartiallyChecked);
            setCheckState(state);
        }
        else if (fieldValue.GetValue().CanCast<bool>())
        {
            dataType = eContainedDataType::TYPE_BOOL;
            Qt::CheckState state = fieldValue.GetValue().Cast<bool>() ? Qt::Checked : Qt::Unchecked;
            setTristate(false);
            setCheckState(state);
        }
        else
        {
            DVASSERT(false);
        }
    }
}

void CheckBox::StateChanged(int newState)
{
    if (newState != Qt::PartiallyChecked)
    {
        setTristate(false);
        if (dataType == eContainedDataType::TYPE_CHECK_STATE)
        {
            wrapper.SetFieldValue(fieldsDescr.valueFieldName, Any(checkState()));
        }
        else if (dataType == eContainedDataType::TYPE_BOOL)
        {
            wrapper.SetFieldValue(fieldsDescr.valueFieldName, Any(checkState() == Qt::Checked));
        }
        else
        {
            DVASSERT(false);
        }
    }
}

} // namespace TArc
} // namespace DAVA
