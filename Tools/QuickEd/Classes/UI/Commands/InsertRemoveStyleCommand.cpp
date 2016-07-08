#include "InsertRemoveStyleCommand.h"

#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/PackageHierarchy/StyleSheetsNode.h"
#include "Model/PackageHierarchy/PackageNode.h"

using namespace DAVA;

InsertRemoveStyleCommand::InsertRemoveStyleCommand(PackageNode* _root, StyleSheetNode* _node, StyleSheetsNode* _dest, int _index, bool _insert, QUndoCommand* parent)
    : QUndoCommand(parent)
    , root(SafeRetain(_root))
    , node(SafeRetain(_node))
    , dest(SafeRetain(_dest))
    , index(_index)
    , insert(_insert)
{
}

InsertRemoveStyleCommand::~InsertRemoveStyleCommand()
{
    SafeRelease(root);
    SafeRelease(node);
    SafeRelease(dest);
}

void InsertRemoveStyleCommand::redo()
{
    if (insert)
        root->InsertStyle(node, dest, index);
    else
        root->RemoveStyle(node, dest);
}

void InsertRemoveStyleCommand::undo()
{
    if (insert)
        root->RemoveStyle(node, dest);
    else
        root->InsertStyle(node, dest, index);
}
