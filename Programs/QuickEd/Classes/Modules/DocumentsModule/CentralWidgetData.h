#pragma once

#include <TArc/DataProcessing/DataNode.h>

#include <Math/Vector.h>

class CentralWidgetData : public DAVA::TArc::DataNode
{
public:
    CentralWidgetData() = default;
    ~CentralWidgetData() override = default;

private:
    friend class PreviewWidget;

    DAVA::Vector2 canvasPosition;

    DAVA_VIRTUAL_REFLECTION(CentralWidgetData, DAVA::TArc::DataNode);
};
