#pragma once

#include <TArc/DataProcessing/DataNode.h>

struct WidgetContext
{
    virtual ~WidgetContext() = 0;
};

inline WidgetContext::~WidgetContext()
{
}

class WidgetsData : public DAVA::TArc::DataNode
{
public:
    WidgetsData();
    ~WidgetsData() override;

    WidgetContext* GetContext(void* requester) const;
    void SetContext(void* requester, WidgetContext* widgetContext);

private:
    DAVA::UnorderedMap<void*, WidgetContext*> contexts;
    DAVA_VIRTUAL_REFLECTION(WidgetsData, DAVA::TArc::DataNode);
};
