#ifndef __DAVAENGINE_UI_FLOW_LAYOUT_HINT_COMPONENT_H__
#define __DAVAENGINE_UI_FLOW_LAYOUT_HINT_COMPONENT_H__

#include "UI/Components/UIComponent.h"
#include "Reflection/Reflection.h"
#include <bitset>

namespace DAVA
{
class UIControl;

class UIFlowLayoutHintComponent : public UIBaseComponent<UIComponent::FLOW_LAYOUT_HINT_COMPONENT>
{
    DAVA_VIRTUAL_REFLECTION(UIFlowLayoutHintComponent, UIBaseComponent<UIComponent::FLOW_LAYOUT_HINT_COMPONENT>);

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

    std::bitset<eFlags::FLAG_COUNT> flags;
};
}


#endif //__DAVAENGINE_UI_FLOW_LAYOUT_HINT_COMPONENT_H__
