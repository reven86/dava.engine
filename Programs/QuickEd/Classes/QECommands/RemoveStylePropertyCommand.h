#pragma once

#include "QECommands/Private/QEPackageCommand.h"

class StyleSheetNode;
class StyleSheetProperty;

class RemoveStylePropertyCommand : public QEPackageCommand
{
public:
    RemoveStylePropertyCommand(PackageNode* package, StyleSheetNode* node, StyleSheetProperty* property);
    ~RemoveStylePropertyCommand() override;

    void Redo() override;
    void Undo() override;

private:
    StyleSheetNode* node = nullptr;
    StyleSheetProperty* property = nullptr;
};
