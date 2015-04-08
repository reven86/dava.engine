#include "SharedData.h"
#include <DAVAEngine.h>

SharedData::SharedData(QObject *parent)
    : QObject(parent)
{

}

QVariant& SharedData::GetData(const QByteArray &role)
{
    return values[role];
}

void SharedData::SetData(const QByteArray &role, const QVariant &value)
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

WidgetContext* SharedData::GetContext(QWidget* requester) const
{
    return contexts.find(requester) != contexts.end() ? contexts.at(requester).get() : nullptr;
}

void SharedData::SetContext(QWidget* requester, WidgetContext* widgetContext)
{
    contexts.insert(std::make_pair(requester, std::unique_ptr<WidgetContext>(widgetContext)));
}