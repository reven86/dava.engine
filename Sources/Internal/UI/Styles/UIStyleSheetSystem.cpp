#include "UI/Styles/UIStyleSheetSystem.h"
#include "UI/Styles/UIStyleSheet.h"
#include "UI/UIControl.h"
#include "UI/UIControlPackageContext.h"
#include "UI/Components/UIComponent.h"
#include "Platform/SystemTimer.h"
#include "Animation/LinearPropertyAnimation.h"
#include "Animation/AnimationManager.h"
#include "Logger/Logger.h"

namespace DAVA
{
namespace
{
const int32 PROPERTY_ANIMATION_GROUP_OFFSET = 100000;
}

struct ImmediatePropertySetter
{
    void operator()(UIControl* control, void* targetObject, const InspMember* targetIntrospectionMember) const
    {
        control->StopAnimations(PROPERTY_ANIMATION_GROUP_OFFSET + propertyIndex);
        targetIntrospectionMember->SetValue(targetObject, value);
    }

    uint32 propertyIndex;
    const VariantType& value;
};

struct AnimatedPropertySetter
{
    template <typename T>
    void Animate(UIControl* control, void* targetObject, const InspMember* targetIntrospectionMember, const T& startValue, const T& endValue) const
    {
        const int32 track = PROPERTY_ANIMATION_GROUP_OFFSET + propertyIndex;
        LinearPropertyAnimation<T>* currentAnimation = DynamicTypeCheck<LinearPropertyAnimation<T>*>(AnimationManager::Instance()->FindPlayingAnimation(control, track));

        if (!currentAnimation || currentAnimation->GetEndValue() != endValue)
        {
            if (currentAnimation)
                control->StopAnimations(track);

            if (targetIntrospectionMember->Value(targetObject) != value)
            {
                (new LinearPropertyAnimation<T>(control, targetObject, targetIntrospectionMember, startValue, endValue, time, transitionFunction))->Start(track);
            }
        }
    }

    void operator()(UIControl* control, void* targetObject, const InspMember* targetIntrospectionMember) const
    {
        switch (value.GetType())
        {
        case VariantType::TYPE_VECTOR2:
            Animate<Vector2>(control, targetObject, targetIntrospectionMember, targetIntrospectionMember->Value(targetObject).AsVector2(), value.AsVector2());
            break;
        case VariantType::TYPE_VECTOR3:
            Animate<Vector3>(control, targetObject, targetIntrospectionMember, targetIntrospectionMember->Value(targetObject).AsVector3(), value.AsVector3());
            break;
        case VariantType::TYPE_VECTOR4:
            Animate<Vector4>(control, targetObject, targetIntrospectionMember, targetIntrospectionMember->Value(targetObject).AsVector4(), value.AsVector4());
            break;
        case VariantType::TYPE_FLOAT:
            Animate<float32>(control, targetObject, targetIntrospectionMember, targetIntrospectionMember->Value(targetObject).AsFloat(), value.AsFloat());
            break;
        case VariantType::TYPE_COLOR:
            Animate<Color>(control, targetObject, targetIntrospectionMember, targetIntrospectionMember->Value(targetObject).AsColor(), value.AsColor());
            break;
        default:
            DVASSERT(false, "Non-animatable property");
        }
    }

    uint32 propertyIndex;
    const VariantType& value;
    Interpolation::FuncType transitionFunction;
    float32 time;
};

UIStyleSheetSystem::UIStyleSheetSystem()
{
}

UIStyleSheetSystem::~UIStyleSheetSystem()
{
}

void UIStyleSheetSystem::ProcessControl(UIControl* control, bool styleSheetListChanged /* = false*/)
{
#if STYLESHEET_STATS
    uint64 startTime = SystemTimer::Instance()->GetAbsoluteUs();
#endif
    ProcessControl(control, 0, styleSheetListChanged, true, false, nullptr);
#if STYLESHEET_STATS
    statsTime += SystemTimer::Instance()->GetAbsoluteUs() - startTime;
#endif
}

void UIStyleSheetSystem::DebugControl(UIControl* control, UIStyleSheetProcessDebugData* debugData)
{
    ProcessControl(control, 0, true, false, true, debugData);
}

void UIStyleSheetSystem::ProcessControl(UIControl* control, int32 distanceFromDirty, bool styleSheetListChanged, bool recursively, bool dryRun, UIStyleSheetProcessDebugData* debugData)
{
    UIControlPackageContext* packageContext = control->GetPackageContext();
    const UIStyleSheetPropertyDataBase* propertyDB = UIStyleSheetPropertyDataBase::Instance();

    if (control->IsStyleSheetDirty())
    {
        distanceFromDirty = 0;
    }

    if (packageContext
        && (styleSheetListChanged || distanceFromDirty < packageContext->GetMaxStyleSheetHierarchyDepth()))
    {
#if STYLESHEET_STATS
        ++statsProcessedControls;
#endif

        UIStyleSheetPropertySet cascadeProperties;
        const UIStyleSheetPropertySet localControlProperties = control->GetLocalPropertySet();
        const Vector<UIPriorityStyleSheet>& styleSheets = packageContext->GetSortedStyleSheets();
        
#if STYLESHEET_STATS
        statsStyleSheetCount += styleSheets.size();
#endif

        Array<const UIStyleSheetProperty*, UIStyleSheetPropertyDataBase::STYLE_SHEET_PROPERTY_COUNT> propertySources = {};

        for (auto styleSheetIter = styleSheets.rbegin(); styleSheetIter != styleSheets.rend(); ++styleSheetIter)
        {
            const UIStyleSheet* styleSheet = styleSheetIter->GetStyleSheet();

            if (StyleSheetMatchesControl(styleSheet, control))
            {
                cascadeProperties |= styleSheet->GetPropertyTable()->GetPropertySet();

                const Vector<UIStyleSheetProperty>& propertyTable = styleSheet->GetPropertyTable()->GetProperties();
                for (const UIStyleSheetProperty& prop : propertyTable)
                {
                    propertySources[prop.propertyIndex] = &prop;

                    if (debugData != nullptr)
                    {
                        debugData->propertySources[prop.propertyIndex] = styleSheet;
                    }
                }

                if (debugData != nullptr)
                {
                    debugData->styleSheets.push_back(*styleSheetIter);
                }
            }
        }

        const UIStyleSheetPropertySet propertiesToApply = cascadeProperties & (~localControlProperties);
        if (debugData != nullptr)
        {
            debugData->appliedProperties = propertiesToApply;
        }

        const UIStyleSheetPropertySet propertiesToReset = control->GetStyledPropertySet() & (~propertiesToApply) & (~localControlProperties);

        if ((propertiesToReset.any() || propertiesToApply.any())
            && !dryRun)
        {
            for (uint32 propertyIndex = 0; propertyIndex < propertySources.size(); ++propertyIndex)
            {
                if (propertiesToApply.test(propertyIndex))
                {
                    const UIStyleSheetProperty* prop = propertySources[propertyIndex];

                    if (prop->transition && control->IsStyleSheetInitialized())
                    {
                        DoForAllPropertyInstances(control, propertyIndex, AnimatedPropertySetter{ propertyIndex, prop->value, prop->transitionFunction, prop->transitionTime });
                    }
                    else
                    {
                        DoForAllPropertyInstances(control, propertyIndex, ImmediatePropertySetter{ propertyIndex, prop->value });
                    }
                }

                if (propertiesToReset.test(propertyIndex))
                {
                    const UIStyleSheetPropertyDescriptor& propertyDescr = propertyDB->GetStyleSheetPropertyByIndex(propertyIndex);
                    DoForAllPropertyInstances(control, propertyIndex, ImmediatePropertySetter{ propertyIndex, propertyDescr.defaultValue });
                }
            }
        }

        control->SetStyledPropertySet(propertiesToApply);
    }

    control->ResetStyleSheetDirty();
    control->SetStyleSheetInitialized();

    if (recursively)
    {
        for (UIControl* child : control->GetChildren())
        {
            ProcessControl(child, distanceFromDirty + 1, styleSheetListChanged, true, dryRun, debugData);
        }
    }
}

void UIStyleSheetSystem::AddGlobalClass(const FastName& clazz)
{
    globalClasses.AddClass(clazz);
}

void UIStyleSheetSystem::RemoveGlobalClass(const FastName& clazz)
{
    globalClasses.RemoveClass(clazz);
}

bool UIStyleSheetSystem::HasGlobalClass(const FastName& clazz) const
{
    return globalClasses.HasClass(clazz);
}

void UIStyleSheetSystem::SetGlobalTaggedClass(const FastName& tag, const FastName& clazz)
{
    globalClasses.SetTaggedClass(tag, clazz);
}

FastName UIStyleSheetSystem::GetGlobalTaggedClass(const FastName& tag) const
{
    return globalClasses.GetTaggedClass(tag);
}

void UIStyleSheetSystem::ResetGlobalTaggedClass(const FastName& tag)
{
    globalClasses.ResetTaggedClass(tag);
}

void UIStyleSheetSystem::ClearGlobalClasses()
{
    globalClasses.RemoveAllClasses();
}

void UIStyleSheetSystem::ClearStats()
{
    statsTime = 0;
    statsProcessedControls = 0;
    statsMatches = 0;
    statsStyleSheetCount = 0;
}

void UIStyleSheetSystem::DumpStats()
{
    if (statsProcessedControls > 0)
    {
        Logger::Debug("%s %i %f %i %f", __FUNCTION__, statsProcessedControls,
                      static_cast<float>(statsTime / 1000000.0f), statsMatches,
                      static_cast<float>(statsStyleSheetCount / statsProcessedControls));
    }
}

bool UIStyleSheetSystem::StyleSheetMatchesControl(const UIStyleSheet* styleSheet, const UIControl* control)
{
#if STYLESHEET_STATS
    ++statsMatches;
#endif

    const UIControl* currentControl = control;

    auto endIter = styleSheet->GetSelectorChain().rend();
    for (auto selectorIter = styleSheet->GetSelectorChain().rbegin(); selectorIter != endIter; ++selectorIter)
    {
        if (!currentControl || !SelectorMatchesControl(*selectorIter, currentControl))
            return false;

        currentControl = currentControl->GetParent();
    }

    return true;
}

bool UIStyleSheetSystem::SelectorMatchesControl(const UIStyleSheetSelector& selector, const UIControl* control)
{
    if (((selector.stateMask & control->GetState()) != selector.stateMask) || (selector.name.IsValid() && selector.name != control->GetName()) || (!selector.className.empty() && selector.className != control->GetClassName()))
        return false;

    for (const FastName& clazz : selector.classes)
    {
        if (!control->HasClass(clazz)
            && !HasGlobalClass(clazz))
            return false;
    }

    return true;
}

template <typename CallbackType>
void UIStyleSheetSystem::DoForAllPropertyInstances(UIControl* control, uint32 propertyIndex, const CallbackType& action)
{
    const UIStyleSheetPropertyDataBase* propertyDB = UIStyleSheetPropertyDataBase::Instance();

    const UIStyleSheetPropertyDescriptor& descr = propertyDB->GetStyleSheetPropertyByIndex(propertyIndex);

    switch (descr.group->propertyOwner)
    {
    case ePropertyOwner::CONTROL:
    {
        const InspInfo* typeInfo = control->GetTypeInfo();
        do
        {
            if (typeInfo == descr.group->typeInfo)
            {
                action(control, control, descr.memberInfo);
                break;
            }
            typeInfo = typeInfo->BaseInfo();
        } while (typeInfo);

        break;
    }
    case ePropertyOwner::COMPONENT:
        if (UIComponent* component = control->GetComponent(descr.group->componentType))
            action(control, component, descr.memberInfo);
        break;
    default:
        DVASSERT(false);
        break;
    }
}
}
