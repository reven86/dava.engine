#include "UI/Styles/UIStyleSheetPropertyDataBase.h"
#include "UI/UIControl.h"
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
    , properties({ { UIStyleSheetPropertyDescriptor(&controlGroup, FastName("angle"), 0.0f, ReflectedTypeDB::Get<UIControl>(), "angle"),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("scale"), Vector2(1.0f, 1.0f), ReflectedTypeDB::Get<UIControl>(), "scale"),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("visible"), true, ReflectedTypeDB::Get<UIControl>(), "visible"),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("noInput"), false, ReflectedTypeDB::Get<UIControl>(), "noInput"),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("exclusiveInput"), false, ReflectedTypeDB::Get<UIControl>(), "exclusiveInput"),

                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("drawType"), UIControlBackground::DRAW_ALIGNED, ReflectedTypeDB::Get<UIControlBackground>(), "drawType"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("sprite"), FilePath(), ReflectedTypeDB::Get<UIControlBackground>(), "sprite"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("frame"), 0, ReflectedTypeDB::Get<UIControlBackground>(), "frame"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("mask"), FilePath(), ReflectedTypeDB::Get<UIControlBackground>(), "mask"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("detail"), FilePath(), ReflectedTypeDB::Get<UIControlBackground>(), "detail"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("gradient"), FilePath(), ReflectedTypeDB::Get<UIControlBackground>(), "gradient"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("contour"), FilePath(), ReflectedTypeDB::Get<UIControlBackground>(), "contour"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("gradientMode"), eGradientBlendMode::GRADIENT_MULTIPLY, ReflectedTypeDB::Get<UIControlBackground>(), "gradientMode"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("spriteModification"), 0, ReflectedTypeDB::Get<UIControlBackground>(), "spriteModification"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("color"), Color::White, ReflectedTypeDB::Get<UIControlBackground>(), "color"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("colorInherit"), UIControlBackground::COLOR_IGNORE_PARENT, ReflectedTypeDB::Get<UIControlBackground>(), "colorInherit"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("align"), ALIGN_HCENTER | ALIGN_VCENTER, ReflectedTypeDB::Get<UIControlBackground>(), "align"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("leftRightStretchCap"), 0.0f, ReflectedTypeDB::Get<UIControlBackground>(), "leftRightStretchCap"),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("topBottomStretchCap"), 0.0f, ReflectedTypeDB::Get<UIControlBackground>(), "topBottomStretchCap"),

                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("font"), String(""), ReflectedTypeDB::Get<UIStaticText>(), "font"),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("textColor"), Color::White, ReflectedTypeDB::Get<UIStaticText>(), "textColor"),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("textcolorInheritType"), UIControlBackground::COLOR_MULTIPLY_ON_PARENT, ReflectedTypeDB::Get<UIStaticText>(), "textcolorInheritType"),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("shadowoffset"), Vector2(0.0f, 0.0f), ReflectedTypeDB::Get<UIStaticText>(), "shadowoffset"),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("shadowcolor"), Color::White, ReflectedTypeDB::Get<UIStaticText>(), "shadowcolor"),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("textalign"), ALIGN_HCENTER | ALIGN_VCENTER, ReflectedTypeDB::Get<UIStaticText>(), "textalign"),

                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("enabled"), true, ReflectedTypeDB::Get<UILinearLayoutComponent>(), "enabled"),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("orientation"), UILinearLayoutComponent::LEFT_TO_RIGHT, ReflectedTypeDB::Get<UILinearLayoutComponent>(), "orientation"),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("padding"), 0.0f, ReflectedTypeDB::Get<UILinearLayoutComponent>(), "padding"),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("dynamicPadding"), false, ReflectedTypeDB::Get<UILinearLayoutComponent>(), "dynamicPadding"),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("spacing"), 0.0f, ReflectedTypeDB::Get<UILinearLayoutComponent>(), "spacing"),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("dynamicSpacing"), false, ReflectedTypeDB::Get<UILinearLayoutComponent>(), "dynamicSpacing"),

                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("enabled"), true, ReflectedTypeDB::Get<UIFlowLayoutComponent>(), "enabled"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("orientation"), UIFlowLayoutComponent::ORIENTATION_LEFT_TO_RIGHT, ReflectedTypeDB::Get<UIFlowLayoutComponent>(), "orientation"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hPadding"), 0.0f, ReflectedTypeDB::Get<UIFlowLayoutComponent>(), "hPadding"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hDynamicPadding"), false, ReflectedTypeDB::Get<UIFlowLayoutComponent>(), "hDynamicPadding"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hDynamicInLinePadding"), false, ReflectedTypeDB::Get<UIFlowLayoutComponent>(), "hDynamicInLinePadding"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hSpacing"), 0.0f, ReflectedTypeDB::Get<UIFlowLayoutComponent>(), "hSpacing"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hDynamicSpacing"), false, ReflectedTypeDB::Get<UIFlowLayoutComponent>(), "hDynamicSpacing"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vPadding"), 0.0f, ReflectedTypeDB::Get<UIFlowLayoutComponent>(), "vPadding"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vDynamicPadding"), false, ReflectedTypeDB::Get<UIFlowLayoutComponent>(), "vDynamicPadding"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vSpacing"), 0.0f, ReflectedTypeDB::Get<UIFlowLayoutComponent>(), "vSpacing"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vDynamicSpacing"), false, ReflectedTypeDB::Get<UIFlowLayoutComponent>(), "vDynamicSpacing"),

                     UIStyleSheetPropertyDescriptor(&flowLayoutHintGroup, FastName("newLineBeforeThis"), false, ReflectedTypeDB::Get<UIFlowLayoutHintComponent>(), "newLineBeforeThis"),
                     UIStyleSheetPropertyDescriptor(&flowLayoutHintGroup, FastName("newLineAfterThis"), false, ReflectedTypeDB::Get<UIFlowLayoutHintComponent>(), "newLineAfterThis"),

                     UIStyleSheetPropertyDescriptor(&ignoreLayoutGroup, FastName("enabled"), true, ReflectedTypeDB::Get<UIIgnoreLayoutComponent>(), "enabled"),

                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalPolicy"), UISizePolicyComponent::IGNORE_SIZE, ReflectedTypeDB::Get<UISizePolicyComponent>(), "horizontalPolicy"),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalValue"), 100.0f, ReflectedTypeDB::Get<UISizePolicyComponent>(), "horizontalValue"),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalMin"), 0.0f, ReflectedTypeDB::Get<UISizePolicyComponent>(), "horizontalMin"),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalMax"), 99999.0f, ReflectedTypeDB::Get<UISizePolicyComponent>(), "horizontalMax"),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalPolicy"), UISizePolicyComponent::IGNORE_SIZE, ReflectedTypeDB::Get<UISizePolicyComponent>(), "verticalPolicy"),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalValue"), 100.0f, ReflectedTypeDB::Get<UISizePolicyComponent>(), "verticalValue"),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalMin"), 0.0f, ReflectedTypeDB::Get<UISizePolicyComponent>(), "verticalMin"),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalMax"), 99999.0f, ReflectedTypeDB::Get<UISizePolicyComponent>(), "verticalMax"),

                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("enabled"), true, ReflectedTypeDB::Get<UIAnchorComponent>(), "enabled"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("leftAnchorEnabled"), false, ReflectedTypeDB::Get<UIAnchorComponent>(), "leftAnchorEnabled"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("leftAnchor"), 0.0f, ReflectedTypeDB::Get<UIAnchorComponent>(), "leftAnchor"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("rightAnchorEnabled"), false, ReflectedTypeDB::Get<UIAnchorComponent>(), "rightAnchorEnabled"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("rightAnchor"), 0.0f, ReflectedTypeDB::Get<UIAnchorComponent>(), "rightAnchor"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("bottomAnchorEnabled"), false, ReflectedTypeDB::Get<UIAnchorComponent>(), "bottomAnchorEnabled"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("bottomAnchor"), 0.0f, ReflectedTypeDB::Get<UIAnchorComponent>(), "bottomAnchor"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("topAnchorEnabled"), false, ReflectedTypeDB::Get<UIAnchorComponent>(), "topAnchorEnabled"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("topAnchor"), 0.0f, ReflectedTypeDB::Get<UIAnchorComponent>(), "topAnchor"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("hCenterAnchorEnabled"), false, ReflectedTypeDB::Get<UIAnchorComponent>(), "hCenterAnchorEnabled"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("hCenterAnchor"), 0.0f, ReflectedTypeDB::Get<UIAnchorComponent>(), "hCenterAnchor"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("vCenterAnchorEnabled"), false, ReflectedTypeDB::Get<UIAnchorComponent>(), "vCenterAnchorEnabled"),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("vCenterAnchor"), 0.0f, ReflectedTypeDB::Get<UIAnchorComponent>(), "vCenterAnchor") } })
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

int32 UIStyleSheetPropertyDataBase::FindStyleSheetPropertyByField(const DAVA::ReflectedStructure::Field* field) const
{
    for (size_t index = 0; index < properties.size(); index++)
    {
        const UIStyleSheetPropertyDescriptor& descr = properties[index];
        if (descr.field_s == field)
        {
            return static_cast<int32>(index);
        }
    }
    return -1;
}
    
}
