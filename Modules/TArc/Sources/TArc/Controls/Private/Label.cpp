#include "TArc/Controls/Label.h"
#include "TArc/Controls/CommonStrings.h"

#include <Base/FastName.h>
#include <Reflection/ReflectedMeta.h>

#include <QString>

namespace DAVA
{
namespace TArc
{
Label::Label(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QLabel>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
}

Label::Label(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QLabel>(params, ControlDescriptor(params.fields), accessor, model, parent)
{
}

void Label::UpdateControl(const ControlDescriptor& descriptor)
{
    if (descriptor.IsChanged(Fields::Text))
    {
        DAVA::Reflection fieldValue = model.GetField(descriptor.GetName(Fields::Text));
        DVASSERT(fieldValue.IsValid());

        QString stringValue;
        Any value = fieldValue.GetValue();
        if (value.IsEmpty() == true)
        {
            stringValue = QString(MultipleValuesString);
        }
        else if (value.CanCast<QString>())
        {
            stringValue = value.Cast<QString>();
        }
        else if (value.CanCast<String>())
        {
            stringValue = QString::fromStdString(value.Cast<String>());
        }
        else
        {
            DVASSERT(false);
            stringValue = QString("ALARM!!! Cast from %1 to String is not registered").arg(value.GetType()->GetName());
        }

        setText(stringValue);
        setToolTip(stringValue);
    }
}

} // namespace TArc
} // namespace DAVA
