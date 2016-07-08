#ifndef __DAVAENGINE_UI_FLOW_LAYOUT_HINT_COMPONENT_H__
#define __DAVAENGINE_UI_FLOW_LAYOUT_HINT_COMPONENT_H__

#include "UI/Components/UIComponent.h"

namespace DAVA
{
class UIControl;

class UIFlowLayoutHintComponent : public UIBaseComponent<UIComponent::FLOW_LAYOUT_HINT_COMPONENT>
{
public:
    UIFlowLayoutHintComponent();
    UIFlowLayoutHintComponent(const UIFlowLayoutHintComponent& src);

protected:
    virtual ~UIFlowLayoutHintComponent();

private:
    UIFlowLayoutHintComponent& operator=(const UIFlowLayoutHintComponent&) = delete;

public:
    UIFlowLayoutHintComponent* Clone() const override;

    bool IsNewLineBeforeThis() const;
    void SetNewLineBeforeThis(bool flag);

    bool IsNewLineAfterThis() const;
    void SetNewLineAfterThis(bool flag);

private:
    void SetLayoutDirty();

private:
    enum eFlags
    {
        FLAG_NEW_LINE_BEFORE_THIS,
        FLAG_NEW_LINE_AFTER_THIS,
        FLAG_COUNT
    };

    Bitset<eFlags::FLAG_COUNT> flags;

public:
    INTROSPECTION_EXTEND(UIFlowLayoutHintComponent, UIComponent,
                         PROPERTY("newLineBeforeThis", "New Line Before This", IsNewLineBeforeThis, SetNewLineBeforeThis, I_SAVE | I_VIEW | I_EDIT)
                         PROPERTY("newLineAfterThis", "New Line After This", IsNewLineAfterThis, SetNewLineAfterThis, I_SAVE | I_VIEW | I_EDIT))
};
}


#endif //__DAVAENGINE_UI_FLOW_LAYOUT_HINT_COMPONENT_H__
