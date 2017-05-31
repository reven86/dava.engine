#pragma once

#include "QECommands/Private/QEPackageCommand.h"

#include <Math/Vector.h>

class PackageBaseNode;

class SetGuidesCommand : public QEPackageCommand
{
public:
    SetGuidesCommand(PackageNode* package, const DAVA::String& controlName, DAVA::Vector2::eAxis orientation, const DAVA::List<DAVA::float32>& guides);

    void Redo() override;
    void Undo() override;

private:
    DAVA::String controlName;
    const DAVA::Vector2::eAxis orientation;
    DAVA::List<DAVA::float32> guides;
    DAVA::List<DAVA::float32> oldGuides;
};
