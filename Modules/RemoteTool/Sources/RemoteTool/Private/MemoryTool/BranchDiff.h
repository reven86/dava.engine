#pragma once

#include <Base/BaseTypes.h>

struct Branch;

struct BranchDiff final
{
    BranchDiff(Branch* leftBranch, Branch* rightBranch);
    ~BranchDiff();

    void AppendChild(BranchDiff* child);

    int level = 0;
    Branch* left;
    Branch* right;
    BranchDiff* parent = nullptr;
    DAVA::Vector<BranchDiff*> children;

    // Create diff from two branches
    static BranchDiff* Create(Branch* rootLeft, Branch* rootRight);

private:
    static void FollowBoth(BranchDiff* parent, Branch* rootLeft, Branch* rootRight);
    static void FollowLeft(BranchDiff* parent, Branch* left);
    static void FollowRight(BranchDiff* parent, Branch* right);
};
