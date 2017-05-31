#include "QECommands/SetGuidesCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"

using namespace DAVA;

SetGuidesCommand::SetGuidesCommand(PackageNode* package, const DAVA::String& controlName_, DAVA::Vector2::eAxis orientation_, const DAVA::List<DAVA::float32>& guides_)
    : QEPackageCommand(package, "Set vertical guides")
    , controlName(controlName_)
    , orientation(orientation_)
    , guides(guides_)
    , oldGuides(package->GetGuides(controlName, orientation))
{
}

void SetGuidesCommand::Redo()
{
    package->SetGuides(controlName, orientation, guides);
}

void SetGuidesCommand::Undo()
{
    package->SetGuides(controlName, orientation, oldGuides);
}
