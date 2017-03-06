
#pragma once

#include "QECommands/Private/QEPackageCommand.h"

class StyleSheetNode;
class StyleSheetSelectorProperty;

class AddRemoveStyleSelectorCommand : public QEPackageCommand
{
public:
    AddRemoveStyleSelectorCommand(PackageNode* package, StyleSheetNode* node, StyleSheetSelectorProperty* property, bool add);
    ~AddRemoveStyleSelectorCommand() override;

    void Redo() override;
    void Undo() override;

private:
    StyleSheetNode* node = nullptr;
    StyleSheetSelectorProperty* property = nullptr;
    const bool add;
    int index = -1;
};
