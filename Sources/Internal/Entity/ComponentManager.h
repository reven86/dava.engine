#pragma once
#include "Reflection/Reflection.h"
#include "Reflection/ReflectedTypeDB.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{
/**
Track UIComponents types.
Before any UIComponent is used, it must be added to both Reflection(through registering Reflection permanent name) and ComponentManager(through ComponentManager::RegisterComponent).
For example:
\code
DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(UIFlowLayoutComponent, "FlowLayout");
GetEngineContext()->componentManager->RegisterComponent<UILinearLayoutComponent>();
\endcode
Base engine UIComponents are registered in ReflectionDeclaration.cpp

After registration, UIComponent can be created through UIComponent::CreateByType(const Type* componentType).

UIComponent registration also introduces 'runtimeType' (just integer) for UIComponent. 
'runtimeType' can be used for UIComponents management in UIControl along with its' Type. 'runtimeType' is typically used for optimization (for example, indeces in array). You cannot rely on actual 'runtimeType' value between launches of the application.
*/
class ComponentManager
{
public:
    /** Register new component of specified type 'T'. The behavior is undefined until 'T' is a UIComponent subclass. */
    template <class T>
    void RegisterComponent();

    /** Return total number of registered components. */
    uint32 GetComponentsCount();

    /** Check if specified 'type' was registered as UIComponent. */
    bool IsUIComponent(const Type* type);

    /** Return runtimeType for specified 'type'. The behavior is undefined until 'type' is registered in ComponentManager. */
    int32 GetRuntimeType(const Type* type);

    /** Return reference to sorted vector of registered components types. */
    Vector<const Type*>& GetRegisteredComponents();

private:
    int32 runtimeComponentsCount = 0;
    UnorderedMap<const Type*, int32> typeToRuntimeType;

    Vector<const Type*> sortedTypes;
};

template <class T>
void ComponentManager::RegisterComponent()
{
    bool isUIComponent = std::is_base_of<UIComponent, T>::value;
    if (isUIComponent)
    {
        T::runtimeType = runtimeComponentsCount;
        T::reflectionType = Type::Instance<T>();
        typeToRuntimeType[T::reflectionType] = runtimeComponentsCount;

        sortedTypes.push_back(T::reflectionType);
        std::sort(sortedTypes.begin(), sortedTypes.end(),
                  [](const Type*& a, const Type*& b) -> bool
                  {
                      return ReflectedTypeDB::GetByType(a)->GetPermanentName() < ReflectedTypeDB::GetByType(b)->GetPermanentName();
                  });

        runtimeComponentsCount++;
    }
    else
    {
        throw new std::logic_error("Can only register UIComponents");
    }
}

inline bool ComponentManager::IsUIComponent(const Type* type)
{
    auto it = typeToRuntimeType.find(type);
    return (it != typeToRuntimeType.end());
}

inline int32 ComponentManager::GetRuntimeType(const Type* type)
{
    DVASSERT(IsUIComponent(type));
    return typeToRuntimeType[type];
}

inline Vector<const Type*>& ComponentManager::GetRegisteredComponents()
{
    return sortedTypes;
}

inline uint32 ComponentManager::GetComponentsCount()
{
    return runtimeComponentsCount;
}
}