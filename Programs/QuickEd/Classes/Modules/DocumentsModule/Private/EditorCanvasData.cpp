#include "Modules/DocumentsModule/Private/EditorCanvasData.h"

const char* EditorCanvasData::canvasPositionPropertyName = "canvas position";
const char* EditorCanvasData::needCentralizeXPropertyName = "need centralize x";
const char* EditorCanvasData::needCentralizeYPropertyName = "need centralize y";

const DAVA::Vector2 EditorCanvasData::invalidPosition = DAVA::Vector2(-1.0f, -1.0f);

DAVA_VIRTUAL_REFLECTION_IMPL(EditorCanvasData)
{
    DAVA::ReflectionRegistrator<EditorCanvasData>::Begin()
    .Field(canvasPositionPropertyName, &EditorCanvasData::canvasPosition)
    .Field(needCentralizeXPropertyName, &EditorCanvasData::needCentralizeX)
    .Field(needCentralizeYPropertyName, &EditorCanvasData::needCentralizeY)
    .End();
}
