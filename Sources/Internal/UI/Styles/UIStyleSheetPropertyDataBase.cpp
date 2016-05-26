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
    : controlGroup("", ePropertyOwner::CONTROL, 0, UIControl::TypeInfo())
    , bgGroup("bg", ePropertyOwner::BACKGROUND, 0, UIControlBackground::TypeInfo())
    , staticTextGroup("text", ePropertyOwner::CONTROL, 0, UIStaticText::TypeInfo())
    , textFieldGroup("textField", ePropertyOwner::CONTROL, 0, UITextField::TypeInfo())
    , linearLayoutGroup("linearLayout", ePropertyOwner::COMPONENT, UIComponent::LINEAR_LAYOUT_COMPONENT, UILinearLayoutComponent::TypeInfo())
    , flowLayoutGroup("flowLayout", ePropertyOwner::COMPONENT, UIComponent::FLOW_LAYOUT_COMPONENT, UIFlowLayoutComponent::TypeInfo())
    , flowLayoutHintGroup("flowLayoutHint", ePropertyOwner::COMPONENT, UIComponent::FLOW_LAYOUT_HINT_COMPONENT, UIFlowLayoutHintComponent::TypeInfo())
    , ignoreLayoutGroup("ignoreLayout", ePropertyOwner::COMPONENT, UIComponent::IGNORE_LAYOUT_COMPONENT, UIIgnoreLayoutComponent::TypeInfo())
    , sizePolicyGroup("sizePolicy", ePropertyOwner::COMPONENT, UIComponent::SIZE_POLICY_COMPONENT, UISizePolicyComponent::TypeInfo())
    , anchorGroup("anchor", ePropertyOwner::COMPONENT, UIComponent::ANCHOR_COMPONENT, UIAnchorComponent::TypeInfo())

    , properties({ { UIStyleSheetPropertyDescriptor(&controlGroup, FastName("angle"), VariantType(0.0f)),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("scale"), VariantType(Vector2(1.0f, 1.0f))),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("visible"), VariantType(true)),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("noInput"), VariantType(false)),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("exclusiveInput"), VariantType(false)),

                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("drawType"), VariantType(UIControlBackground::DRAW_ALIGNED)),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("sprite"), VariantType(FilePath())),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("frame"), VariantType(0)),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("mask"), VariantType(FilePath())),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("detail"), VariantType(FilePath())),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("gradient"), VariantType(FilePath())),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("contour"), VariantType(FilePath())),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("gradientMode"), VariantType(eGradientBlendMode::GRADIENT_MULTIPLY)),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("spriteModification"), VariantType(0)),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("color"), VariantType(Color::White)),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("colorInherit"), VariantType(UIControlBackground::COLOR_IGNORE_PARENT)),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("align"), VariantType(ALIGN_HCENTER | ALIGN_VCENTER)),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("leftRightStretchCap"), VariantType(0.0f)),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("topBottomStretchCap"), VariantType(0.0f)),

                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("font"), VariantType(String(""))),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("textColor"), VariantType(Color::White)),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("textcolorInheritType"), VariantType(UIControlBackground::COLOR_MULTIPLY_ON_PARENT)),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("shadowoffset"), VariantType(Vector2(0.0f, 0.0f))),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("shadowcolor"), VariantType(Color::White)),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("textalign"), VariantType(ALIGN_HCENTER | ALIGN_VCENTER)),

                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("enabled"), VariantType(true)),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("orientation"), VariantType(UILinearLayoutComponent::LEFT_TO_RIGHT)),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("padding"), VariantType(0.0f)),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("dynamicPadding"), VariantType(false)),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("spacing"), VariantType(0.0f)),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("dynamicSpacing"), VariantType(false)),

                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("enabled"), VariantType(true)),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("orientation"), VariantType(UIFlowLayoutComponent::ORIENTATION_LEFT_TO_RIGHT)),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hPadding"), VariantType(0.0f)),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hDynamicPadding"), VariantType(false)),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hDynamicInLinePadding"), VariantType(false)),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hSpacing"), VariantType(0.0f)),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hDynamicSpacing"), VariantType(false)),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vPadding"), VariantType(0.0f)),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vDynamicPadding"), VariantType(false)),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vSpacing"), VariantType(0.0f)),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vDynamicSpacing"), VariantType(false)),

                     UIStyleSheetPropertyDescriptor(&flowLayoutHintGroup, FastName("newLineBeforeThis"), VariantType(false)),
                     UIStyleSheetPropertyDescriptor(&flowLayoutHintGroup, FastName("newLineAfterThis"), VariantType(false)),

                     UIStyleSheetPropertyDescriptor(&ignoreLayoutGroup, FastName("enabled"), VariantType(true)),

                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalPolicy"), VariantType(UISizePolicyComponent::IGNORE_SIZE)),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalValue"), VariantType(100.0f)),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalMin"), VariantType(0.0f)),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalMax"), VariantType(99999.0f)),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalPolicy"), VariantType(UISizePolicyComponent::IGNORE_SIZE)),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalValue"), VariantType(100.0f)),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalMin"), VariantType(0.0f)),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalMax"), VariantType(99999.0f)),

                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("enabled"), VariantType(true)),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("leftAnchorEnabled"), VariantType(false)),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("leftAnchor"), VariantType(0.0f)),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("rightAnchorEnabled"), VariantType(false)),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("rightAnchor"), VariantType(0.0f)),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("bottomAnchorEnabled"), VariantType(false)),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("bottomAnchor"), VariantType(0.0f)),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("topAnchorEnabled"), VariantType(false)),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("topAnchor"), VariantType(0.0f)),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("hCenterAnchorEnabled"), VariantType(false)),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("hCenterAnchor"), VariantType(0.0f)),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("vCenterAnchorEnabled"), VariantType(false)),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("vCenterAnchor"), VariantType(0.0f)) } })
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
        for (int32 i = 0; i < descr.group->typeInfo->MembersCount(); i++)
        {
            const InspMember* member = descr.group->typeInfo->Member(i);
            if (member->Name() == descr.name)
            {
                DVASSERT(descr.memberInfo == nullptr);
                DVASSERT(descr.defaultValue.GetType() == VariantType::TypeFromMetaInfo(member->Type()));
                descr.memberInfo = member;
                break;
            }
        }
        DVASSERT(descr.memberInfo != nullptr);

        FastName fullName = FastName(descr.GetFullName());
        propertyNameToIndexMap[fullName] = propertyIndex;

        auto legacyNameIt = legacyNames.find(fullName);
        if (legacyNameIt != legacyNames.end())
        {
            propertyNameToIndexMap[legacyNameIt->second] = propertyIndex;
        }
    }
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

int32 UIStyleSheetPropertyDataBase::FindStyleSheetPropertyByMember(const InspMember* memberInfo) const
{
    for (size_t index = 0; index < properties.size(); index++)
    {
        const UIStyleSheetPropertyDescriptor& descr = properties[index];
        if (descr.memberInfo == memberInfo)
        {
            return static_cast<int32>(index);
        }
    }
    return -1;
}
}
