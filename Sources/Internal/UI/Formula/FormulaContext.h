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

    virtual AnyFn FindFunction(const String& name, const Vector<const Type*>& types) const;
    virtual Reflection FindReflection(const String& name) const;
};

class FormulaReflectionContext : public FormulaContext
{
public:
    FormulaReflectionContext(const Reflection &ref, std::shared_ptr<FormulaContext> parent);
    ~FormulaReflectionContext() override;

    AnyFn FindFunction(const String& name, const Vector<const Type*>& types) const override;
    Reflection FindReflection(const String& name) const override;

    const Reflection &GetReflection() const;
    FormulaContext* GetParent() const;

private:
    bool IsArgsMatchToFn(const Vector<const Type*>& types, const AnyFn& fn) const;

    Reflection reflection;
    std::shared_ptr<FormulaContext> parent;
};

}
