#ifndef __RESOURCEEDITORQT__DAECONVERTACTION__
#define __RESOURCEEDITORQT__DAECONVERTACTION__

#include "Commands2/Base/CommandAction.h"
#include "DAVAEngine.h"

class DAEConvertAction : public CommandAction
{
public:
    DAEConvertAction(const DAVA::FilePath& path);

    void Redo() override;

protected:
    DAVA::FilePath daePath;
};

#endif // __RESOURCEEDITORQT__DAECONVERTACTION__
