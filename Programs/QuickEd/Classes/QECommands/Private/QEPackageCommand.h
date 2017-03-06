#pragma once

#include "Command/Command.h"

class PackageNode;

class QEPackageCommand : public DAVA::Command
{
public:
    QEPackageCommand(PackageNode* package, DAVA::int32 commandID, const DAVA::String& description = "");
    ~QEPackageCommand() override;

protected:
    PackageNode* package = nullptr;
};
