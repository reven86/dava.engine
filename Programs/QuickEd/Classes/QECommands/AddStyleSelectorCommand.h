
#pragma once

#include "QECommands/Private/QEPackageCommand.h"

class StyleSheetNode;
class StyleSheetSelectorProperty;

class AddStyleSelectorCommand : public QEPackageCommand
{
public:
    AddStyleSelectorCommand(PackageNode* package, StyleSheetNode* node, StyleSheetSelectorProperty* property);
    ~AddStyleSelectorCommand() override;

    void Redo() override;
    void Undo() override;

private:
    StyleSheetNode* node = nullptr;
    StyleSheetSelectorProperty* property = nullptr;
    int index = -1;
};
