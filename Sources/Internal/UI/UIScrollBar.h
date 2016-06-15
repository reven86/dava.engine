#ifndef __DAVAENGINE_UI_SCROLL_BAR_H__
#define __DAVAENGINE_UI_SCROLL_BAR_H__

#include "Base/BaseTypes.h"
#include "UI/UIControl.h"

#define MINIMUM_SLIDER_SIZE 30

namespace DAVA
{
class UIScrollBar;
class UIScrollBarDelegate
{
public:
    friend class UIScrollBar;
    virtual ~UIScrollBarDelegate() = default;

    virtual float32 VisibleAreaSize(UIScrollBar* forScrollBar) = 0;
    virtual float32 TotalAreaSize(UIScrollBar* forScrollBar) = 0;
    virtual float32 ViewPosition(UIScrollBar* forScrollBar) = 0;
    virtual void OnViewPositionChanged(UIScrollBar* byScrollBar, float32 newPosition) = 0;
    virtual const String GetDelegateControlPath(const UIControl* rootControl) const
    {
        return String();
    }; // TODO!! TEMP SOLUTION, CHECK WITH AUTHOR!
};

class UIScrollBar : public UIControl
{ //TODO: add top and bottom buttons
public:
    enum eScrollOrientation
    {
        ORIENTATION_VERTICAL = 0
        ,
        ORIENTATION_HORIZONTAL
    };

protected:
    virtual ~UIScrollBar();

public:
    UIScrollBar(const Rect& rect = Rect(), eScrollOrientation requiredOrientation = ORIENTATION_VERTICAL);

    void SetDelegate(UIScrollBarDelegate* newDelegate);
    const String GetDelegatePath(const UIControl* rootControl) const;
    UIControl* GetSlider();

    void Draw(const UIGeometricData& geometricData) override;
    void AddControl(UIControl* control) override;
    void RemoveControl(UIControl* control) override;
    UIScrollBar* Clone() override;
    void CopyDataFrom(UIControl* srcControl) override;

    void LoadFromYamlNodeCompleted() override;

    void Input(UIEvent* currentInput) override;

    int32 GetOrientation() const;
    void SetOrientation(int32 value);

protected:
    // Calculate the start offset based on the initial click point.
    void CalculateStartOffset(const Vector2& inputPoint);
    void InitControls(const Rect& rect = Rect());

private:
    eScrollOrientation orientation;
    UIScrollBarDelegate* delegate;

    UIControl* slider;

    bool resizeSliderProportionally;

    Vector2 startPoint;
    Vector2 startOffset;

    float32 GetValidSliderSize(float32 size);

public:
    INTROSPECTION_EXTEND(UIScrollBar, UIControl,
                         PROPERTY("orientation", InspDesc("Bar orientation", GlobalEnumMap<UIScrollBar::eScrollOrientation>::Instance()), GetOrientation, SetOrientation, I_SAVE | I_VIEW | I_EDIT)
                         );
};
};



#endif