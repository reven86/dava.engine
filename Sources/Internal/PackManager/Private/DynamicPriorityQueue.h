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

#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
template <typename Type, typename PriorityComparator>
class DynamicPriorityQueue
{
public:
    DynamicPriorityQueue() = default;

    bool Empty() const;
    uint32 Size() const;
    Type* Top() const;
    void Push(Type*);
    void UpdatePriority(Type*);
    Type* Pop();

private:
    Vector<Type*> items;
};

template <typename Type, typename PriorityComparator>
bool DynamicPriorityQueue<Type, PriorityComparator>::Empty() const
{
    return items.empty();
}

template <typename Type, typename PriorityComparator>
uint32 DynamicPriorityQueue<Type, PriorityComparator>::Size() const
{
    return static_cast<uint32>(items.size());
}

template <typename Type, typename PriorityComparator>
Type* DynamicPriorityQueue<Type, PriorityComparator>::Top() const
{
    Type* topItem = items.front();
    return topItem;
}

template <typename Type, typename PriorityComparator>
void DynamicPriorityQueue<Type, PriorityComparator>::Push(Type* elem)
{
    auto it = std::find(begin(items), end(items), elem);
    if (it == end(items))
    {
        items.push_back(elem);
        std::push_heap(begin(items), end(items), PriorityComparator());
    }
    else
    {
        std::sort_heap(begin(items), end(items), PriorityComparator());
    }
}

template <typename Type, typename PriorityComparator>
void DynamicPriorityQueue<Type, PriorityComparator>::UpdatePriority(Type* elem)
{
    auto it = std::find(begin(items), end(items), elem);
    if (it != end(items))
    {
        std::sort_heap(begin(items), end(items), PriorityComparator());
    }
}

template <typename Type, typename PriorityComparator>
Type* DynamicPriorityQueue<Type, PriorityComparator>::Pop()
{
    std::pop_heap(begin(items), end(items), PriorityComparator());
    Type* topItem = items.back();
    items.pop_back();
    return topItem;
}
} // end namespace DAVA
