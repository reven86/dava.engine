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

#ifndef __BRANCH_H__
#define __BRANCH_H__

#include "Base/BaseTypes.h"
#include "MemoryManager/MemoryManagerTypes.h"

struct Branch final
{
    Branch(const DAVA::String* name = nullptr);
    ~Branch();

    Branch* FindInChildren(const DAVA::String* name) const;
    int ChildIndex(Branch* child) const;

    void AppendChild(Branch* child);
    void UpdateStat(DAVA::uint32 allocSize, DAVA::uint32 blockCount, DAVA::uint32 pools, DAVA::uint32 tags);

    // Get memory blocks from all children
    DAVA::Vector<DAVA::MMBlock*> GetMemoryBlocks() const;

    template<typename F>
    void SortChildren(F fn);

    int level = 0;                          // Level in tree hierarchy - for convenience in Qt models
    const DAVA::String* name = nullptr;     // Function name
    Branch* parent = nullptr;
    DAVA::Vector<Branch*> children;

    DAVA::uint32 allocByApp = 0;            // Total allocated size in branch including children
    DAVA::uint32 nblocks = 0;               // Total block count in branch including children
    DAVA::Vector<DAVA::MMBlock*> mblocks;   // Memory blocks belonging to leaf branch

private:
    static void CollectBlocks(const Branch* branch, DAVA::Vector<DAVA::MMBlock*>& target);
};

//////////////////////////////////////////////////////////////////////////
inline Branch::Branch(const DAVA::String* name_)
    : name(name_)
{}

template<typename F>
inline void Branch::SortChildren(F fn)
{
    std::sort(children.begin(), children.end(), fn);
    for (Branch* child : children)
    {
        child->SortChildren(fn);
    }
}

#endif  // __BRANCH_H__
