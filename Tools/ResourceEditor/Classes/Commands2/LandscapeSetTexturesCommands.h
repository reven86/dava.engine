#ifndef __RESOURCEEDITORQT__LANDSCAPESETTEXTURESCOMMANDS2__
#define __RESOURCEEDITORQT__LANDSCAPESETTEXTURESCOMMANDS2__

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "Math/AABBox3.h"

#include "Commands2/Base/RECommand.h"

namespace DAVA
{
class Entity;
class Landscape;
}

class LandscapeProxy;

class LandscapeSetHeightMapCommand : public RECommand
{
public:
    LandscapeSetHeightMapCommand(DAVA::Entity* landscapeEntity,
                                 const DAVA::FilePath& texturePath,
                                 const DAVA::AABBox3& newLandscapeBox);

    ~LandscapeSetHeightMapCommand();

    void Undo() override;
    void Redo() override;

    DAVA::Entity* GetEntity() const
    {
        return landscapeEntity;
    }

protected:
    DAVA::FilePath originalHeightMapPath;
    DAVA::FilePath newHeightMapPath;
    DAVA::Entity* landscapeEntity;
    DAVA::Landscape* landscape;
    DAVA::AABBox3 originalLandscapeBox;
    DAVA::AABBox3 newLandscapeBox;
};


#endif /* defined(__RESOURCEEDITORQT__LANDSCAPESETTEXTURESCOMMANDS2__) */
