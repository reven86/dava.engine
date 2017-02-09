#include "UIComponent.h"
#include "UI/UIControl.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(UIComponent)
{
    ReflectionRegistrator<UIComponent>::Begin()
    .Field("control", &UIComponent::GetControl, &UIComponent::SetControl)
    .End();
}

UIComponent::UIComponent()
    : control(nullptr)
{
    //type = ReflectedTypeDB::GetByPointer(this)->GetType();
}

UIComponent::UIComponent(const UIComponent& src)
    : control(nullptr)
{
}

UIComponent::~UIComponent()
{
}

UIComponent& UIComponent::operator=(const UIComponent& src)
{
    return *this;
}

UIComponent* UIComponent::CreateByType(const Type* componentType)
{
    bool isUIComponent = TypeInheritance::CanDownCast(componentType->Pointer(), Type::Instance<UIComponent*>());
    if (isUIComponent)
    {
        const ReflectedType* reflType = ReflectedTypeDB::GetByType(componentType);

        Any obj = reflType->CreateObject(ReflectedType::CreatePolicy::ByPointer);
        return obj.Cast<UIComponent*>();
    }
    else
    {
        throw new std::logic_error("UIComponent::CreateByType can only create UIComponents");
    }
}

RefPtr<UIComponent> UIComponent::SafeCreateByType(const Type* componentType)
{
    return RefPtr<UIComponent>(CreateByType(componentType));
}

bool UIComponent::IsMultiple(const Type* componentType)
{
    return false;
}

RefPtr<UIComponent> UIComponent::SafeClone() const
{
    return RefPtr<UIComponent>(Clone());
}

//const Type* UIComponent::GetType() const
//{
//    return ReflectedTypeDB::GetByPointer(this)->GetType();
//}
}
