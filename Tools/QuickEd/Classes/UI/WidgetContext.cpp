#include "WidgetContext.h"
#include <DAVAEngine.h>

WidgetContext::WidgetContext(QObject *parent)
    : QObject(parent)
{

}

QVariant& WidgetContext::GetData(const QByteArray &role)
{
    return values[role];
}

void WidgetContext::SetData(const QByteArray &role, const QVariant &value)
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

WidgetDelta* WidgetContext::GetDelta(QWidget* requester) const
{
    return deltas.find(requester) != deltas.end() ? deltas.at(requester).get() : nullptr;
}

void WidgetContext::SetDelta(QWidget* requester, WidgetDelta* widgetDelta)
{
    deltas.insert(std::make_pair(requester, std::unique_ptr<WidgetDelta>(widgetDelta)));
}