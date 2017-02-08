#include "TArc/Controls/LineEdit.h"
#include "TArc/Controls/Private/TextValidator.h"

#include <Base/FastName.h>
#include <Reflection/ReflectedMeta.h>

#include <QToolTip>

namespace DAVA
{
namespace TArc
{
LineEdit::LineEdit(const ControlDescriptorBuilder<LineEdit::Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxy<QLineEdit>(ControlDescriptor(fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

LineEdit::LineEdit(const ControlDescriptorBuilder<LineEdit::Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxy<QLineEdit>(ControlDescriptor(fields), accessor, model, parent)
{
    SetupControl();
}

void LineEdit::SetupControl()
{
    connections.AddConnection(this, &QLineEdit::editingFinished, MakeFunction(this, &LineEdit::EditingFinished));
    TextValidator* validator = new TextValidator(this, this);
    setValidator(validator);
}

void LineEdit::EditingFinished()
{
    if (!isReadOnly())
    {
        wrapper.SetFieldValue(GetFieldName(Fields::Text), text().toStdString());
    }
}

void LineEdit::UpdateControl(const ControlDescriptor& descriptor)
{
    bool readOnlyChanged = descriptor.IsChanged(Fields::IsReadOnly);
    bool textChanged = descriptor.IsChanged(Fields::Text);
    if (readOnlyChanged || textChanged)
    {
        DAVA::Reflection fieldValue = model.GetField(descriptor.GetName(Fields::Text));
        DVASSERT(fieldValue.IsValid());

        bool readOnlyFieldValue = false;
        if (readOnlyChanged)
        {
            DAVA::Reflection fieldReadOnly = model.GetField(descriptor.GetName(Fields::IsReadOnly));
            if (fieldReadOnly.IsValid())
            {
                readOnlyFieldValue = fieldReadOnly.GetValue().Cast<bool>();
            }
        }
        setReadOnly(fieldValue.IsReadonly() == true || fieldValue.GetMeta<DAVA::M::ReadOnly>() != nullptr || readOnlyFieldValue == true);

        if (textChanged)
        {
            setText(QString::fromStdString(fieldValue.GetValue().Cast<String>()));
        }
    }

    if (descriptor.IsChanged(Fields::IsEnabled))
    {
        DAVA::Reflection fieldEnabled = model.GetField(descriptor.GetName(Fields::IsEnabled));
        bool isEnabled = true;
        if (fieldEnabled.IsValid())
        {
            isEnabled = fieldEnabled.GetValue().Cast<bool>();
        }

        setEnabled(isEnabled);
    }

    if (descriptor.IsChanged(Fields::PlaceHolder))
    {
        DAVA::Reflection fieldPlaceholder = model.GetField(descriptor.GetName(Fields::PlaceHolder));
        String placeHolder;
        if (fieldPlaceholder.IsValid())
        {
            placeHolder = fieldPlaceholder.GetValue().Cast<String>();
        }

        setPlaceholderText(QString::fromStdString(placeHolder));
    }
}

M::ValidationResult LineEdit::Validate(const Any& value) const
{
    Reflection field = model.GetField(GetFieldName(Fields::Text));
    DVASSERT(field.IsValid());

    const M::Validator* validator = field.GetMeta<M::Validator>();
    if (validator != nullptr)
    {
        return validator->Validate(value, field.GetValue());
    }

    M::ValidationResult r;
    r.state = M::ValidationResult::eState::Valid;
    return r;
}

void LineEdit::ShowHint(const QString& message)
{
    QPoint pos = mapToGlobal(QPoint(0, 0));
    QToolTip::showText(pos, message, this);
}

} // namespace TArc
} // namespace DAVA
