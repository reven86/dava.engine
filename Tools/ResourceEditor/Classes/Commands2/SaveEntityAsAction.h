#ifndef __RESOURCEEDITORQT__SAVEENTITYASACTION__
#define __RESOURCEEDITORQT__SAVEENTITYASACTION__

#include "Commands2/Base/CommandAction.h"

#include "FileSystem/FilePath.h"

namespace DAVA
{
class Entity;
}

class SelectableGroup;
class SaveEntityAsAction : public CommandAction
{
public:
    SaveEntityAsAction(const SelectableGroup* entities, const DAVA::FilePath& path);

    void Redo() override;

protected:
    void RemoveLightmapsRecursive(DAVA::Entity* entity) const;

protected:
    const SelectableGroup* entities;
    DAVA::FilePath sc2Path;
};

#endif // __RESOURCEEDITORQT__SAVEENTITYASACTION__
