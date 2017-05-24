#ifndef __DAVAENGINE_UI_FLOW_LAYOUT_HINT_COMPONENT_H__
#define __DAVAENGINE_UI_FLOW_LAYOUT_HINT_COMPONENT_H__

#include "UI/Components/UIComponent.h"
#include "Reflection/Reflection.h"
#include "Utils/BiDiHelper.h"
#include <bitset>

namespace DAVA
{
class UIControl;

class UIFlowLayoutHintComponent : public UIComponent
{
    DAVA_VIRTUAL_REFLECTION(UIFlowLayoutHintComponent, UIComponent);
    IMPLEMENT_UI_COMPONENT(UIFlowLayoutHintComponent);

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

    BiDiHelper::Direction GetContentDirection() const;
    void SetContentDirection(BiDiHelper::Direction direction);

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
    BiDiHelper::Direction contentDirection = BiDiHelper::Direction::NEUTRAL;
};
}


#endif //__DAVAENGINE_UI_FLOW_LAYOUT_HINT_COMPONENT_H__
