#pragma once

#include "DAVAEngine.h"
#include "Deprecated/EditorHeightmap.h"

class HeightmapProxy : public EditorHeightmap
{
public:
    HeightmapProxy(DAVA::Heightmap* heightmap);

    void UpdateRect(const DAVA::Rect& rect);
    void ResetHeightmapChanged();

    bool IsHeightmapChanged() const;
    const DAVA::Rect& GetChangedRect() const;

private:
    DAVA::Rect changedRect;
    bool heightmapChanged = false;
};
