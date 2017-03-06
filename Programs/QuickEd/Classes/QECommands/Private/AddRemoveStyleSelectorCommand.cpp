#include "QECommands/AddRemoveStyleSelectorCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/ControlProperties/StyleSheetSelectorProperty.h"
#include "Model/ControlProperties/StyleSheetRootProperty.h"
#include "Model/ControlProperties/SectionProperty.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

AddRemoveStyleSelectorCommand::AddRemoveStyleSelectorCommand(PackageNode* package, StyleSheetNode* node_, StyleSheetSelectorProperty* property_, bool add_)
    : QEPackageCommand(package, ADD_REMOVE_STYLE_SELECTOR_COMMAND, "AddRemoveStyleSelector")
    , node(SafeRetain(node_))
    , property(SafeRetain(property_))
    , add(add_)
    , index(-1)
{
    if (add)
    {
        index = node->GetRootProperty()->GetSelectors()->GetCount();
    }
    else
    {
        index = node->GetRootProperty()->GetSelectors()->GetIndex(property);
    }

    DVASSERT(index != -1);
}

AddRemoveStyleSelectorCommand::~AddRemoveStyleSelectorCommand()
{
    SafeRelease(node);
    SafeRelease(property);
}

void AddRemoveStyleSelectorCommand::Redo()
{
    if (index != -1)
    {
        if (add)
            package->InsertSelector(node, property, index);
        else
            package->RemoveSelector(node, property);
    }
}

void AddRemoveStyleSelectorCommand::Undo()
{
    if (index != -1)
    {
        if (add)
            package->RemoveSelector(node, property);
        else
            package->InsertSelector(node, property, index);
    }
}
