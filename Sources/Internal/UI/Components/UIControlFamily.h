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
    uint32 GetComponentIndex(int32 runtimeType, uint32 index) const;
    uint32 GetComponentsCount(int32 runtimeType) const;

    static UIControlFamily* GetOrCreate(const Vector<UIComponent*>& components);
    static void Release(UIControlFamily*& family);

    bool operator==(const UIControlFamily& rhs) const;

private:
    UIControlFamily(const Vector<UIComponent*>& components);

    int32 refCount = 0;
    int32 hash = 0;

    Vector<uint32> componentIndices;
    Vector<uint32> componentsCount;

    template <typename EntityFamilyType>
    friend class BaseFamilyRepository;
};
}
