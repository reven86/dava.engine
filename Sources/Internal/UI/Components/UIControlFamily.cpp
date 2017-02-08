#include "UIControlFamily.h"

namespace DAVA
{
// call constructor on first use not globally
BaseFamilyRepository<UIControlFamily>& GetUIControlFamilyRepository()
{
    static BaseFamilyRepository<UIControlFamily> repository;
    return repository;
}

UIControlFamily* UIControlFamily::GetOrCreate(const Vector<UIComponent*>& components)
{
    return GetUIControlFamilyRepository().GetOrCreate(UIControlFamily(components));
}

UIControlFamily::UIControlFamily(const Vector<UIComponent*>& components)
    : refCount(0)
{
    types.reserve(components.size());
    for (UIComponent* c : components)
    {
        types.push_back(c->GetType());
    }
    hash = types.size();
    TypeCount tc;
    for (const Type* t : types)
    {
        hash ^= reinterpret_cast<uint64>(t) + 0x9e3779b9 + (hash << 6) + (hash >> 2);
        if (t != tc.type)
        {
            if (tc.type == nullptr)
            {
                //first iteration
                tc.count = 1;
                tc.type = t;
            }
            else
            {
                //dump previous type
                typeCounts.push_back(tc);
                tc.type = t;
                tc.count = 1;
            }
        }
        else
        {
            tc.count++;
        }
    }
    if (tc.type != nullptr)
    {
        typeCounts.push_back(tc);
    }
}

uint32 UIControlFamily::GetComponentIndex(const Type* componentType, uint32 index) const
{
    DVASSERT(GetComponentsCount(componentType) >= index);

    size_t size = types.size();
    for (size_t i = 0; i < size; ++i)
    {
        if (types[i] == componentType)
        {
            return static_cast<uint32>(i + index);
        }
    }
    throw std::logic_error("componentType no found");
}

uint32 UIControlFamily::GetComponentsCount(const Type* componentType) const
{
    uint32 ret = 0;
    for (const TypeCount& tc : typeCounts)
    {
        if (tc.type == componentType)
        {
            ret = tc.count;
            break;
        }
    }

    return ret;
}

void UIControlFamily::Release(UIControlFamily*& family)
{
    GetUIControlFamilyRepository().ReleaseFamily(family);
    family = nullptr;
}

bool UIControlFamily::operator==(const UIControlFamily& rhs) const
{
    return (hash == rhs.hash) && (types == rhs.types);
}
}
