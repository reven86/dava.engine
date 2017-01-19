#include "TArc/Controls/LineEdit.h"
#include "Base/FastName.h"

namespace DAVA
{
namespace TArc
{
LineEdit::LineEdit(const FieldsDescriptor& fields_, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxy<QLineEdit>(wrappersProcessor, model, parent)
    , fieldsDescr(fields_)
{
    SetupControl();
}

LineEdit::LineEdit(const FieldsDescriptor& fields_, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxy<QLineEdit>(accessor, model, parent)
    , fieldsDescr(fields_)
{
    SetupControl();
}

void LineEdit::OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields)
{
    DVASSERT(wrapper.HasData());
    bool shouldUpdateText = fields.empty();
    for (const Any& fieldName : fields)
    {
        if (fieldName.Cast<String>() == fieldsDescr.valueFieldName.Cast<String>())
        {
            shouldUpdateText = true;
            break;
        }
    }

    if (shouldUpdateText == true)
    {
        DAVA::Reflection fieldValue = model.GetField(fieldsDescr.valueFieldName);
        DVASSERT(fieldValue.IsValid());
        setText(QString::fromStdString(fieldValue.GetValue().Cast<String>()));
    }
}

void LineEdit::SetupControl()
{
    connections.AddConnection(static_cast<QLineEdit*>(this), &QLineEdit::editingFinished, MakeFunction(this, &LineEdit::EditingFinished));
}

void LineEdit::EditingFinished()
{
    wrapper.SetFieldValue(fieldsDescr.valueFieldName, text().toStdString());
}

} // namespace TArc
} // namespace DAVA
