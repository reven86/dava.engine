#include "TArc/Controls/LineEdit.h"
#include "Base/FastName.h"

#include <Reflection/MetaObjects.h>

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
    connections.AddConnection(static_cast<QLineEdit*>(this), &QLineEdit::editingFinished, MakeFunction(this, &LineEdit::EditingFinished));
}

void LineEdit::EditingFinished()
{
    if (!isReadOnly())
    {
        wrapper.SetFieldValue(GetFieldName(Text), text().toStdString());
    }
}

void LineEdit::UpdateControl(const ControlDescriptor& descriptor)
{
    bool readOnlyChanged = descriptor.IsChanged(IsReadOnly);
    bool textChanged = descriptor.IsChanged(Text);
    if (readOnlyChanged || textChanged)
    {
        DAVA::Reflection fieldValue = model.GetField(descriptor.GetName(Text));
        DVASSERT(fieldValue.IsValid());

        if (readOnlyChanged)
        {
            DAVA::Reflection fieldReadOnly = model.GetField(descriptor.GetName(IsReadOnly));
            bool readOnlyFieldValue = false;
            if (fieldReadOnly.IsValid())
            {
                readOnlyFieldValue = fieldReadOnly.GetValue().Cast<bool>();
            }
            setReadOnly(fieldValue.IsReadonly() || fieldValue.GetMeta<DAVA::M::ReadOnly>() || readOnlyFieldValue);
        }

        if (textChanged)
        {
            setText(QString::fromStdString(fieldValue.GetValue().Cast<String>()));
        }
    }

    if (descriptor.IsChanged(IsEnabled))
    {
        DAVA::Reflection fieldEnabled = model.GetField(descriptor.GetName(IsEnabled));
        bool isEnabled = true;
        if (fieldEnabled.IsValid())
        {
            isEnabled = fieldEnabled.GetValue().Cast<bool>();
        }

        setEnabled(isEnabled);
    }

    if (descriptor.IsChanged(PlaceHolder))
    {
        DAVA::Reflection fieldPlaceholder = model.GetField(descriptor.GetName(PlaceHolder));
        String placeHolder;
        if (fieldPlaceholder.IsValid())
        {
            placeHolder = fieldPlaceholder.GetValue().Cast<String>();
        }

        setPlaceholderText(QString::fromStdString(placeHolder));
    }
}

} // namespace TArc
} // namespace DAVA
