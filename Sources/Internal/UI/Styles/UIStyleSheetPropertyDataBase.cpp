#include "UI/Styles/UIStyleSheetPropertyDataBase.h"
#include "UI/UIControl.h"
#include "UI/UIButton.h"
#include "UI/UIStaticText.h"
#include "UI/UITextField.h"
#include "UI/UIControlBackground.h"
#include "UI/Layouts/UILinearLayoutComponent.h"
#include "UI/Layouts/UIFlowLayoutComponent.h"
#include "UI/Layouts/UIFlowLayoutHintComponent.h"
#include "UI/Layouts/UIIgnoreLayoutComponent.h"
#include "UI/Layouts/UISizePolicyComponent.h"
#include "UI/Layouts/UIAnchorComponent.h"

namespace DAVA
{
UIStyleSheetPropertyDataBase::UIStyleSheetPropertyDataBase()
    : controlGroup("", ePropertyOwner::CONTROL, 0)
    , bgGroup("bg", ePropertyOwner::BACKGROUND, 0)
    , staticTextGroup("text", ePropertyOwner::CONTROL, 0)
    , textFieldGroup("textField", ePropertyOwner::CONTROL, 0)
    , linearLayoutGroup("linearLayout", ePropertyOwner::COMPONENT, UIComponent::LINEAR_LAYOUT_COMPONENT)
    , flowLayoutGroup("flowLayout", ePropertyOwner::COMPONENT, UIComponent::FLOW_LAYOUT_COMPONENT)
    , flowLayoutHintGroup("flowLayoutHint", ePropertyOwner::COMPONENT, UIComponent::FLOW_LAYOUT_HINT_COMPONENT)
    , ignoreLayoutGroup("ignoreLayout", ePropertyOwner::COMPONENT, UIComponent::IGNORE_LAYOUT_COMPONENT)
    , sizePolicyGroup("sizePolicy", ePropertyOwner::COMPONENT, UIComponent::SIZE_POLICY_COMPONENT)
    , anchorGroup("anchor", ePropertyOwner::COMPONENT, UIComponent::ANCHOR_COMPONENT)
    , properties({ { UIStyleSheetPropertyDescriptor(&controlGroup, FastName("angle"), 0.0f, "angle"),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("scale"), Vector2(1.0f, 1.0f), "scale"),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("visible"), true, "visible"),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("noInput"), false, "noInput"),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("exclusiveInput"), false, "exclusiveInput"),

                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("drawType"), UIControlBackground::DRAW_ALIGNED, "drawType"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("sprite"), FilePath(), "sprite"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("frame"), 0, "frame"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("mask"), FilePath(), "mask"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("detail"), FilePath(), "detail"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("gradient"), FilePath(), "gradient"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("contour"), FilePath(), "contour"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("gradientMode"), eGradientBlendMode::GRADIENT_MULTIPLY, "gradientMode"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("spriteModification"), 0, "spriteModification"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("color"), Color::White, "color"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("colorInherit"), UIControlBackground::COLOR_IGNORE_PARENT, "colorInherit"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("align"), ALIGN_HCENTER | ALIGN_VCENTER, "align"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("leftRightStretchCap"), 0.0f, "leftRightStretchCap"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("topBottomStretchCap"), 0.0f, "topBottomStretchCap"),

                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("font"), String(""), "font"),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("textColor"), Color::White, "textColor"),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("textcolorInheritType"), UIControlBackground::COLOR_MULTIPLY_ON_PARENT, "textcolorInheritType"),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("shadowoffset"), Vector2(0.0f, 0.0f), "shadowoffset"),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("shadowcolor"), Color::White, "shadowcolor"),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("textalign"), ALIGN_HCENTER | ALIGN_VCENTER, "textalign"),

                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("enabled"), true, "enabled"),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("orientation"), UILinearLayoutComponent::LEFT_TO_RIGHT, "orientation"),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("padding"), 0.0f, "padding"),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("dynamicPadding"), false, "dynamicPadding"),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("spacing"), 0.0f, "spacing"),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("dynamicSpacing"), false, "dynamicSpacing"),

                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("enabled"), true, "enabled"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("orientation"), UIFlowLayoutComponent::ORIENTATION_LEFT_TO_RIGHT, "orientation"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hPadding"), 0.0f, "hPadding"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hDynamicPadding"), false, "hDynamicPadding"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hDynamicInLinePadding"), false, "hDynamicInLinePadding"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hSpacing"), 0.0f, "hSpacing"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hDynamicSpacing"), false, "hDynamicSpacing"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vPadding"), 0.0f, "vPadding"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vDynamicPadding"), false, "vDynamicPadding"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vSpacing"), 0.0f, "vSpacing"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vDynamicSpacing"), false, "vDynamicSpacing"),

                     UIStyleSheetPropertyDescriptor(&flowLayoutHintGroup, FastName("newLineBeforeThis"), false, "newLineBeforeThis"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutHintGroup, FastName("newLineAfterThis"), false, "newLineAfterThis"),

                     UIStyleSheetPropertyDescriptor(&ignoreLayoutGroup, FastName("enabled"), true, "enabled"),

                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalPolicy"), UISizePolicyComponent::IGNORE_SIZE, "horizontalPolicy"),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalValue"), 100.0f, "horizontalValue"),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalMin"), 0.0f, "horizontalMin"),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalMax"), 99999.0f, "horizontalMax"),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalPolicy"), UISizePolicyComponent::IGNORE_SIZE, "verticalPolicy"),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalValue"), 100.0f, "verticalValue"),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalMin"), 0.0f, "verticalMin"),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalMax"), 99999.0f, "verticalMax"),

                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("enabled"), true, "enabled"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("leftAnchorEnabled"), false, "leftAnchorEnabled"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("leftAnchor"), 0.0f, "leftAnchor"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("rightAnchorEnabled"), false, "rightAnchorEnabled"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("rightAnchor"), 0.0f, "rightAnchor"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("bottomAnchorEnabled"), false, "bottomAnchorEnabled"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("bottomAnchor"), 0.0f, "bottomAnchor"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("topAnchorEnabled"), false, "topAnchorEnabled"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("topAnchor"), 0.0f, "topAnchor"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("hCenterAnchorEnabled"), false, "hCenterAnchorEnabled"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("hCenterAnchor"), 0.0f, "hCenterAnchor"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("vCenterAnchorEnabled"), false, "vCenterAnchorEnabled"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("vCenterAnchor"), 0.0f, "vCenterAnchor") } })
{
    UnorderedMap<FastName, FastName> legacyNames;
    legacyNames[FastName("bg-drawType")] = FastName("drawType");
    legacyNames[FastName("bg-sprite")] = FastName("sprite");
    legacyNames[FastName("bg-frame")] = FastName("frame");
    legacyNames[FastName("bg-color")] = FastName("color");
    legacyNames[FastName("bg-colorInherit")] = FastName("colorInherit");
    legacyNames[FastName("bg-align")] = FastName("align");
    legacyNames[FastName("bg-leftRightStretchCap")] = FastName("leftRightStretchCap");
    legacyNames[FastName("bg-topBottomStretchCap")] = FastName("topBottomStretchCap");

    legacyNames[FastName("text-font")] = FastName("font");
    legacyNames[FastName("text-textColor")] = FastName("textColor");
    legacyNames[FastName("text-textcolorInheritType")] = FastName("textcolorInheritType");
    legacyNames[FastName("text-shadowoffset")] = FastName("shadowoffset");
    legacyNames[FastName("text-shadowcolor")] = FastName("shadowcolor");
    legacyNames[FastName("text-textalign")] = FastName("textalign");

    legacyNames[FastName("anchor-leftAnchorEnabled")] = FastName("leftAnchorEnabled");
    legacyNames[FastName("anchor-leftAnchor")] = FastName("leftAnchor");
    legacyNames[FastName("anchor-rightAnchorEnabled")] = FastName("rightAnchorEnabled");
    legacyNames[FastName("anchor-rightAnchor")] = FastName("rightAnchor");
    legacyNames[FastName("anchor-bottomAnchorEnabled")] = FastName("bottomAnchorEnabled");
    legacyNames[FastName("anchor-bottomAnchor")] = FastName("bottomAnchor");
    legacyNames[FastName("anchor-topAnchorEnabled")] = FastName("topAnchorEnabled");
    legacyNames[FastName("anchor-topAnchor")] = FastName("topAnchor");
    legacyNames[FastName("anchor-hCenterAnchorEnabled")] = FastName("hCenterAnchorEnabled");
    legacyNames[FastName("anchor-hCenterAnchor")] = FastName("hCenterAnchor");
    legacyNames[FastName("anchor-vCenterAnchorEnabled")] = FastName("vCenterAnchorEnabled");
    legacyNames[FastName("anchor-vCenterAnchor")] = FastName("vCenterAnchor");

    for (int32 propertyIndex = 0; propertyIndex < STYLE_SHEET_PROPERTY_COUNT; propertyIndex++)
    {
        UIStyleSheetPropertyDescriptor& descr = properties[propertyIndex];
        //         for (int32 i = 0; i < descr.group->typeInfo->MembersCount(); i++)
        //         {
        //             const InspMember* member = descr.group->typeInfo->Member(i);
        //             if (member->Name() == descr.name)
        //             {
        //                 DVASSERT(descr.memberInfo == nullptr);
        //                 DVASSERT(descr.defaultValue.GetType() == VariantType::TypeFromMetaInfo(member->Type()));
        //                 descr.memberInfo = member;
        //                 break;
        //             }
        //         }
        //         DVASSERT(descr.memberInfo != nullptr);

        FastName fullName = FastName(descr.GetFullName());
        propertyNameToIndexMap[fullName] = propertyIndex;

        auto legacyNameIt = legacyNames.find(fullName);
        if (legacyNameIt != legacyNames.end())
        {
            propertyNameToIndexMap[legacyNameIt->second] = propertyIndex;
        }
    }

    visiblePropertyIndex = GetStyleSheetPropertyIndex(FastName("visible"));
}

UIStyleSheetPropertyDataBase::~UIStyleSheetPropertyDataBase()
{
}

uint32 UIStyleSheetPropertyDataBase::GetStyleSheetPropertyIndex(const FastName& name) const
{
    const auto& iter = propertyNameToIndexMap.find(name);

    DVASSERT(iter != propertyNameToIndexMap.end());

    return iter->second;
}

bool UIStyleSheetPropertyDataBase::IsValidStyleSheetProperty(const FastName& name) const
{
    return propertyNameToIndexMap.find(name) != propertyNameToIndexMap.end();
}

const UIStyleSheetPropertyDescriptor& UIStyleSheetPropertyDataBase::GetStyleSheetPropertyByIndex(uint32 index) const
{
    return properties[index];
}

// int32 UIStyleSheetPropertyDataBase::FindStyleSheetPropertyByMember(const InspMember* memberInfo) const
// {
//     for (size_t index = 0; index < properties.size(); index++)
//     {
//         const UIStyleSheetPropertyDescriptor& descr = properties[index];
//         if (descr.memberInfo == memberInfo)
//         {
//             return static_cast<int32>(index);
//         }
//     }
//     return -1;
// }
}
