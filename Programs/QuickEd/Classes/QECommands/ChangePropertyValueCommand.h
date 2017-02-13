#ifndef __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__
#define __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__

#include "Base/Any.h"
#include "EditorSystems/EditorSystemsManager.h"

#include "Command/Command.h"

class PackageNode;
class ControlNode;
class AbstractProperty;

class ChangePropertyValueCommand : public DAVA::Command
{
public:
    ChangePropertyValueCommand(PackageNode* _root, ControlNode* _node, AbstractProperty* _property, const DAVA::Any& _newValue);
    ~ChangePropertyValueCommand() override = default;

    void Redo() override;
    void Undo() override;

private:
    DAVA::Any GetValueFromProperty(AbstractProperty* property);
    PackageNode* root = nullptr;
    ControlNode* node = nullptr;
    AbstractProperty* property = nullptr;
    DAVA::Any oldValue;
    DAVA::Any newValue;
};

#endif // __QUICKED_CHANGE_PROPERTY_VALUE_COMMAND_H__
