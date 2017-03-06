#pragma once

#include "QECommands/Private/QEPackageCommand.h"

class StyleSheetNode;
class StyleSheetsNode;

class InsertRemoveStyleCommand : public QEPackageCommand
{
public:
    InsertRemoveStyleCommand(PackageNode* package, StyleSheetNode* node, StyleSheetsNode* dest, int index, bool insert);
    ~InsertRemoveStyleCommand() override;

    void Redo() override;
    void Undo() override;

private:
    StyleSheetNode* node = nullptr;
    StyleSheetsNode* dest = nullptr;
    const int index;
    const bool insert;
};
