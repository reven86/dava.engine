/*==================================================================================
 Copyright (c) 2008, binaryzebra
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.
 * Neither the name of the binaryzebra nor the
 names of its contributors may be used to endorse or promote products
 derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 =====================================================================================*/
#ifndef __DAVAENGINE_BASE_FAMILY_H__
#define __DAVAENGINE_BASE_FAMILY_H__

#include "Base/BaseTypes.h"

namespace DAVA
{
    
////////////////////////////////////////////////////////////////////////////////
// BaseFamily
////////////////////////////////////////////////////////////////////////////////

template <typename Component>
class BaseFamily
{
public:
    BaseFamily(const Vector<Component*> & components);
    
    uint32 GetComponentIndex(uint32 componentType, uint32 index) const;
    uint32 GetComponentsCount(uint32 componentType) const;
    uint64 GetComponentsFlags() const;
    
    bool operator==(const BaseFamily<Component> & rhs) const;
    
private:
    uint32 componentIndices[Component::COMPONENT_COUNT];
    uint32 componentCount[Component::COMPONENT_COUNT];
    uint64 componentsFlags;
};
    
////////////////////////////////////////////////////////////////////////////////
// BaseFamilyRepository
////////////////////////////////////////////////////////////////////////////////

template <typename EntityFamilyType>
class BaseFamilyRepository
{
public:
    EntityFamilyType * GetOrCreate(const EntityFamilyType &localFamily);
    
private:
    Vector<EntityFamilyType*> families;
};
    
    
////////////////////////////////////////////////////////////////////////////////
// BaseFamily Impl
////////////////////////////////////////////////////////////////////////////////

template <typename Component>
BaseFamily<Component>::BaseFamily(const Vector<Component*> & components)
    : componentsFlags(0)
{
    Memset(componentIndices, 0, sizeof(componentIndices));
    Memset(componentCount, 0, sizeof(componentCount));
    
    int32 size = static_cast<int32>(components.size ());
    for (int32 i = size - 1; i >= 0; --i)
    {
        uint32 type = components[i]->GetType ();
        componentIndices[type] = i;
        componentCount[type]++;
        componentsFlags |= (uint64)1 << type;
    }
}

template <typename Component>
inline uint32 BaseFamily<Component>::GetComponentIndex(uint32 componentType, uint32 index) const
{
    return componentIndices[componentType] + index;
}

template <typename Component>
inline uint32 BaseFamily<Component>::GetComponentsCount(uint32 componentType) const
{
    return componentCount[componentType];
}

template <typename Component>
inline uint64 BaseFamily<Component>::GetComponentsFlags() const
{
    return componentsFlags;
}

template <typename Component>
inline bool BaseFamily<Component>::operator==(const BaseFamily<Component> & rhs) const
{
    return (componentsFlags == rhs.componentsFlags) && (0 == Memcmp(componentCount, rhs.componentCount, sizeof(componentCount)));
}

////////////////////////////////////////////////////////////////////////////////
// BaseFamilyRepository Impl
////////////////////////////////////////////////////////////////////////////////

template <typename EntityFamilyType>
EntityFamilyType * BaseFamilyRepository<EntityFamilyType>::GetOrCreate(const EntityFamilyType &localFamily)
{
    EntityFamilyType * ret = nullptr;
    
    //check if such family already exists in cache
    size_t familiesSize = families.size ();
    for (size_t i = 0; i < familiesSize; ++i)
    {
        EntityFamilyType * current = families[i];
        if (localFamily == *current)
        {
            ret = current;
            break;
        }
    }
    
    //not exists - add to cache
    if (!ret)
    {
        ret = new EntityFamilyType(localFamily);
        families.push_back(ret);
    }
    
    return ret;
}
    
}

#endif //__DAVAENGINE_BASE_FAMILY_H__
