#include "UIControlFamily.h"

namespace DAVA
{
// call constructor on first use not globaly
static BaseFamilyRepository<UIControlFamily>& GetUIControlFamilyRepository()
{
    static BaseFamilyRepository<UIControlFamily> repository;
    return repository;
}

UIControlFamily::UIControlFamily(const Vector<UIComponent*>& components)
    : BaseFamily<UIComponent>(components)
{
}

UIControlFamily* UIControlFamily::GetOrCreate(const Vector<UIComponent*>& components)
{
    return GetUIControlFamilyRepository().GetOrCreate(UIControlFamily(components));
}

void UIControlFamily::Release(UIControlFamily*& family)
{
    GetUIControlFamilyRepository().ReleaseFamily(family);
    family = nullptr;
}
}
