#include "QECommands/Private/QEPackageCommand.h"
#include "Model/PackageHierarchy/PackageNode.h"

QEPackageCommand::QEPackageCommand(PackageNode* package_, DAVA::int32 commandID, const DAVA::String& description)
    : DAVA::Command(commandID, description)
    , package(SafeRetain(package_))
{
}

QEPackageCommand::~QEPackageCommand()
{
    SafeRelease(package);
}
