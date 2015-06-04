#include "UIAnchorLayout.h"

#include "UI/UIControl.h"
#include "UIAnchorHintComponent.h"

namespace DAVA
{
    UIAnchorLayout::UIAnchorLayout()
    {
        
    }
    
    UIAnchorLayout::~UIAnchorLayout()
    {
        
    }
    
    void UIAnchorLayout::MeasureSize(UIControl *control)
    {
        
    }
    
    void UIAnchorLayout::ApplyLayout(UIControl *control)
    {
        const Vector2 &parentSize = control->GetSize();
        const List<UIControl*> &children = control->GetChildren();
        for (UIControl *child : children)
        {
            UIAnchorHintComponent *hint = child->GetComponent<UIAnchorHintComponent>();
            if (hint)
            {
                const Rect &rect = child->GetRect();
                Rect newRect = rect;

                if (hint->IsLeftAnchorEnabled() || hint->IsHCenterAnchorEnabled() || hint->IsRightAnchorEnabled())
                {
                    GetAxisDataByAnchorData(rect.dx, parentSize.x,
                                           hint->IsLeftAnchorEnabled(), hint->GetLeftAnchor(),
                                           hint->IsHCenterAnchorEnabled(), hint->GetHCenterAnchor(),
                                           hint->IsRightAnchorEnabled(), hint->GetRightAnchor(),
                                           newRect.x, newRect.dx);
                    
                }
                if (hint->IsTopAnchorEnabled() || hint->IsVCenterAnchorEnabled() || hint->IsBottomAnchorEnabled())
                {
                    GetAxisDataByAnchorData(rect.dy, parentSize.y,
                                           hint->IsTopAnchorEnabled(), hint->GetTopAnchor(),
                                           hint->IsVCenterAnchorEnabled(), hint->GetVCenterAnchor(),
                                           hint->IsBottomAnchorEnabled(), hint->GetBottomAnchor(),
                                           newRect.y, newRect.dy);
                }
                
                if (rect != newRect)
                {
                    child->SetSize(newRect.GetSize());
                    child->SetPosition(newRect.GetPosition() + child->GetPivotPoint());
                }
            }
        }
    }
    
    void UIAnchorLayout::GetAxisDataByAnchorData(float32 size, float32 parentSize,
                                           bool firstSideAnchorEnabled, float32 firstSideAnchor,
                                           bool centerAnchorEnabled, float32 centerAnchor,
                                           bool secondSideAnchorEnabled, float32 secondSideAnchor,
                                           float32 &newPos, float32 &newSize)
    {
        if (firstSideAnchorEnabled && secondSideAnchorEnabled)
        {
            newPos = firstSideAnchor;
            newSize = parentSize - (firstSideAnchor + secondSideAnchor);
        }
        else if (firstSideAnchorEnabled && centerAnchorEnabled)
        {
            newPos = firstSideAnchor;
            newSize = parentSize / 2.0f - (firstSideAnchor - centerAnchor);
        }
        else if (centerAnchorEnabled && secondSideAnchorEnabled)
        {
            newPos = parentSize / 2.0f + centerAnchor;
            newSize = parentSize / 2.0f - (centerAnchor + secondSideAnchor);
        }
        else if (firstSideAnchorEnabled)
        {
            newPos = firstSideAnchor;
            newSize = size;
        }
        else if (secondSideAnchorEnabled)
        {
            newPos = parentSize - (size + secondSideAnchor);
            newSize = size;
        }
        else if (centerAnchorEnabled)
        {
            newPos = (parentSize - size) / 2.0f + centerAnchor;
            newSize = size;
        }
    }
    
    void UIAnchorLayout::GetAnchorDataByAxisData(float32 size, float32 pos, float32 parentSize, bool firstSideAnchorEnabled, bool centerAnchorEnabled, bool secondSideAnchorEnabled, float32 &firstSideAnchor, float32 &centerAnchor, float32 &secondSideAnchor)
    {
        if (firstSideAnchorEnabled && secondSideAnchorEnabled)
        {
            firstSideAnchor = pos;
            secondSideAnchor = parentSize - (pos + size);
        }
        else if (firstSideAnchorEnabled && centerAnchorEnabled)
        {
            firstSideAnchor = pos;
            centerAnchor = pos + size - parentSize / 2.0f;
        }
        else if (centerAnchorEnabled && secondSideAnchorEnabled)
        {
            centerAnchor = pos - parentSize / 2.0f;
            secondSideAnchor = parentSize - (pos + size);
        }
        else if (firstSideAnchorEnabled)
        {
            firstSideAnchor = pos;
        }
        else if (secondSideAnchorEnabled)
        {
            secondSideAnchor = parentSize - (pos + size);
        }
        else if (centerAnchorEnabled)
        {
            centerAnchor = pos - parentSize / 2.0f + size / 2.0f;
        }
    }

}
