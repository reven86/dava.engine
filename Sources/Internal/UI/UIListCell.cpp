#include "UI/UIListCell.h"
#include "Base/ObjectFactory.h"

namespace DAVA
{
DAVA_REFLECTION_IMPL(UIListCell)
{
    ReflectionRegistrator<UIListCell>::Begin()
    .Field("identifier", &UIListCell::GetIdentifier, &UIListCell::SetIdentifier)
    .End();
}

UIListCell::UIListCell(const Rect& rect, const String& cellIdentifier)
    : UIButton(rect)
    , currentIndex(-1)
    , identifier(cellIdentifier)
    , cellStore(NULL)
{
}

UIListCell::~UIListCell()
{
}

const String& UIListCell::GetIdentifier() const
{
    return identifier;
}

void UIListCell::SetIdentifier(const String& newIdentifier)
{
    identifier = newIdentifier;
}

int32 UIListCell::GetIndex() const
{
    return currentIndex;
}

UIListCell* UIListCell::Clone()
{
    UIListCell* c = new UIListCell(GetRect(), identifier);
    c->CopyDataFrom(this);
    return c;
}

void UIListCell::CopyDataFrom(UIControl* srcControl)
{
    UIButton::CopyDataFrom(srcControl);
    UIListCell* srcListCell = static_cast<UIListCell*>(srcControl);
    identifier = srcListCell->identifier;
}
};