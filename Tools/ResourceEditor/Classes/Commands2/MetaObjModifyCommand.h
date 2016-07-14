#ifndef __META_OBJ_MODIFY_COMMAND_H__
#define __META_OBJ_MODIFY_COMMAND_H__

#include "QtTools/Commands/CommandWithoutExecute.h"
#include "FileSystem/VariantType.h"

namespace DAVA
{
class MetaInfo;
}

class MetaObjModifyCommand : public CommandWithoutExecute
{
public:
    MetaObjModifyCommand(const DAVA::MetaInfo* info, void* object, const DAVA::VariantType& value);
    ~MetaObjModifyCommand();

    void Undo() override;
    void Redo() override;

    const DAVA::MetaInfo* info;
    void* object;

    DAVA::VariantType oldValue;
    DAVA::VariantType newValue;
};

#endif // __META_OBJ_MODIFY_COMMAND_H__
