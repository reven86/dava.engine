#pragma once

#include "QECommands/Private/QEPackageCommand.h"
#include <FileSystem/VariantType.h>

class StyleSheetNode;
class AbstractProperty;

class ChangeStylePropertyCommand : public QEPackageCommand
{
public:
    ChangeStylePropertyCommand(PackageNode* package, StyleSheetNode* node, AbstractProperty* property, const DAVA::VariantType& newValue);

    void Redo() override;
    void Undo() override;

private:
    DAVA::RefPtr<StyleSheetNode> node;
    DAVA::RefPtr<AbstractProperty> property;
    DAVA::VariantType oldValue;
    DAVA::VariantType newValue;
};
