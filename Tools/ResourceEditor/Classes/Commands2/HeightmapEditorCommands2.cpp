#include "HeightmapEditorCommands2.h"
#include "../Qt/Scene/System/LandscapeEditorDrawSystem/HeightmapProxy.h"
#include "../Qt/Scene/System/LandscapeEditorDrawSystem/LandscapeProxy.h"
#include "../Qt/Scene/SceneEditor2.h"
#include "../Qt/Scene/SceneSignals.h"

#include "../Qt/Main/QtUtils.h"

ModifyHeightmapCommand::ModifyHeightmapCommand(HeightmapProxy* heightmapProxy,
                                               DAVA::Heightmap* originalHeightmap,
                                               const DAVA::Rect& updatedRect)
    : Command2(CMDID_HEIGHTMAP_MODIFY, "Height Map Change")
    , heightmapProxy(heightmapProxy)
{
    if (originalHeightmap && heightmapProxy)
    {
        this->updatedRect = updatedRect;
        undoRegion = GetHeightmapRegion(originalHeightmap);
        redoRegion = GetHeightmapRegion(heightmapProxy);
    }
}

ModifyHeightmapCommand::~ModifyHeightmapCommand()
{
    DAVA::SafeDeleteArray(undoRegion);
    DAVA::SafeDeleteArray(redoRegion);
}

DAVA::Entity* ModifyHeightmapCommand::GetEntity() const
{
    return NULL;
}

void ModifyHeightmapCommand::Redo()
{
    ApplyHeightmapRegion(redoRegion);
}

void ModifyHeightmapCommand::Undo()
{
    ApplyHeightmapRegion(undoRegion);
}

DAVA::uint16* ModifyHeightmapCommand::GetHeightmapRegion(DAVA::Heightmap* heightmap)
{
    DAVA::int32 size = heightmap->Size();
    DAVA::int32 width = static_cast<DAVA::int32>(ceilf(updatedRect.dx));
    DAVA::int32 height = static_cast<DAVA::int32>(ceilf(updatedRect.dy));
    DAVA::int32 xOffset = static_cast<DAVA::int32>(floorf(updatedRect.x));
    DAVA::int32 yOffset = static_cast<DAVA::int32>(floorf(updatedRect.y));

    DVASSERT((xOffset + width) <= size && (yOffset + height) <= size);

    DAVA::uint16* newData = new DAVA::uint16[width * height];
    DAVA::uint16* oldData = heightmap->Data();

    for (DAVA::int32 i = 0; i < height; ++i)
    {
        DAVA::uint16* src = oldData + (yOffset + i) * size + xOffset;
        DAVA::uint16* dst = newData + i * width;
        memcpy(dst, src, sizeof(DAVA::uint16) * width);
    }

    return newData;
}

void ModifyHeightmapCommand::ApplyHeightmapRegion(DAVA::uint16* region)
{
    DAVA::int32 size = heightmapProxy->Size();
    DAVA::int32 width = static_cast<DAVA::int32>(ceilf(updatedRect.dx));
    DAVA::int32 height = static_cast<DAVA::int32>(ceilf(updatedRect.dy));
    DAVA::int32 xOffset = static_cast<DAVA::int32>(floorf(updatedRect.x));
    DAVA::int32 yOffset = static_cast<DAVA::int32>(floorf(updatedRect.y));

    DVASSERT((xOffset + width) <= size && (yOffset + height) <= size);

    DAVA::uint16* data = heightmapProxy->Data();

    for (DAVA::int32 i = 0; i < height; ++i)
    {
        DAVA::uint16* src = region + i * width;
        DAVA::uint16* dst = data + (yOffset + i) * size + xOffset;
        memcpy(dst, src, sizeof(DAVA::uint16) * width);
    }

    heightmapProxy->UpdateRect(updatedRect);
}
