#include "QECommands/AddStyleSelectorCommand.h"
#include "QECommands/QECommandIDs.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/StyleSheetNode.h"
#include "Model/ControlProperties/StyleSheetSelectorProperty.h"
#include "Model/ControlProperties/StyleSheetRootProperty.h"
#include "Model/ControlProperties/SectionProperty.h"
#include "UI/Components/UIComponent.h"

using namespace DAVA;

AddStyleSelectorCommand::AddStyleSelectorCommand(PackageNode* package, StyleSheetNode* node_, StyleSheetSelectorProperty* property_)
    : QEPackageCommand(package, ADD_REMOVE_STYLE_SELECTOR_COMMAND, "AddRemoveStyleSelector")
    , node(SafeRetain(node_))
    , property(SafeRetain(property_))
    , index(-1)
{
    index = node->GetRootProperty()->GetSelectors()->GetCount();
    DVASSERT(index != -1);
}

AddStyleSelectorCommand::~AddStyleSelectorCommand()
{
    SafeRelease(node);
    SafeRelease(property);
}

void AddStyleSelectorCommand::Redo()
{
    if (index != -1)
    {
        package->InsertSelector(node, property, index);
    }
}

void AddStyleSelectorCommand::Undo()
{
    if (index != -1)
    {
        package->RemoveSelector(node, property);
    }
}
