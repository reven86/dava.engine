#include "TArc/Controls/CheckBox.h"

#include <Base/FastName.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
namespace TArc
{
CheckBox::CheckBox(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QCheckBox>(ControlDescriptor(fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

CheckBox::CheckBox(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QCheckBox>(ControlDescriptor(fields), accessor, model, parent)
{
    SetupControl();
}

void CheckBox::SetupControl()
{
    connections.AddConnection(this, &QCheckBox::stateChanged, MakeFunction(this, &CheckBox::StateChanged));
}

void CheckBox::UpdateControl(const ControlDescriptor& changedFields)
{
    DAVA::Reflection fieldValue = model.GetField(changedFields.GetName(Fields::Checked));
    DVASSERT(fieldValue.IsValid());

    setEnabled(!IsValueReadOnly(changedFields, Fields::Checked, Fields::IsReadOnly));

    const DAVA::M::ValueDescription* valueDescriptor = fieldValue.GetMeta<DAVA::M::ValueDescription>();
    if (valueDescriptor != nullptr)
    {
        setText(QString::fromStdString(valueDescriptor->GetDescription(fieldValue.GetValue())));
    }
    else if (changedFields.IsChanged(Fields::TextHint) == true)
    {
        DAVA::Reflection hintField = model.GetField(changedFields.GetName(Fields::TextHint));
        DVASSERT(hintField.IsValid());

        setText(QString::fromStdString(hintField.GetValue().Cast<String>()));
    }

    if (changedFields.IsChanged(Fields::Checked) == true)
    {
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
    if (isEnabled() == false)
    {
        return;
    }

    if (newState != Qt::PartiallyChecked)
    {
        Any currentValue = GetFieldValue(Fields::Checked, Any());
        setTristate(false);
        if (dataType == eContainedDataType::TYPE_CHECK_STATE)
        {
            Any newValue = Any(checkState());
            if (currentValue != newValue)
            {
                wrapper.SetFieldValue(GetFieldName(Fields::Checked), newValue);
            }
        }
        else if (dataType == eContainedDataType::TYPE_BOOL)
        {
            Qt::CheckState checkStateValue = checkState();
            DVASSERT(checkStateValue != Qt::PartiallyChecked);
            Any newValue = Any(checkStateValue == Qt::Checked ? true : false);
            if (currentValue != newValue)
            {
                wrapper.SetFieldValue(GetFieldName(Fields::Checked), newValue);
            }
        }
        else
        {
            DVASSERT(false);
        }

        DAVA::Reflection fieldValue = model.GetField(GetFieldName(Fields::Checked));
        const DAVA::M::ValueDescription* valueDescriptor = fieldValue.GetMeta<DAVA::M::ValueDescription>();
        if (valueDescriptor != nullptr)
        {
            setText(QString::fromStdString(valueDescriptor->GetDescription(fieldValue.GetValue())));
        }
    }
}

} // namespace TArc
} // namespace DAVA
