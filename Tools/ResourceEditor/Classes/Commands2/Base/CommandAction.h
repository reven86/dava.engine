#ifndef __RESOURCEEDITORQT__COMMANDACTION__
#define __RESOURCEEDITORQT__COMMANDACTION__

#include "Commands2/Base/Command2.h"

class CommandAction : public Command2
{
public:
    CommandAction(DAVA::int32 _id, const DAVA::String& _text = "");

    bool CanUndo() const override;

    void Undo() override;
    DAVA::Entity* GetEntity() const override;
};

inline bool CommandAction::CanUndo() const
{
    return false;
}

#endif /* defined(__RESOURCEEDITORQT__COMMANDACTION__) */
