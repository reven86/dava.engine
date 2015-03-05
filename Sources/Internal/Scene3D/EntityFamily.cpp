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

#include "Entity.h"
#include "Scene3D/EntityFamily.h"

namespace DAVA
{

Vector<EntityFamily*> EntityFamily::families;

EntityFamily::EntityFamily()
: componentsFlags(0)
{
    Memset(componentIndices, 0, sizeof(componentIndices));
    Memset(componentCount, 0, sizeof(componentCount));
}

EntityFamily * EntityFamily::GetOrCreate (const Vector<Component*> & components)
{
    EntityFamily * ret = 0;

    //setup new family
    EntityFamily localFamily;
    int32 size = static_cast<int32>(components.size ());
    for (int32 i = size - 1; i >= 0; --i)
    {
        uint32 type = components[i]->GetType ();
        localFamily.componentIndices[type] = i;
        localFamily.componentCount[type]++;
        localFamily.componentsFlags |= (uint64)1 << type;
    }

    //check if such family already exists in cache
    size_t familiesSize = families.size ();
    for (size_t i = 0; i < familiesSize; ++i)
    {
        EntityFamily * current = families[i];
        if (localFamily == *current)
        {
            ret = current;
            break;
        }
    }

    //not exists - add to cache
    if (0 == ret)
    {
        ret = new EntityFamily (localFamily);
        families.push_back (ret);
    }

    return ret;
}

}
