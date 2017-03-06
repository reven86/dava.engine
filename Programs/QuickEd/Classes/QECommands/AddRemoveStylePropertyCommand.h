#pragma once

#include "QECommands/Private/QEPackageCommand.h"

class StyleSheetNode;
class StyleSheetProperty;

class AddRemoveStylePropertyCommand : public QEPackageCommand
{
public:
    AddRemoveStylePropertyCommand(PackageNode* package, StyleSheetNode* node, StyleSheetProperty* property, bool add);
    ~AddRemoveStylePropertyCommand() override;

    void Redo() override;
    void Undo() override;

private:
    StyleSheetNode* node = nullptr;
    StyleSheetProperty* property = nullptr;
    const bool add;
};
