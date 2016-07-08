#ifndef __DAVAFRAMEWORK_SETPROPERTYVALUECOMMAND_H__
#define __DAVAFRAMEWORK_SETPROPERTYVALUECOMMAND_H__

#include "ICommand.h"
#include "ObjectHandle.h"
#include "FileSystem/VariantType.h"

namespace DAVA
{
class InspMember;

class SetPropertyValueCommand : public ICommand
{
public:
    SetPropertyValueCommand(const ObjectHandle& object, const InspMember* property, VariantType newValue);

    void Execute() override;
    void Redo() override;
    void Undo() override;

private:
    ObjectHandle object;
    const InspMember* property;
    VariantType newValue;
    VariantType oldValue;
};
}

#endif // __DAVAFRAMEWORK_SETPROPERTYVALUECOMMAND_H__