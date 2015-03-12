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
#ifndef __DAVAENGINE_ENTITY_FAMILY_H__
#define __DAVAENGINE_ENTITY_FAMILY_H__

#include "Base/BaseTypes.h"
#include "Entity/Component.h"

namespace DAVA
{

class EntityFamily
{
public:
    EntityFamily ();

    static EntityFamily * GetOrCreate (const Vector<Component*> & components);
    uint32 GetComponentIndex (uint32 componentType, uint32 index) const;
    uint32 GetComponentsCount (uint32 componentType) const;
    uint64 GetComponentsFlags () const;

private:
    uint32 componentIndices[Component::COMPONENT_COUNT];
    uint32 componentCount[Component::COMPONENT_COUNT];
    uint64 componentsFlags;
    static Vector<EntityFamily*> families;
    friend bool operator==(const EntityFamily & lhs, const EntityFamily & rhs);
};

inline uint32 EntityFamily::GetComponentIndex(uint32 componentType, uint32 index) const
{
    return componentIndices[componentType] + index;
}

inline uint32 EntityFamily::GetComponentsCount (uint32 componentType) const
{
    return componentCount[componentType];
}

inline uint64 EntityFamily::GetComponentsFlags () const
{
    return componentsFlags;
}

inline bool operator==(const EntityFamily & lhs, const EntityFamily & rhs)
{
    return (lhs.componentsFlags == rhs.componentsFlags) && (0 == Memcmp (lhs.componentCount, rhs.componentCount, sizeof (lhs.componentCount)));
}

}

#endif //__DAVAENGINE_ENTITY_FAMILY_H_