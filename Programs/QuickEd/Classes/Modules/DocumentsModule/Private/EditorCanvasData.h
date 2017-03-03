#pragma once

#include <TArc/DataProcessing/DataNode.h>

#include <Math/Vector.h>

class EditorCanvasData : public DAVA::TArc::DataNode
{
public:
    EditorCanvasData() = default;
    ~EditorCanvasData() override = default;

    static const char* canvasPositionPropertyName;
    static const char* needCentralizeXPropertyName;
    static const char* needCentralizeYPropertyName;
    static const DAVA::Vector2 invalidPosition;

private:
    friend class EditorCanvas;

    DAVA::Vector2 canvasPosition = invalidPosition;
    bool needCentralizeX = false;
    bool needCentralizeY = false;

    DAVA_VIRTUAL_REFLECTION(EditorCanvasData, DAVA::TArc::DataNode);
};
