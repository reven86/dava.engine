#pragma once

#include "Reflection/Reflection.h"

namespace DAVA
{
class FormulaDataFormatter
{
public:
    FormulaDataFormatter();

    void Dump(const Reflection &ref);

private:
    void DumpReflection(const Reflection& ref);

    void Begin();
    void End();
    void DumpKey(const String& str);
    void DumpValue(const String& str);
    void Flush();

    int32 indent = 0;
    DAVA::String msg;
};
}
