#pragma once

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
struct ScriptState;

class LuaScript
{
public:
    LuaScript();
    ~LuaScript();

    bool LoadString(const String& script);
    bool LoadFile(const FilePath& filepath);
    bool Run(const Reflection& context);

    void RegisterGlobalReflection(const String& name, const Reflection& reflection);

private:
    ScriptState* state = nullptr;
};
}