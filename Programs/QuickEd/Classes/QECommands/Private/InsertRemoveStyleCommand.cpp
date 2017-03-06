#include "QECommands/InsertRemoveStyleCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/PackageHierarchy/StyleSheetsNode.h"
#include "Model/PackageHierarchy/PackageNode.h"

using namespace DAVA;

InsertRemoveStyleCommand::InsertRemoveStyleCommand(PackageNode* package, StyleSheetNode* node_, StyleSheetsNode* dest_, int index_, bool insert_)
    : QEPackageCommand(package, INSERT_REMOVE_STYLE_COMMAND, "InsertRemoveStyle")
    , node(SafeRetain(node_))
    , dest(SafeRetain(dest_))
    , index(index_)
    , insert(insert_)
{
}

InsertRemoveStyleCommand::~InsertRemoveStyleCommand()
{
    SafeRelease(node);
    SafeRelease(dest);
}

void InsertRemoveStyleCommand::Redo()
{
    if (insert)
        package->InsertStyle(node, dest, index);
    else
        package->RemoveStyle(node, dest);
}

void InsertRemoveStyleCommand::Undo()
{
    if (insert)
        package->RemoveStyle(node, dest);
    else
        package->InsertStyle(node, dest, index);
}
