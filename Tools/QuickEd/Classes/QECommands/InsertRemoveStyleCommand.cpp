#include "InsertRemoveStyleCommand.h"

#include "Document/CommandsBase/QECommandIDs.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/PackageHierarchy/StyleSheetsNode.h"
#include "Model/PackageHierarchy/PackageNode.h"

using namespace DAVA;

InsertRemoveStyleCommand::InsertRemoveStyleCommand(PackageNode* _root, StyleSheetNode* _node, StyleSheetsNode* _dest, int _index, bool _insert)
    : CommandWithoutExecute(CMDID_INSERT_REMOVE_STYLE, "InsertRemoveStyle")
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

void InsertRemoveStyleCommand::Redo()
{
    if (insert)
        root->InsertStyle(node, dest, index);
    else
        root->RemoveStyle(node, dest);
}

void InsertRemoveStyleCommand::Undo()
{
    if (insert)
        root->RemoveStyle(node, dest);
    else
        root->InsertStyle(node, dest, index);
}
