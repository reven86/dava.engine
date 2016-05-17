#ifndef __INSP_DYNAMIC_MODIFY_COMMAND_H__
#define __INSP_DYNAMIC_MODIFY_COMMAND_H__

#include "Commands2/Base/Command2.h"

class InspDynamicModifyCommand : public Command2
{
public:
    InspDynamicModifyCommand(DAVA::InspInfoDynamic* dynamicInfo, const DAVA::InspInfoDynamic::DynamicData& ddata, DAVA::FastName key, const DAVA::VariantType& value);
    ~InspDynamicModifyCommand();

    virtual void Undo();
    virtual void Redo();
    virtual DAVA::Entity* GetEntity() const
    {
        return NULL;
    };

    DAVA::InspInfoDynamic* dynamicInfo;
    DAVA::FastName key;

    DAVA::InspInfoDynamic::DynamicData ddata;

    DAVA::VariantType oldValue;
    DAVA::VariantType newValue;
};

#endif // __INSP_DYNAMIC_MODIFY_COMMAND_H__
