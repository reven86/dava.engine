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
#include "UI/Sound/UISoundComponent.h"
#include "Reflection/ReflectedTypeDB.h"

namespace DAVA
{
UIStyleSheetPropertyDataBase::UIStyleSheetPropertyDataBase()
    : controlGroup("", nullptr, ReflectedTypeDB::Get<UIControl>())
    , bgGroup("bg", Type::Instance<UIControlBackground>(), ReflectedTypeDB::Get<UIControlBackground>())
    , staticTextGroup("text", nullptr, ReflectedTypeDB::Get<UIStaticText>())
    , textFieldGroup("textField", nullptr, ReflectedTypeDB::Get<UITextField>())
    , linearLayoutGroup("linearLayout", Type::Instance<UILinearLayoutComponent>(), ReflectedTypeDB::Get<UILinearLayoutComponent>())
    , flowLayoutGroup("flowLayout", Type::Instance<UIFlowLayoutComponent>(), ReflectedTypeDB::Get<UIFlowLayoutComponent>())
    , flowLayoutHintGroup("flowLayoutHint", Type::Instance<UIFlowLayoutHintComponent>(), ReflectedTypeDB::Get<UIFlowLayoutHintComponent>())
    , ignoreLayoutGroup("ignoreLayout", Type::Instance<UIIgnoreLayoutComponent>(), ReflectedTypeDB::Get<UIIgnoreLayoutComponent>())
    , sizePolicyGroup("sizePolicy", Type::Instance<UISizePolicyComponent>(), ReflectedTypeDB::Get<UISizePolicyComponent>())
    , anchorGroup("anchor", Type::Instance<UIAnchorComponent>(), ReflectedTypeDB::Get<UIAnchorComponent>())
    , soundGroup("sound", Type::Instance<UISoundComponent>(), ReflectedTypeDB::Get<UISoundComponent>())
    , properties({ { UIStyleSheetPropertyDescriptor(&controlGroup, FastName("angle"), 0.0f),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("scale"), Vector2(1.0f, 1.0f)),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("pivot"), Vector2(0.0f, 0.0f)),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("visible"), true),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("noInput"), false),
                     UIStyleSheetPropertyDescriptor(&controlGroup, FastName("exclusiveInput"), false),

                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("drawType"), static_cast<int32>(UIControlBackground::DRAW_ALIGNED)),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("sprite"), FilePath()),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("frame"), 0),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("mask"), FilePath()),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("detail"), FilePath()),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("gradient"), FilePath()),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("contour"), FilePath()),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("gradientMode"), eGradientBlendMode::GRADIENT_MULTIPLY),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("spriteModification"), 0),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("color"), Color::White),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("colorInherit"), UIControlBackground::COLOR_IGNORE_PARENT),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("align"), ALIGN_HCENTER | ALIGN_VCENTER),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("leftRightStretchCap"), 0.0f),
                     UIStyleSheetPropertyDescriptor(&bgGroup, FastName("topBottomStretchCap"), 0.0f),

                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("font"), String("")),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("textColor"), Color::White),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("textcolorInheritType"), UIControlBackground::COLOR_MULTIPLY_ON_PARENT),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("shadowoffset"), Vector2(0.0f, 0.0f)),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("shadowcolor"), Color::White),
                     UIStyleSheetPropertyDescriptor(&staticTextGroup, FastName("textalign"), ALIGN_HCENTER | ALIGN_VCENTER),

                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("enabled"), true),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("orientation"), UILinearLayoutComponent::LEFT_TO_RIGHT),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("padding"), 0.0f),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("dynamicPadding"), false),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("spacing"), 0.0f),
                     UIStyleSheetPropertyDescriptor(&linearLayoutGroup, FastName("dynamicSpacing"), false),

                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("enabled"), true),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("orientation"), UIFlowLayoutComponent::ORIENTATION_LEFT_TO_RIGHT),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hPadding"), 0.0f),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hDynamicPadding"), false),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hDynamicInLinePadding"), false),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hSpacing"), 0.0f),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("hDynamicSpacing"), false),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vPadding"), 0.0f),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vDynamicPadding"), false),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vSpacing"), 0.0f),
                     UIStyleSheetPropertyDescriptor(&flowLayoutGroup, FastName("vDynamicSpacing"), false),

                     UIStyleSheetPropertyDescriptor(&flowLayoutHintGroup, FastName("newLineBeforeThis"), false),
                     UIStyleSheetPropertyDescriptor(&flowLayoutHintGroup, FastName("newLineAfterThis"), false),

                     UIStyleSheetPropertyDescriptor(&ignoreLayoutGroup, FastName("enabled"), true),

                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalPolicy"), UISizePolicyComponent::IGNORE_SIZE),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalValue"), 100.0f),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalMin"), 0.0f),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("horizontalMax"), 99999.0f),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalPolicy"), UISizePolicyComponent::IGNORE_SIZE),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalValue"), 100.0f),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalMin"), 0.0f),
                     UIStyleSheetPropertyDescriptor(&sizePolicyGroup, FastName("verticalMax"), 99999.0f),

                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("enabled"), true),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("leftAnchorEnabled"), false),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("leftAnchor"), 0.0f),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("rightAnchorEnabled"), false),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("rightAnchor"), 0.0f),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("bottomAnchorEnabled"), false),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("bottomAnchor"), 0.0f),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("topAnchorEnabled"), false),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("topAnchor"), 0.0f),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("hCenterAnchorEnabled"), false),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("hCenterAnchor"), 0.0f),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("vCenterAnchorEnabled"), false),
                     UIStyleSheetPropertyDescriptor(&anchorGroup, FastName("vCenterAnchor"), 0.0f),

                     UIStyleSheetPropertyDescriptor(&soundGroup, FastName("touchDown"), FastName()),
                     UIStyleSheetPropertyDescriptor(&soundGroup, FastName("touchUpInside"), FastName()),
                     UIStyleSheetPropertyDescriptor(&soundGroup, FastName("touchUpOutside"), FastName()),
                     UIStyleSheetPropertyDescriptor(&soundGroup, FastName("valueChanged"), FastName()) } })
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

int32 UIStyleSheetPropertyDataBase::FindStyleSheetProperty(const Type* componentType, const FastName& name) const
{
    for (size_t index = 0; index < properties.size(); index++)
    {
        const UIStyleSheetPropertyDescriptor& descr = properties[index];
        if (descr.group->componentType == componentType && descr.name == name)
        {
            return static_cast<int32>(index);
        }
    }
    return -1;
}
}
