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

#include "Debug/DVAssert.h"
#include "FileSystem/Logger.h"

#include "Qt/DeviceInfo/MemoryTool/BranchDiff.h"
#include "Qt/DeviceInfo/MemoryTool/Branch.h"

using namespace DAVA;

BranchDiff::BranchDiff(Branch* leftBranch, Branch* rightBranch)
    : left(leftBranch)
    , right(rightBranch)
{}

BranchDiff::~BranchDiff()
{
    for (auto child : children)
    {
        delete child;
    }
}

void BranchDiff::AppendChild(BranchDiff* child)
{
    DVASSERT(child != nullptr);

    children.push_back(child);
    child->parent = this;
    child->level = level + 1;
}

BranchDiff* BranchDiff::Create(Branch* rootLeft, Branch* rootRight)
{
    DVASSERT(rootLeft != nullptr && rootRight != nullptr);

    BranchDiff* root = new BranchDiff(nullptr, nullptr);
    auto sortByPtr = [](const Branch* l, const Branch* r) -> bool { return l->name < r->name; };
    rootLeft->SortChildren(sortByPtr);
    rootRight->SortChildren(sortByPtr);

    FollowBoth(root, rootLeft, rootRight);

    return root;
}

void BranchDiff::FollowBoth(BranchDiff* parent, Branch* rootLeft, Branch* rootRight)
{
    auto lbegin = rootLeft->children.begin();
    auto lend = rootLeft->children.end();
    auto rbegin = rootRight->children.begin();
    auto rend = rootRight->children.end();

    while (lbegin != lend && rbegin != rend)
    {
        if ((*lbegin)->name < (*rbegin)->name)
        {
            BranchDiff* diff = new BranchDiff(*lbegin, nullptr);
            parent->AppendChild(diff);
            FollowLeft(diff, *lbegin);
            ++lbegin;
        }
        else if ((*rbegin)->name < (*lbegin)->name)
        {
            BranchDiff* diff = new BranchDiff(nullptr, *rbegin);
            parent->AppendChild(diff);
            FollowRight(diff, *rbegin);
            ++rbegin;
        }
        else
        {
            BranchDiff* diff = new BranchDiff(*lbegin, *rbegin);
            parent->AppendChild(diff);
            FollowBoth(diff, *lbegin, *rbegin);

            ++lbegin;
            ++rbegin;
        }
    }
    while (lbegin != lend)
    {
        BranchDiff* diff = new BranchDiff(*lbegin, nullptr);
        parent->AppendChild(diff);
        FollowLeft(diff, *lbegin);
        ++lbegin;
    }
    while (rbegin != rend)
    {
        BranchDiff* diff = new BranchDiff(nullptr, *rbegin);
        parent->AppendChild(diff);
        FollowRight(diff, *rbegin);
        ++rbegin;
    }
}

void BranchDiff::FollowLeft(BranchDiff* parent, Branch* left)
{
    for (auto child : left->children)
    {
        BranchDiff* diff = new BranchDiff(child, nullptr);
        parent->AppendChild(diff);
        FollowLeft(diff, child);
    }
}

void BranchDiff::FollowRight(BranchDiff* parent, Branch* right)
{
    for (auto child : right->children)
    {
        BranchDiff* diff = new BranchDiff(nullptr, child);
        parent->AppendChild(diff);
        FollowRight(diff, child);
    }
}
