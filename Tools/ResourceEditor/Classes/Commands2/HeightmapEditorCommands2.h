#ifndef __RESOURCEEDITORQT__HEIGHTMAPEDITORCOMMANDS2__
#define __RESOURCEEDITORQT__HEIGHTMAPEDITORCOMMANDS2__

#include "Base/BaseTypes.h"

#include "Commands2/Base/Command2.h"
#include "Commands2/Base/CommandAction.h"

namespace DAVA
{
class Heightmap;
}

class HeightmapProxy;
class LandscapeProxy;
class SceneEditor2;

class ModifyHeightmapCommand : public Command2
{
public:
    ModifyHeightmapCommand(HeightmapProxy* heightmapProxy, DAVA::Heightmap* originalHeightmap, const DAVA::Rect& updatedRect);
    ~ModifyHeightmapCommand() override;

private:
    void Redo() override;
    void Undo() override;
    DAVA::Entity* GetEntity() const override;

    DAVA::uint16* GetHeightmapRegion(DAVA::Heightmap* heightmap);
    void ApplyHeightmapRegion(DAVA::uint16* region);

private:
    HeightmapProxy* heightmapProxy = nullptr;
    DAVA::uint16* undoRegion = nullptr;
    DAVA::uint16* redoRegion = nullptr;
    DAVA::Rect updatedRect;
};

#endif /* defined(__RESOURCEEDITORQT__HEIGHTMAPEDITORCOMMANDS2__) */
