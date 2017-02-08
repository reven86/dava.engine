#pragma once


#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"
#include "UI/Components/UIComponent.h"
#include "Entity/BaseFamily.h"

namespace DAVA
{
class UIControlFamily
{
public:
    uint32 GetComponentIndex(const Type* componentType, uint32 index) const;
    uint32 GetComponentsCount(const Type* componentType) const;

    static UIControlFamily* GetOrCreate(const Vector<UIComponent*>& components);
    static void Release(UIControlFamily*& family);

    bool operator==(const UIControlFamily& rhs) const;

private:
    UIControlFamily(const Vector<UIComponent*>& components);

    int32 refCount = 0;

    Vector<const Type*> types;
    uint64 hash = 0;

    struct TypeCount
    {
        const Type* type = nullptr;
        uint32 count = 0;
    };
    Vector<TypeCount> typeCounts;

    template <typename EntityFamilyType>
    friend class BaseFamilyRepository;
};
}
