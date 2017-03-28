#include "FormulaDataFormatter.h"

#include "UI/Formula/FormulaFormatter.h"
#include "UI/Formula/AnyConverter.h"

#include "Logger/Logger.h"
#include "Utils/StringFormat.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
FormulaDataFormatter::FormulaDataFormatter()
{
}

void FormulaDataFormatter::Dump(const Reflection &ref)
{
    DumpReflection(ref);
}

void FormulaDataFormatter::DumpReflection(const Reflection& ref)
{
    Begin();

    Vector<Reflection::Field> fields = ref.GetFields();

    for (auto& it : fields)
    {
        const Any& key = it.key;

        String name = "";

        if (key.CanGet<size_t>())
        {
            name = Format("[%d]", key.Get<size_t>());
        }
        else if (key.CanGet<String>())
        {
            name = key.Get<String>();
        }
        else
        {
            DVASSERT(false);
            name = "??";
        }

        DumpReflection(it.ref);
    }

    End();
}

void FormulaDataFormatter::Begin()
{
    indent += 2;
}

void FormulaDataFormatter::End()
{
    DVASSERT(indent >= 2);
    indent -= 2;
}

void FormulaDataFormatter::DumpKey(const String& str)
{
    Flush();
    for (int32 i = 0; i < indent; i++)
    {
        msg += " ";
    }
    msg += str;
}

void FormulaDataFormatter::DumpValue(const String& str)
{
    msg += " " + str;
}

void FormulaDataFormatter::Flush()
{
    if (!msg.empty())
    {
        Logger::Debug("%s", msg.c_str());
        msg = "";
    }
}
}
