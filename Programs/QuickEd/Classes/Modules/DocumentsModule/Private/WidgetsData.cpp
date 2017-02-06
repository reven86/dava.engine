#include "Modules/DocumentsModule/WidgetsData.h"

DAVA_VIRTUAL_REFLECTION_IMPL(WidgetsData)
{
}

WidgetsData::WidgetsData() = default;

WidgetsData::~WidgetsData()
{
    for (auto& context : contexts)
    {
        delete context.second;
    }
}

WidgetContext* WidgetsData::GetContext(void* requester) const
{
    auto iter = contexts.find(requester);
    if (iter != contexts.end())
    {
        return iter->second;
    }
    return nullptr;
}

void WidgetsData::SetContext(void* requester, WidgetContext* widgetContext)
{
    auto iter = contexts.find(requester);
    if (iter != contexts.end())
    {
        DVASSERT(false, "document already have this context");
        delete iter->second;
        contexts.erase(iter);
    }
    contexts.emplace(requester, widgetContext);
}