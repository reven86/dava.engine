#include "UI/Formula/FormulaContext.h"
#include "Reflection/ReflectedTypeDB.h"

namespace DAVA
{
using std::make_shared;
using std::shared_ptr;

FormulaReflectionContext::FormulaReflectionContext(const Reflection& ref_, std::shared_ptr<FormulaContext> parent_)
    : reflection(ref_)
    , parent(parent_)
{
}

FormulaReflectionContext::~FormulaReflectionContext()
{
}

AnyFn FormulaReflectionContext::FindFunction(const String& name, const Vector<const Type*>& types) const
{
    AnyFn method = reflection.GetMethod(name);

    if (method.IsValid() && IsArgsMatchToFn(types, method))
    {
        return method;
    }

    if (parent)
    {
        return parent->FindFunction(name, types);
    }

    return AnyFn();
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

const Reflection& FormulaReflectionContext::GetReflection() const
{
    return reflection;
}

FormulaContext* FormulaReflectionContext::GetParent() const
{
    return parent.get();
}

bool FormulaReflectionContext::IsArgsMatchToFn(const Vector<const Type*>& types, const AnyFn& fn) const
{
    const Vector<const Type*>& fnTypes = fn.GetInvokeParams().argsType;

    if (fnTypes.size() != types.size())
    {
        return false;
    }

    for (std::size_t i = 0; i < types.size(); i++)
    {
        const Type* type = types[i];
        const Type* fnType = fnTypes[i]->Decay();

        const Type* int32T = Type::Instance<int32>();
        const Type* float32T = Type::Instance<float32>();
        if (type != fnType && (type != int32T || fnType != float32T)) // allow conversion from int32 to float32
        {
            return false;
        }
    }

    return true;
}
}
