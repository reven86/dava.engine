#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
struct ScriptState;

class Script
{
public:
    Script();
    ~Script();

    bool LoadString(const String& script);
    bool LoadFile(const FilePath& filepath);
    bool Run(const Reflection& context);

    void RegisterGlobalReflection(const String& name, const Reflection& reflection);

private:
    ScriptState* state = nullptr;
};
}