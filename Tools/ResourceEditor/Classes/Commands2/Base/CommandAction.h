#pragma once

#include "Base/BaseTypes.h"

class CommandAction
{
public:
    CommandAction(DAVA::uint32 id, const DAVA::String& text = DAVA::String());
    virtual void Redo() = 0;

private:
    const DAVA::uint32 id;
    const DAVA::String text;
};

