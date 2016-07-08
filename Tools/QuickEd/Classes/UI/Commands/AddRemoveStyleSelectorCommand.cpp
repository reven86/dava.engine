#include "AddRemoveStyleSelectorCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/ControlProperties/StyleSheetSelectorProperty.h"
#include "Model/ControlProperties/StyleSheetRootProperty.h"
#include "Model/ControlProperties/SectionProperty.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

AddRemoveStyleSelectorCommand::AddRemoveStyleSelectorCommand(PackageNode* aRoot, StyleSheetNode* aNode, StyleSheetSelectorProperty* aProperty, bool anAdd, QUndoCommand* parent)
    : QUndoCommand(parent)
    , root(SafeRetain(aRoot))
    , node(SafeRetain(aNode))
    , property(SafeRetain(aProperty))
    , add(anAdd)
    , index(-1)
{
    if (add)
    {
        index = aNode->GetRootProperty()->GetSelectors()->GetCount();
    }
    else
    {
        index = aNode->GetRootProperty()->GetSelectors()->GetIndex(property);
    }

    DVASSERT(index != -1);
}

AddRemoveStyleSelectorCommand::~AddRemoveStyleSelectorCommand()
{
    SafeRelease(root);
    SafeRelease(node);
    SafeRelease(property);
}

void AddRemoveStyleSelectorCommand::redo()
{
    if (index != -1)
    {
        if (add)
            root->InsertSelector(node, property, index);
        else
            root->RemoveSelector(node, property);
    }
}

void AddRemoveStyleSelectorCommand::undo()
{
    if (index != -1)
    {
        if (add)
            root->RemoveSelector(node, property);
        else
            root->InsertSelector(node, property, index);
    }
}
