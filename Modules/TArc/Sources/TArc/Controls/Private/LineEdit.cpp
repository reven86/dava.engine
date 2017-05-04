#include "TArc/Controls/LineEdit.h"
#include "TArc/Controls/Private/TextValidator.h"

#include <Base/FastName.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
namespace TArc
{
LineEdit::LineEdit(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QLineEdit>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

LineEdit::LineEdit(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QLineEdit>(params, ControlDescriptor(params.fields), accessor, model, parent)
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
    RETURN_IF_MODEL_LOST(void());

    if (!isReadOnly())
    {
        String newText = text().toStdString();
        if (GetFieldValue<String>(Fields::Text, "") != newText)
        {
            wrapper.SetFieldValue(GetFieldName(Fields::Text), newText);
        }
    }
}

void LineEdit::UpdateControl(const ControlDescriptor& descriptor)
{
    RETURN_IF_MODEL_LOST(void());
    bool readOnlyChanged = descriptor.IsChanged(Fields::IsReadOnly);
    bool textChanged = descriptor.IsChanged(Fields::Text);
    if (readOnlyChanged || textChanged)
    {
        DAVA::Reflection fieldValue = model.GetField(descriptor.GetName(Fields::Text));
        DVASSERT(fieldValue.IsValid());

        setReadOnly(IsValueReadOnly(descriptor, Fields::Text, Fields::IsReadOnly));

        if (textChanged)
        {
            setText(QString::fromStdString(fieldValue.GetValue().Cast<String>()));
        }
    }

    if (descriptor.IsChanged(Fields::IsEnabled))
    {
        setEnabled(GetFieldValue<bool>(Fields::IsEnabled, true));
    }

    if (descriptor.IsChanged(Fields::PlaceHolder))
    {
        setPlaceholderText(QString::fromStdString(GetFieldValue<String>(Fields::PlaceHolder, "")));
    }
}

M::ValidationResult LineEdit::Validate(const Any& value) const
{
    RETURN_IF_MODEL_LOST(M::ValidationResult());
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
    NotificationParams notifParams;
    notifParams.title = "Invalid value";
    notifParams.message.message = message.toStdString();
    notifParams.message.type = ::DAVA::Result::RESULT_ERROR;
    controlParams.ui->ShowNotification(controlParams.wndKey, notifParams);
}

} // namespace TArc
} // namespace DAVA
