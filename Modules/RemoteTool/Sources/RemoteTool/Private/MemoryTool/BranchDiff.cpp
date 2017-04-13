#include "RemoteTool/Private/MemoryTool/BranchDiff.h"
#include "RemoteTool/Private/MemoryTool/Branch.h"

#include <Debug/DVAssert.h>
#include <Logger/Logger.h>

using namespace DAVA;

BranchDiff::BranchDiff(Branch* leftBranch, Branch* rightBranch)
    : left(leftBranch)
    , right(rightBranch)
{
}

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
