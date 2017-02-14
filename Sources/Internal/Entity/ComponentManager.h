#pragma once
#include "Reflection/Reflection.h"
#include "UI/Components/UIComponent.h"

namespace DAVA
{
/**

*/
class ComponentManager
{
public:
    template <class T>
    void RegisterComponent();

    uint32 GetComponentsCount();

    int32 GetRuntimeType(const Type* type)
    {
        return typeToRuntimeType[type];
    }

    const UnorderedMap<const Type*, int32>& GetRegisteredTypes()
    {
        return typeToRuntimeType;
    }

private:
    int32 runtimeComponentsCount = 0;
    UnorderedMap<const Type*, int32> typeToRuntimeType;
};

template <class T>
void ComponentManager::RegisterComponent()
{
    bool isUIComponent = std::is_base_of<UIComponent, T>::value;
    if (isUIComponent)
    {
        T::runtimeType = runtimeComponentsCount;
        T::reflectionType = Type::Instance<T>();
        typeToRuntimeType[Type::Instance<T>()] = runtimeComponentsCount;
        runtimeComponentsCount++;
    }
    else
    {
        throw new std::logic_error("Can only register UIComponents");
    }
}

inline uint32 ComponentManager::GetComponentsCount()
{
    return runtimeComponentsCount;
}
}