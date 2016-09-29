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
    struct BaseResult
    {
        int32 code = 0;
        String error;
    };

    struct LoadResult : public BaseResult
    {
        bool loaded = false;
    };

    struct RunResult : public BaseResult
    {
        Any value;
    };

    LuaScript();
    ~LuaScript();

    LoadResult RunString(const String& script);
    LoadResult RunFile(const FilePath& filepath);
    RunResult RunMain(Any args[]);

private:
    ScriptState* state = nullptr;
};
}