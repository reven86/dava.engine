#include "FormulaContext.h"

#include "UI/Formula/FormulaParser.h"
#include "Functional/Function.h"
#include "FileSystem/LocalizationSystem.h"
#include "FileSystem/File.h"
#include "Utils/StringFormat.h"
#include "Utils/Utils.h"
#include "Logger/Logger.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
using std::make_shared;
using std::shared_ptr;

FormulaContext::FormulaContext()
{
}

FormulaContext::~FormulaContext()
{
}

const AnyFn* FormulaContext::FindFunction(const String& name, const Vector<const Type*>& types) const
{
    return nullptr;
}

Reflection FormulaContext::FindReflection(const String& name) const
{
    return Reflection();
}

FormulaReflectionContext::FormulaReflectionContext(const Reflection &ref_, std::shared_ptr<FormulaContext> parent_)
    : reflection(ref_)
    , parent(parent_)
{
    
}

FormulaReflectionContext::~FormulaReflectionContext()
{
    
}

const AnyFn* FormulaReflectionContext::FindFunction(const String& name, const Vector<const Type*>& types) const
{
    if (parent)
    {
        return parent->FindFunction(name, types);
    }
    return nullptr;
}

Reflection FormulaReflectionContext::FindReflection(const String& name) const
{
    Reflection res = reflection.GetField(name);
    if (res.IsValid())
    {
        return res;
    }
    
    if (parent)
    {
        return parent->FindReflection(name);
    }
    
    return Reflection();

}

const Reflection &FormulaReflectionContext::GetReflection() const
{
    return reflection;
}
    
FormulaContext* FormulaReflectionContext::GetParent() const
{
    return parent.get();
}

FormulaFuncContext::FormulaFuncContext()
{
    RegisterFunction("localize", MakeFunction(this, &FormulaFuncContext::FuncLocalize));
    RegisterFunction("vec", MakeFunction(this, &FormulaFuncContext::FuncVec));
    RegisterFunction("roman", MakeFunction(this, &FormulaFuncContext::FuncRoman));
    RegisterFunction("formatTime", MakeFunction(this, &FormulaFuncContext::FuncFormatTime));
    RegisterFunction("format", MakeFunction(this, &FormulaFuncContext::FuncFormatInt));
    RegisterFunction("format", MakeFunction(this, &FormulaFuncContext::FuncFormatBool));
    RegisterFunction("format", MakeFunction(this, &FormulaFuncContext::FuncFormatVec2));
    RegisterFunction("format", MakeFunction(this, &FormulaFuncContext::FuncFormatFloatPrec));
    RegisterFunction("format", MakeFunction(this, &FormulaFuncContext::FuncFormatFloat));
    RegisterFunction("mod", MakeFunction(this, &FormulaFuncContext::FuncMod));
}

FormulaFuncContext::~FormulaFuncContext()
{
}

const AnyFn* FormulaFuncContext::FindFunction(const String& name, const Vector<const Type*>& types) const
{
    auto it = functions.find(name);
    if (it != functions.end())
    {
        const AnyFn* res = nullptr;
        for (const AnyFn& fn : it->second)
        {
            if (fn.GetInvokeParams().argsType.size() == types.size())
            {
                int mt = 2; // 2 = full match, 1 = need convertation, 0 =

                for (std::size_t i = 0; i < types.size(); i++)
                {
                    const Type* type = types[i];
                    const Type* fnType = fn.GetInvokeParams().argsType[i];

                    if (type != fnType && type->Pointer() != fnType->Pointer())
                    {
                        if (fnType == Type::Instance<float>() && type == Type::Instance<int>())
                        {
                            mt = 1;
                        }
                        else
                        {
                            mt = 0;
                            break;
                        }
                    }
                }

                if (mt == 2)
                {
                    return &fn;
                }
                if (mt == 1)
                {
                    res = &fn;
                }
            }
        }
        return res;
    }
    return nullptr;
}

void FormulaFuncContext::RegisterFunction(const String& name, const AnyFn& fn)
{
    auto it = functions.find(name);
    if (it != functions.end())
    {
        it->second.push_back(fn);
    }
    else
    {
        Vector<AnyFn> vector;
        vector.push_back(fn);
        functions[name] = vector;
    }
}

String FormulaFuncContext::FuncLocalize(const String& key)
{
    return LocalizedUtf8String(key);
}

Vector2 FormulaFuncContext::FuncVec(float x, float y)
{
    return Vector2(x, y);
}

String FormulaFuncContext::FuncFormatTime(const String format, int32 time)
{
    tm localtime;
    time_t t = time;
    localtime_r(&t, &localtime);

    char tmp[256];
    strftime(tmp, 256, format.c_str(), &localtime);
    return String(tmp);
}

DAVA::String FormulaFuncContext::FuncFormatBool(bool value)
{
    return value ? "true" : "false";
}

DAVA::String FormulaFuncContext::FuncFormatInt(DAVA::int32 value)
{
    String resultStr;

    bool negative = value < 0;
    value = Abs(value);

    while (value >= 1000)
    {
        resultStr = Format(" %03d", value % 1000) + resultStr;
        value /= 1000;
    }

    value = negative ? -value : value;
    resultStr = Format("%d", value) + resultStr;

    return resultStr;
}

String FormulaFuncContext::FuncFormatVec2(const Vector2& value)
{
    return Format("%f, %f", value.x, value.y);
}

DAVA::String FormulaFuncContext::FuncFormatFloat(DAVA::float32 value)
{
    return FuncFormatFloatPrec(value, 2);
}

DAVA::String FormulaFuncContext::FuncFormatFloatPrec(DAVA::float32 value, int32 precision)
{
    bool skipLastZero = false;
    String resultStr;

    int32 sign;
    int32 intPart;
    float32 fractionalPart;

    ParseIntFractionalPart(value, precision, sign, intPart, fractionalPart);

    if (intPart == 0)
    {
        resultStr = "0";
    }
    else
    {
        int32 position = 0;
        while (intPart > 0)
        {
            char digit = '0' + intPart % 10;
            intPart /= 10;
            resultStr = digit + resultStr;

            position++;
            if (position == 3 && intPart > 0)
            {
                resultStr = " " + resultStr;
                position = 0;
            }
        }
    }

    if (precision > 0 && skipLastZero)
    {
        float32 testPart = fractionalPart;
        int32 newPrecission = 0;
        for (int32 i = 0; i < precision; i++)
        {
            testPart *= 10;
            int32 digit = (int32)testPart;
            testPart -= digit;
            if (digit > 0)
                newPrecission = i + 1;
        }
        precision = newPrecission;
    }

    if (precision > 0)
    {
        resultStr += ".";
        for (int32 i = 0; i < precision; i++)
        {
            fractionalPart *= 10;
            int32 digit = (int32)fractionalPart;
            fractionalPart -= digit;
            resultStr += ('0' + digit);
        }
    }

    if (sign < 0)
        resultStr = "-" + resultStr;

    return resultStr;
}

void FormulaFuncContext::ParseIntFractionalPart(float32 value, int32 precision, int32& sign, int32& intPart, float32& fractionalPart)
{
    sign = value < 0 ? -1 : 1;
    float32 r = 0.5f * sign;
    for (int32 i = 0; i < precision; i++)
        r /= 10.0f;

    intPart = (int32)Abs(value + r);
    fractionalPart = Abs(value + r) - intPart;
}

String FormulaFuncContext::FuncRoman(int32 value)
{
    static String levelStr[10] = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX", "X" };
    if (value < 1 || value > 10)
        return "";

    return levelStr[value - 1];
}

int32 FormulaFuncContext::FuncMod(int32 value, int32 mod)
{
    return value % mod;
}

}
