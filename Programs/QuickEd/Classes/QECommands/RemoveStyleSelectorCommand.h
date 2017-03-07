
#pragma once

#include "QECommands/Private/QEPackageCommand.h"

class StyleSheetNode;
class StyleSheetSelectorProperty;

class RemoveStyleSelectorCommand : public QEPackageCommand
{
public:
    RemoveStyleSelectorCommand(PackageNode* package, StyleSheetNode* node, StyleSheetSelectorProperty* property);
    ~RemoveStyleSelectorCommand() override;

    void Redo() override;
    void Undo() override;

private:
    StyleSheetNode* node = nullptr;
    StyleSheetSelectorProperty* property = nullptr;
    int index = -1;
};
