#include "AddRemoveStylePropertyCommand.h"

#include "Document/CommandsBase/QECommandIDs.h"
#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/ControlProperties/StyleSheetProperty.h"
#include "Model/ControlProperties/StyleSheetRootProperty.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

AddRemoveStylePropertyCommand::AddRemoveStylePropertyCommand(PackageNode* aRoot, StyleSheetNode* aNode, StyleSheetProperty* aProperty, bool anAdd)
    : CommandWithoutExecute(CMDID_ADD_REMOVE_STYLE_PROPERTY, "AddRemoveStyleProperty")
    , root(SafeRetain(aRoot))
    , node(SafeRetain(aNode))
    , property(SafeRetain(aProperty))
    , add(anAdd)
{
}

AddRemoveStylePropertyCommand::~AddRemoveStylePropertyCommand()
{
    SafeRelease(root);
    SafeRelease(node);
    SafeRelease(property);
}

void AddRemoveStylePropertyCommand::Redo()
{
    if (add)
        root->AddStyleProperty(node, property);
    else
        root->RemoveStyleProperty(node, property);
}

void AddRemoveStylePropertyCommand::Undo()
{
    if (add)
        root->RemoveStyleProperty(node, property);
    else
        root->AddStyleProperty(node, property);
}
