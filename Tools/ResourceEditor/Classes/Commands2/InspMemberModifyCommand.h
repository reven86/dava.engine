#ifndef __INSP_MEMEBER_MODIFY_COMMAND_H__
#define __INSP_MEMEBER_MODIFY_COMMAND_H__

#include "Commands2/Base/Command2.h"

class InspMemberModifyCommand : public Command2
{
public:
    InspMemberModifyCommand(const DAVA::InspMember* member, void* object, const DAVA::VariantType& value);
    ~InspMemberModifyCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const
    {
        return NULL;
    };

    const DAVA::InspMember* member;
    void* object;

    DAVA::VariantType oldValue;
    DAVA::VariantType newValue;
};

#endif // __INSP_MEMEBER_MODIFY_COMMAND_H__
