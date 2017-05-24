#include "TArc/Controls/ReflectedButton.h"

#include <Base/FastName.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
namespace TArc
{
ReflectedButton::ReflectedButton(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QToolButton>(params, params.fields, wrappersProcessor, model, parent)
{
    SetupControl();
}

ReflectedButton::ReflectedButton(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QToolButton>(params, params.fields, accessor, model, parent)
{
    SetupControl();
}

void ReflectedButton::SetupControl()
{
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    setAutoRaise(autoRaise);
    setEnabled(true);

    connections.AddConnection(this, &QToolButton::released, MakeFunction(this, &ReflectedButton::ButtonReleased));
}

void ReflectedButton::UpdateControl(const ControlDescriptor& changedFields)
{
    if (changedFields.IsChanged(Fields::Visible) == true)
    {
        setVisible(GetFieldValue<bool>(Fields::Visible, false));
    }
    if (changedFields.IsChanged(Fields::Icon) == true)
    {
        icon = GetFieldValue<QIcon>(Fields::Icon, QIcon());
    }
    if (changedFields.IsChanged(Fields::Text) == true)
    {
        text = GetFieldValue<QString>(Fields::Text, QString());
    }

    setIcon(icon);
    setText(text);

    if (changedFields.IsChanged(Fields::Tooltip) == true)
    {
        QString tooltip = GetFieldValue<QString>(Fields::Tooltip, QString());
        setToolTip(tooltip);
    }

    if (changedFields.IsChanged(Fields::IconSize) == true)
    {
        QSize iconSize = GetFieldValue<QSize>(Fields::IconSize, QSize(16, 16));
        setIconSize(iconSize);
    }

    if (icon.isNull() == false && text.isEmpty() == false)
    {
        setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    }
    else if (icon.isNull() == false)
    {
        setToolButtonStyle(Qt::ToolButtonIconOnly);
    }
    else if (text.isEmpty() == false)
    {
        setToolButtonStyle(Qt::ToolButtonTextOnly);
    }
    else
    {
        DVASSERT(false);
    }

    if (changedFields.IsChanged(Fields::AutoRaise) == true)
    {
        autoRaise = GetFieldValue<bool>(Fields::AutoRaise, true);
    }
    setAutoRaise(autoRaise);

    if (changedFields.IsChanged(Fields::Enabled) == true)
    {
        bool enabled = GetFieldValue<bool>(Fields::Enabled, true);
        setEnabled(enabled);
    }
}

void ReflectedButton::ButtonReleased()
{
    AnyFn method = model.GetMethod(GetFieldName(Fields::Clicked).c_str());
    DVASSERT(method.IsValid());

    const AnyFn::Params& params = method.GetInvokeParams();

    const Type* retType = params.retType;
    Vector<const Type*> argsType = params.argsType;

    if (argsType.empty())
    {
        Reflection resultValue = model.GetField(GetFieldName(Fields::Result));
        if (retType != nullptr && resultValue.IsValid())
        {
            Any result = method.Invoke();
            wrapper.SetFieldValue(GetFieldName(Fields::Result), result);
        }
        else
        {
            method.Invoke();
        }
    }
    else
    {
        DVASSERT(false, "We could invoke only methods without arguments");
    }
}

} // namespace TArc
} // namespace DAVA
