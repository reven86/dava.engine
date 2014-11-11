#include "UIControlHelpers.h"
#include "UI/UIControl.h"
#include "Utils/Utils.h"

namespace DAVA
{

String UIControlHelpers::GetControlPath(const UIControl *control, const UIControl *rootControl /*= NULL*/)
{
    if (!control)
        return "";

    String controlPath = "";
    UIControl * controlIter = control->GetParent();
    do
    {
        if (!controlIter)
            return "";

        controlPath = controlIter->GetName() + "/" + controlPath;

        controlIter = controlIter->GetParent();
    } while (controlIter != rootControl);

    return controlPath;
}

UIControl *UIControlHelpers::GetControlByPath(const String &controlPath, const UIControl *rootControl)
{
    const UIControl* control = rootControl;
    Vector<String> controlNames;
    Split(controlPath, "/", controlNames, false, true);
    Vector<String>::const_iterator iter = controlNames.begin();
    UIControl* findControl = NULL;
    for (; iter!=controlNames.end(); ++iter)
    {
        findControl = control->FindByName(*iter,false);
        if (!findControl)
        {
            return NULL;
        }
        control = findControl;
    }
    return findControl;
}

}
