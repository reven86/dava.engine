#include "UI/Styles/UIStyleSheetSystem.h"
#include "UI/Styles/UIStyleSheet.h"
#include "UI/UIControl.h"
#include "UI/UIControlPackageContext.h"
#include "UI/Components/UIComponent.h"
#include "Time/SystemTimer.h"
#include "Animation/LinearPropertyAnimation.h"
#include "Animation/AnimationManager.h"
#include "Logger/Logger.h"
#include "Render/Renderer.h"
#include "UI/UIScreen.h"
#include "UI/UIScreenTransition.h"

namespace DAVA
{
namespace
{
const int32 PROPERTY_ANIMATION_GROUP_OFFSET = 100000;
}

struct ImmediatePropertySetter
{
    void operator()(UIControl* control, const Reflection& ref) const
    {
        control->StopAnimations(PROPERTY_ANIMATION_GROUP_OFFSET + propertyIndex);
        ref.SetValue(value);
    }

    uint32 propertyIndex;
    const Any& value;
};

struct AnimatedPropertySetter
{
    template <typename T>
    void Animate(UIControl* control, const Reflection& ref, const T& startValue, const T& endValue) const
    {
        const int32 track = PROPERTY_ANIMATION_GROUP_OFFSET + propertyIndex;
        LinearPropertyAnimation<T>* currentAnimation = DynamicTypeCheck<LinearPropertyAnimation<T>*>(AnimationManager::Instance()->FindPlayingAnimation(control, track));

        if (!currentAnimation || currentAnimation->GetEndValue() != endValue)
        {
            if (currentAnimation)
                control->StopAnimations(track);

            if (ref.GetValue() != value)
            {
                (new LinearPropertyAnimation<T>(control, ref, startValue, endValue, time, transitionFunction))->Start(track);
            }
        }
    }

    void operator()(UIControl* control, const Reflection& ref) const
    {
        const Any& refValue = ref.GetValue();
        const Type* valueType = value.GetType()->Decay();
        if (valueType == refValue.GetType()->Decay())
        {
            if (valueType == Type::Instance<Vector2>())
            {
                Animate<Vector2>(control, ref, refValue.Get<Vector2>(), value.Get<Vector2>());
            }
            else if (valueType == Type::Instance<Vector3>())
            {
                Animate<Vector3>(control, ref, refValue.Get<Vector3>(), value.Get<Vector3>());
            }
            else if (valueType == Type::Instance<Vector4>())
            {
                Animate<Vector4>(control, ref, refValue.Get<Vector4>(), value.Get<Vector4>());
            }
            else if (valueType == Type::Instance<float32>())
            {
                Animate<float32>(control, ref, refValue.Get<float32>(), value.Get<float32>());
            }
            else if (valueType == Type::Instance<Color>())
            {
                Animate<Color>(control, ref, refValue.Get<Color>(), value.Get<Color>());
            }
            else
            {
                DVASSERT(false, "Non-animatable property");
            }
        }
        else
        {
            DVASSERT(false, "Different types");
        }
    }

    uint32 propertyIndex;
    const Any& value;
    Interpolation::FuncType transitionFunction;
    float32 time;
};

UIStyleSheetSystem::UIStyleSheetSystem()
{
}

UIStyleSheetSystem::~UIStyleSheetSystem()
{
}

void UIStyleSheetSystem::Process(DAVA::float32 elapsedTime)
{
    if (!Renderer::GetOptions()->IsOptionEnabled(RenderOptions::UPDATE_UI_CONTROL_SYSTEM))
    {
        return;
    }

    CheckDirty();

    if (currentScreenTransition.Valid())
    {
        Update(currentScreenTransition.Get());
    }
    else if (currentScreen.Valid())
    {
        Update(currentScreen.Get());
    }

    if (popupContainer.Valid())
    {
        Update(popupContainer.Get());
    }
}

void UIStyleSheetSystem::SetCurrentScreen(const RefPtr<UIScreen>& screen)
{
    currentScreen = screen;
}

void UIStyleSheetSystem::SetCurrentScreenTransition(const RefPtr<UIScreenTransition>& screenTransition)
{
    currentScreenTransition = screenTransition;
}

void UIStyleSheetSystem::SetPopupContainer(const RefPtr<UIControl>& _popupContainer)
{
    popupContainer = _popupContainer;
}

void UIStyleSheetSystem::ProcessControl(UIControl* control, bool styleSheetListChanged /* = false*/)
{
#if STYLESHEET_STATS
    uint64 startTime = SystemTimer::GetUs();
#endif
    ProcessControl(control, 0, styleSheetListChanged, true, false, nullptr);
#if STYLESHEET_STATS
    statsTime += SystemTimer::GetUs() - startTime;
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

void UIStyleSheetSystem::Update(UIControl* root)
{
    if (!(needUpdate || dirty) || !root)
        return;
    UpdateControl(root);
}

void UIStyleSheetSystem::UpdateControl(UIControl* control)
{
    if ((control->IsVisible() || control->GetStyledPropertySet().test(UIStyleSheetPropertyDataBase::Instance()->GetStyleSheetVisiblePropertyIndex()))
        && control->IsStyleSheetDirty())
    {
        ProcessControl(control);
    }

    for (UIControl* child : control->GetChildren())
    {
        UpdateControl(child);
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

    if (descr.group->componentType == -1)
    {
        Reflection ref = Reflection::Create(ReflectedObject(control));
        ref = ref.GetField(descr.field->name);
        if (ref.IsValid())
        {
            action(control, ref);
        }
    }
    else
    {
        if (UIComponent* component = control->GetComponent(descr.group->componentType))
        {
            Reflection ref = Reflection::Create(ReflectedObject(component));
            ref = ref.GetField(descr.field->name);
            if (ref.IsValid())
            {
                action(control, ref);
            }
        }
        else
        {
            const char* componentName = GlobalEnumMap<UIComponent::eType>::Instance()->ToString(descr.group->componentType);
            const char* controlName = control->GetName().c_str();
            Logger::Warning("Style sheet can not find component \'%s\' in control \'%s\'", componentName, controlName);
        }
    }
}
}
