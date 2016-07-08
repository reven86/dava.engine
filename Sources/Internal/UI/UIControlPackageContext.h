#ifndef __DAVAENGINE_UI_CONTROL_PACKAGE_CONTEXT_H__
#define __DAVAENGINE_UI_CONTROL_PACKAGE_CONTEXT_H__

#include "Base/BaseObject.h"
#include "Base/BaseTypes.h"
#include "UI/Styles/UIPriorityStyleSheet.h"

namespace DAVA
{
class UIStyleSheet;

class UIControlPackageContext :
public BaseObject
{
protected:
    virtual ~UIControlPackageContext();

public:
    UIControlPackageContext();

    void AddStyleSheet(const UIPriorityStyleSheet& styleSheet);
    void RemoveAllStyleSheets();

    const Vector<UIPriorityStyleSheet>& GetSortedStyleSheets();

private:
    Vector<UIPriorityStyleSheet> styleSheets;
    bool styleSheetsSorted;
};
};

#endif
