#pragma once

#include "QECommands/Private/QEPackageCommand.h"

class StyleSheetNode;
class StyleSheetProperty;

class AddStylePropertyCommand : public QEPackageCommand
{
public:
    AddStylePropertyCommand(PackageNode* package, StyleSheetNode* node, StyleSheetProperty* property);
    ~AddStylePropertyCommand() override;

    void Redo() override;
    void Undo() override;

private:
    StyleSheetNode* node = nullptr;
    StyleSheetProperty* property = nullptr;
};
