#pragma once

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
    bool Run(const DAVA::Reflection& context);

private:
    ScriptState* state = nullptr;
};
}