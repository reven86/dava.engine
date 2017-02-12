#include "Modules/DocumentsModule/CentralWidgetData.h"

const char* CentralWidgetData::canvasPositionPropertyName = "canvas posititon";

const DAVA::Vector2 CentralWidgetData::invalidPosition = DAVA::Vector2(-1.0f, -1.0f);

DAVA_VIRTUAL_REFLECTION_IMPL(CentralWidgetData)
{
    DAVA::ReflectionRegistrator<CentralWidgetData>::Begin()
    .Field(canvasPositionPropertyName, &CentralWidgetData::canvasPosition)
    .End();
}
