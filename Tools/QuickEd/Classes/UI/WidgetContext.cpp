#include "WidgetContext.h"
#include <DAVAEngine.h>
WidgetContext::WidgetContext(QObject *parent)
    : QObject(parent)
{

}

WidgetContext::~WidgetContext()
{ //TODO: remove it by passing view and canvas to delta-class
    if (values.contains("view"))
    {
        DAVA::UIControl *control = GetData("view").value<DAVA::UIControl*>();
        DAVA::SafeRelease(control);
    }
    if (values.contains("canvas"))
    {
        DAVA::UIControl *control = GetData("canvas").value<DAVA::UIControl*>();
        DAVA::SafeRelease(control);
    }
}


QVariant& WidgetContext::GetData(const QByteArray &role)
{
    return values[role];
}

void WidgetContext::SetData(const QVariant value, const QByteArray &role)
{
    if (values.contains(role))
    {
        if (values[role] == value)
        {
            return;
        }
    }
    values[role] = value;
    emit DataChanged(role);
}