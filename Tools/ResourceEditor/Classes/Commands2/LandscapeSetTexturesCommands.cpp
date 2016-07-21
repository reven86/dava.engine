#include "LandscapeSetTexturesCommands.h"
#include "../Qt/Scene/SceneEditor2.h"
#include "Scene3D/Components/ComponentHelpers.h"

LandscapeSetHeightMapCommand::LandscapeSetHeightMapCommand(DAVA::Entity* landscapeEntity_,
                                                           const DAVA::FilePath& heightMapPath_,
                                                           const DAVA::AABBox3& newLandscapeBox_)
    : RECommand(CMDID_LANDSCAPE_SET_HEIGHTMAP, "Set Landscape heightmap")
{
    landscape = FindLandscape(landscapeEntity_);
    if (NULL == landscape)
    {
        return;
    }
    landscapeEntity = SafeRetain(landscapeEntity_);

    originalHeightMapPath = landscape->GetHeightmapPathname();
    originalLandscapeBox = landscape->GetBoundingBox();
    newHeightMapPath = heightMapPath_;
    newLandscapeBox = newLandscapeBox_;
}

LandscapeSetHeightMapCommand::~LandscapeSetHeightMapCommand()
{
    SafeRelease(landscapeEntity);
}

void LandscapeSetHeightMapCommand::Undo()
{
    landscape->BuildLandscapeFromHeightmapImage(originalHeightMapPath, originalLandscapeBox);
}

void LandscapeSetHeightMapCommand::Redo()
{
    landscape->BuildLandscapeFromHeightmapImage(newHeightMapPath, newLandscapeBox);
}