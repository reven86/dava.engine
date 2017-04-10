#pragma once

#include "QECommands/Private/QEPackageCommand.h"
#include <Base/Any.h>

class ControlNode;
class AbstractProperty;

class ChangePropertyValueCommand : public QEPackageCommand
{
public:
    ChangePropertyValueCommand(PackageNode* package);
    ChangePropertyValueCommand(PackageNode* package, ControlNode* node, AbstractProperty* property, const DAVA::Any& newValue);

    void AddNodePropertyValue(ControlNode* node, AbstractProperty* property, const DAVA::Any& newValue);

    void Redo() override;
    void Undo() override;

    void ApplyProperty(ControlNode* node, AbstractProperty* property, const DAVA::Any& value);

    bool MergeWith(const DAVA::Command* command) override;

private:
    struct Item
    {
        Item(ControlNode* node, AbstractProperty* property, const DAVA::Any& newValue);
        DAVA::RefPtr<ControlNode> node;
        DAVA::RefPtr<AbstractProperty> property;
        DAVA::Any newValue;
        DAVA::Any oldValue;
    };
    DAVA::Vector<Item> items;
};
