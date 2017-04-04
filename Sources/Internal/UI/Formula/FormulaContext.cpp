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

AnyFn FormulaContext::FindFunction(const String& name, const Vector<const Type*>& types) const
{
    return AnyFn();
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

const Reflection &FormulaReflectionContext::GetReflection() const
{
    return reflection;
}
    
FormulaContext* FormulaReflectionContext::GetParent() const
{
    return parent.get();
}

bool FormulaReflectionContext::IsArgsMatchToFn(const Vector<const Type*>& types, const AnyFn& fn) const
{
    if (fn.GetInvokeParams().argsType.size() != types.size())
    {
        return false;
    }

    for (std::size_t i = 0; i < types.size(); i++)
    {
        const Type* type = types[i]->Decay();
        const Type* fnType = fn.GetInvokeParams().argsType[i]->Decay();

        if (type != fnType && (type != Type::Instance<int32>() && fnType != Type::Instance<float32>())) // allow conversion from int32 to float32
        {
            return false;
        }
    }

    return true;
}
}
