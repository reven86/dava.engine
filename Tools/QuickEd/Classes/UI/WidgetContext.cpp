#include "WidgetContext.h"

WidgetContext::WidgetContext(QObject *parent)
{

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