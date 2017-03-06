#pragma once

#include "QECommands/Private/QEPackageCommand.h"
#include <FileSystem/VariantType.h>

class StyleSheetNode;
class AbstractProperty;

class ChangeStylePropertyCommand : public QEPackageCommand
{
public:
    ChangeStylePropertyCommand(PackageNode* package, StyleSheetNode* node, AbstractProperty* property, const DAVA::VariantType& newValue);
    ~ChangeStylePropertyCommand() override;

    void Redo() override;
    void Undo() override;

private:
    StyleSheetNode* node = nullptr;
    AbstractProperty* property = nullptr;
    DAVA::VariantType oldValue;
    DAVA::VariantType newValue;
};
