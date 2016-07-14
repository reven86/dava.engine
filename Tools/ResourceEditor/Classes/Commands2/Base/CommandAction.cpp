#include "Commands2/Base/CommandAction.h"

CommandAction::CommandAction(DAVA::CommandID_t id, const DAVA::String& text)
    : CommandWithoutExecute(id, text)
{
}
