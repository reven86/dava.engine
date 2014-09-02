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



#ifndef __DAVAENGINE_ENTITY_COMPONENT_TYPE_H__
#define __DAVAENGINE_ENTITY_COMPONENT_TYPE_H__

#include "Entity/Pool.h"

namespace DAVA 
{

class ComponentType
{
public:
    static uint64 globalBit;
    static uint64 globalIndex;
    uint64 bit;
    uint64 index;
    
    ComponentType()
    {
        Init();
    }

    void Init();
    
    uint64 GetBit() const { return bit; } ; 
    uint64 GetIndex() const { return index; };
    
    inline friend bool operator < (const ComponentType & f1, const ComponentType & f2);
};
    
inline bool operator < (const ComponentType & f1, const ComponentType & f2) { return f1.index < f2.index; };

/*
    Entity Family Should be unique ID and pointer or reference to previous Entity Family. 
    TODO: Refactor everything to make it possible to use unlimited number of EntityFamilies, and ComponentTypes.
 */
class EntityFamilyType
{
public:
    uint64 bit;
    
    EntityFamilyType(uint64 _bit = 0) { bit = _bit; };
    static EntityFamilyType AddComponentType(EntityFamilyType oldFamily, ComponentType type) { return EntityFamilyType(oldFamily.GetBit() | type.GetBit()); };
	static EntityFamilyType RemoveComponentType(EntityFamilyType oldFamily, ComponentType type) { return EntityFamilyType(oldFamily.GetBit() & (~type.GetBit())); };
    
    uint64 GetBit() const { return bit; };
    bool IsEmpty() const { return bit == 0; };
    
    
    inline friend bool operator < (const EntityFamilyType & f1, const EntityFamilyType & f2);
};

inline bool operator < (const EntityFamilyType & f1, const EntityFamilyType & f2) { return f1.bit < f2.bit; };
    
};


#endif // __DAVAENGINE_ENTITY_COMPONENT_H__
