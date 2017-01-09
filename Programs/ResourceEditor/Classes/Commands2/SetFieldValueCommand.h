#pragma once

#include "Classes/Commands2/Base/RECommand.h"
#include "Reflection/Reflection.h"

#include "Base/Any.h"

class SetFieldValueCommand : public RECommand
{
public:
    SetFieldValueCommand(const DAVA::Reflection::Field& field, const DAVA::Any& newValue);

    void Redo() override;
    void Undo() override;

private:
    DAVA::Any oldValue;
    DAVA::Any newValue;
    DAVA::Reflection reflection;
};