#ifndef __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__
#define __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__

#include "FileSystem/VariantType.h"
#include "EditorSystems/EditorSystemsManager.h"

#include "QtTools/Commands/CommandWithoutExecute.h"

class PackageNode;
class ControlNode;
class AbstractProperty;

class ChangePropertyValueCommand : public CommandWithoutExecute
{
public:
    ChangePropertyValueCommand(PackageNode* _root, ControlNode* _node, AbstractProperty* _property, const DAVA::VariantType& newValue);
    ~ChangePropertyValueCommand() override = default;

    void Redo() override;
    void Undo() override;

private:
    DAVA::VariantType GetValueFromProperty(AbstractProperty* property);
    PackageNode* root = nullptr;
    ControlNode* node = nullptr;
    AbstractProperty* property = nullptr;
    DAVA::VariantType oldValue;
    DAVA::VariantType newValue;
};

#endif // __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__
