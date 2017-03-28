#pragma once

#include "Reflection/Reflection.h"
#include "UI/Formula/FormulaData.h"
#include "Math/Vector.h"

namespace DAVA
{
class FormulaContext
{
public:
    FormulaContext();
    virtual ~FormulaContext();

    virtual const AnyFn* FindFunction(const String& name, const Vector<const Type*>& types) const;
    virtual Reflection FindReflection(const String& name) const;

private:
};

class FormulaReflectionContext : public FormulaContext
{
public:
    FormulaReflectionContext(const Reflection &ref, std::shared_ptr<FormulaContext> parent);
    ~FormulaReflectionContext() override;

    const AnyFn* FindFunction(const String& name, const Vector<const Type*>& types) const override;
    Reflection FindReflection(const String& name) const override;

    const Reflection &GetReflection() const;
    FormulaContext* GetParent() const;

private:
    Reflection reflection;
    std::shared_ptr<FormulaContext> parent;
};


class FormulaFuncContext : public FormulaContext
{
public:
    FormulaFuncContext();
    ~FormulaFuncContext() override;

    const AnyFn* FindFunction(const String& name, const Vector<const Type*>& types) const override;
    void RegisterFunction(const String& name, const AnyFn& fn);

private:
    String FuncLocalize(const String& key);
    Vector2 FuncVec(float x, float y);
    String FuncFormatTime(const String format, int32 time);
    String FuncFormatBool(bool value);
    String FuncFormatInt(int32 value);
    String FuncFormatVec2(const Vector2& value);
    String FuncFormatFloat(float32 value);
    String FuncFormatFloatPrec(float32 value, int32 precision);
    void ParseIntFractionalPart(float32 value, int32 precision, int32& sign, int32& intPart, float32& fractionalPart);
    String FuncRoman(int32 value);
    int32 FuncMod(int32 value, int32 mod);

    UnorderedMap<String, Vector<AnyFn>> functions;
};
}
