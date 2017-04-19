#pragma once

#include "Reflection/Reflection.h"

namespace DAVA
{
/**
 \ingroup formula
 
 Context provides access to data and methods. Formula uses contexts to find values 
 of variables or functions.
 */
class FormulaContext
{
public:
    FormulaContext(){};
    virtual ~FormulaContext(){};

    virtual AnyFn FindFunction(const String& name, const Vector<const Type*>& types) const = 0;
    virtual Reflection FindReflection(const String& name) const = 0;
};

/**
 \ingroup formula
 
 Default implementation of FormulaContext which uses Reflection.
 */
class FormulaReflectionContext : public FormulaContext
{
public:
    FormulaReflectionContext(const Reflection& ref, std::shared_ptr<FormulaContext> parent);
    ~FormulaReflectionContext() override;

    AnyFn FindFunction(const String& name, const Vector<const Type*>& types) const override;
    Reflection FindReflection(const String& name) const override;

    const Reflection& GetReflection() const;
    FormulaContext* GetParent() const;

private:
    bool IsArgsMatchToFn(const Vector<const Type*>& types, const AnyFn& fn) const;

    Reflection reflection;
    std::shared_ptr<FormulaContext> parent;
};
}
