#include "TArc/Controls/CheckBox.h"
#include "Base/FastName.h"

namespace DAVA
{
namespace TArc
{
CheckBox::CheckBox(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxy<QCheckBox>(ControlDescriptor(fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

CheckBox::CheckBox(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxy<QCheckBox>(ControlDescriptor(fields), accessor, model, parent)
{
    SetupControl();
}

void CheckBox::SetupControl()
{
    connections.AddConnection(static_cast<QCheckBox*>(this), &QCheckBox::stateChanged, MakeFunction(this, &CheckBox::StateChanged));
}

void CheckBox::UpdateControl(const ControlDescriptor& changedFields)
{
    if (changedFields.IsChanged(Checked))
    {
        DAVA::Reflection fieldValue = model.GetField(changedFields.GetName(Checked));
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
            wrapper.SetFieldValue(GetFieldName(Checked), Any(checkState()));
        }
        else if (dataType == eContainedDataType::TYPE_BOOL)
        {
            wrapper.SetFieldValue(GetFieldName(Checked), Any(checkState() == Qt::Checked));
        }
        else
        {
            DVASSERT(false);
        }
    }
}

} // namespace TArc
} // namespace DAVA
