#include "UI/UIControlPackageContext.h"
#include "UI/Styles/UIStyleSheet.h"

namespace DAVA
{
UIControlPackageContext::~UIControlPackageContext()
{
}

UIControlPackageContext::UIControlPackageContext()
    :
    styleSheetsSorted(false)
{
}

void UIControlPackageContext::AddStyleSheet(const UIPriorityStyleSheet& styleSheet)
{
    styleSheetsSorted = false;

    auto it = std::find_if(styleSheets.begin(), styleSheets.end(), [&styleSheet](UIPriorityStyleSheet& ss) {
        return ss.GetStyleSheet() == styleSheet.GetStyleSheet();
    });

    if (it == styleSheets.end())
    {
        styleSheets.push_back(styleSheet);
    }
    else
    {
        if (styleSheet.GetPriority() < it->GetPriority())
            *it = styleSheet;
    }
}

void UIControlPackageContext::RemoveAllStyleSheets()
{
    styleSheets.clear();
}

const Vector<UIPriorityStyleSheet>& UIControlPackageContext::GetSortedStyleSheets()
{
    if (!styleSheetsSorted)
    {
        std::sort(styleSheets.begin(), styleSheets.end());
        styleSheetsSorted = true;
    }

    return styleSheets;
}
}