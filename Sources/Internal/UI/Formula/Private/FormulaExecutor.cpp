#include "UI/Formula/Private/FormulaExecutor.h"

#include "UI/Formula/FormulaContext.h"
#include "UI/Formula/Private/FormulaException.h"
#include "UI/Formula/Private/FormulaFormatter.h"
#include "UI/Formula/Private/FormulaData.h"
#include "Utils/StringFormat.h"
#include "Reflection/ReflectedTypeDB.h"

namespace DAVA
{
FormulaExecutor::FormulaExecutor(FormulaContext* context_)
    : context(context_)
{
}

FormulaExecutor::~FormulaExecutor()
{
}

Any FormulaExecutor::Calculate(FormulaExpression* exp)
{
    return CalculateImpl(exp);
}

Reflection FormulaExecutor::GetDataReference(FormulaExpression* exp)
{
    return GetDataReferenceImpl(exp);
}

const Vector<void*>& FormulaExecutor::GetDependencies() const
{
    return dependencies;
}

void FormulaExecutor::Visit(FormulaValueExpression* exp)
{
    calculationResult = exp->GetValue();

    if (calculationResult.CanGet<std::shared_ptr<FormulaDataMap>>())
    {
        std::shared_ptr<FormulaDataMap> ptr = calculationResult.Get<std::shared_ptr<FormulaDataMap>>();
        dataReference = Reflection::Create(ReflectedObject(ptr.get()));
    }
    else if (calculationResult.CanGet<std::shared_ptr<FormulaDataVector>>())
    {
        std::shared_ptr<FormulaDataVector> ptr = calculationResult.Get<std::shared_ptr<FormulaDataVector>>();
        dataReference = Reflection::Create(ReflectedObject(ptr.get()));
    }
    else
    {
        dataReference = Reflection();
    }
}

void FormulaExecutor::Visit(FormulaNegExpression* exp)
{
    const Any& val = CalculateImpl(exp->GetExp());

    if (val.CanGet<float32>())
    {
        calculationResult = Any(-val.Get<float32>());
    }
    else if (val.CanGet<int64>())
    {
        calculationResult = Any(-val.Get<int64>());
    }
    else if (val.CanCast<int32>())
    {
        calculationResult = Any(-val.Cast<int32>());
    }
    else
    {
        DAVA_THROW(FormulaException, Format("Invalid argument type '%s' to unary '-' expression", FormulaFormatter::AnyTypeToString(val).c_str()), exp);
    }
}

void FormulaExecutor::Visit(FormulaNotExpression* exp)
{
    Any val = Calculate(exp->GetExp());
    if (val.CanGet<bool>())
    {
        calculationResult = Any(!val.Get<bool>());
    }
    else
    {
        DAVA_THROW(FormulaException, Format("Invalid argument type '%s' to unary '!' expression", FormulaFormatter::AnyTypeToString(val).c_str()), exp);
    }
}

void FormulaExecutor::Visit(FormulaBinaryOperatorExpression* exp)
{
    Any l = CalculateImpl(exp->GetLhs());
    Any r = CalculateImpl(exp->GetRhs());

    if (l.CanGet<uint64>() && r.CanGet<uint64>())
    {
        calculationResult = CalculateIntAnyValues<uint64>(exp->GetOperator(), l, r);
    }
    else if (l.CanGet<int64>() && r.CanGet<int64>())
    {
        calculationResult = CalculateIntAnyValues<int64>(exp->GetOperator(), l, r);
    }
    else if (l.CanGet<uint32>() && r.CanGet<uint32>())
    {
        calculationResult = CalculateIntAnyValues<uint32>(exp->GetOperator(), l, r);
    }
    else if (l.CanCast<int32>() && r.CanCast<int32>())
    {
        calculationResult = CalculateIntAnyValues<int32>(exp->GetOperator(), l, r);
    }
    else if ((l.CanGet<float32>() || l.CanCast<int32>()) && (r.CanGet<float32>() || r.CanCast<int32>()))
    {
        float32 lVal = l.CanGet<float32>() ? l.Get<float32>() : static_cast<float32>(l.Cast<int32>());
        float32 rVal = r.CanGet<float32>() ? r.Get<float32>() : static_cast<float32>(r.Cast<int32>());
        calculationResult = CalculateNumberValues<float32>(exp->GetOperator(), lVal, rVal);
    }
    else if (l.CanGet<bool>() && r.CanGet<bool>())
    {
        bool lVal = l.Get<bool>();
        bool rVal = r.Get<bool>();
        switch (exp->GetOperator())
        {
        case FormulaBinaryOperatorExpression::OP_AND:
            calculationResult = Any(lVal && rVal);
            break;

        case FormulaBinaryOperatorExpression::OP_OR:
            calculationResult = Any(lVal || rVal);
            break;

        case FormulaBinaryOperatorExpression::OP_EQ:
            calculationResult = Any(lVal == rVal);
            break;

        case FormulaBinaryOperatorExpression::OP_NOT_EQ:
            calculationResult = Any(lVal != rVal);
            break;

        default:
            DAVA_THROW(FormulaException, Format("Operator '%s' cannot be applied to '%s', '%s'",
                                                FormulaFormatter::BinaryOpToString(exp->GetOperator()).c_str(),
                                                FormulaFormatter::AnyTypeToString(l).c_str(),
                                                FormulaFormatter::AnyTypeToString(r).c_str()),
                       exp);
        }
    }
    else if (l.CanGet<String>() && r.CanGet<String>())
    {
        String lVal = l.Get<String>();
        String rVal = r.Get<String>();
        switch (exp->GetOperator())
        {
        case FormulaBinaryOperatorExpression::OP_PLUS:
            calculationResult = Any(lVal + rVal);
            break;

        case FormulaBinaryOperatorExpression::OP_EQ:
            calculationResult = Any(lVal == rVal);
            break;

        case FormulaBinaryOperatorExpression::OP_NOT_EQ:
            calculationResult = Any(lVal != rVal);
            break;

        default:
            DAVA_THROW(FormulaException, Format("Operator '%s' cannot be applied to '%s', '%s'",
                                                FormulaFormatter::BinaryOpToString(exp->GetOperator()).c_str(),
                                                FormulaFormatter::AnyTypeToString(l).c_str(),
                                                FormulaFormatter::AnyTypeToString(r).c_str()),
                       exp);
        }
    }
    else
    {
        DAVA_THROW(FormulaException, Format("Operator '%s' cannot be applied to '%s', '%s'",
                                            FormulaFormatter::BinaryOpToString(exp->GetOperator()).c_str(),
                                            FormulaFormatter::AnyTypeToString(l).c_str(),
                                            FormulaFormatter::AnyTypeToString(r).c_str()),
                   exp);
    }
}

void FormulaExecutor::Visit(FormulaFunctionExpression* exp)
{
    const Vector<std::shared_ptr<FormulaExpression>>& params = exp->GetParms();
    Vector<const Type*> types;
    types.reserve(params.size());

    Vector<Any> values;
    values.reserve(params.size());

    for (const std::shared_ptr<FormulaExpression>& paramExp : params)
    {
        Any res = CalculateImpl(paramExp.get());
        types.push_back(res.GetType());
        values.push_back(res);
    }

    AnyFn fn = context->FindFunction(exp->GetName(), types);
    if (!fn.IsValid())
    {
        String args;
        for (size_t i = 0; i < values.size(); i++)
        {
            if (i > 0)
            {
                args += ", ";
            }
            args += FormulaFormatter::AnyTypeToString(values[i]);
        }
        DAVA_THROW(FormulaException, Format("Can't resolve function '%s(%s)'", exp->GetName().c_str(), args.c_str()), exp);
    }

    int32 index = 0;
    for (Any& v : values)
    {
        if (v.CanCast<int32>() && fn.GetInvokeParams().argsType[index] == Type::Instance<float32>())
        {
            v = Any(static_cast<float32>(v.Cast<int32>()));
        }

        index++;
    }

    switch (params.size())
    {
    case 0:
        calculationResult = fn.Invoke();
        break;

    case 1:
        calculationResult = fn.Invoke(values[0]);
        break;

    case 2:
        calculationResult = fn.Invoke(values[0], values[1]);
        break;

    case 3:
        calculationResult = fn.Invoke(values[0], values[1], values[2]);
        break;

    case 4:
        calculationResult = fn.Invoke(values[0], values[1], values[2], values[3]);
        break;

    case 5:
        calculationResult = fn.Invoke(values[0], values[1], values[2], values[3], values[4]);
        break;

    case 6:
        calculationResult = fn.Invoke(values[0], values[1], values[2], values[3], values[4], values[5]);
        break;

    default:
    {
        String args;
        for (size_t i = 0; i < values.size(); i++)
        {
            if (i > 0)
            {
                args += ", ";
            }
            args += FormulaFormatter::AnyTypeToString(values[i]);
        }
        DAVA_THROW(FormulaException,
                   Format("Function '%s(%s)' has to much arguments (more than 6)",
                          exp->GetName().c_str(),
                          args.c_str()),
                   exp);
    }
    }
}

void FormulaExecutor::Visit(FormulaFieldAccessExpression* exp)
{
    Reflection res;
    if (exp->GetExp())
    {
        Reflection data = GetDataReference(exp->GetExp());
        if (data.IsValid())
        {
            dataReference = data.GetField(exp->GetFieldName());
        }
        else
        {
            dataReference = Reflection();
        }
    }
    else
    {
        dataReference = context->FindReflection(exp->GetFieldName());
    }

    if (dataReference.IsValid())
    {
        dependencies.push_back(dataReference.GetValueObject().GetVoidPtr());
    }
    else
    {
        DAVA_THROW(FormulaException, Format("Can't resolve symbol '%s'", exp->GetFieldName().c_str()), exp);
    }
}

void FormulaExecutor::Visit(FormulaIndexExpression* exp)
{
    Any indexVal = Calculate(exp->GetIndexExp());
    Reflection data = GetDataReference(exp->GetExp());
    if (data.IsValid())
    {
        dataReference = data.GetField(indexVal);

        if (dataReference.IsValid())
        {
            dependencies.push_back(dataReference.GetValueObject().GetVoidPtr());
        }
        else
        {
            DAVA_THROW(FormulaException, Format("Can't get data '%s' by index '%s' with type '%s'",
                                                FormulaFormatter().Format(exp).c_str(),
                                                FormulaFormatter::AnyToString(indexVal).c_str(),
                                                FormulaFormatter::AnyTypeToString(indexVal).c_str()),
                       exp);
        }
    }
    else
    {
        DAVA_THROW(FormulaException, Format("It's not data access expression '%s'", FormulaFormatter().Format(exp).c_str()), exp);
    }
}

const Any& FormulaExecutor::CalculateImpl(FormulaExpression* exp)
{
    dataReference = Reflection();
    calculationResult.Clear();

    exp->Accept(this);

    if (calculationResult.IsEmpty())
    {
        if (dataReference.IsValid())
        {
            calculationResult = dataReference.GetValue();
        }
        else
        {
            DAVA_THROW(FormulaException,
                       Format("Can't calculate expression '%s'",
                              FormulaFormatter().Format(exp).c_str()),
                       exp);
        }
    }

    dataReference = Reflection();

    return calculationResult;
}

const Reflection& FormulaExecutor::GetDataReferenceImpl(FormulaExpression* exp)
{
    dataReference = Reflection();
    calculationResult.Clear();

    exp->Accept(this);

    if (dataReference.IsValid())
    {
        calculationResult.Clear();
        return dataReference;
    }
    else
    {
        DAVA_THROW(FormulaException,
                   Format("Can't get data reference '%s'",
                          FormulaFormatter().Format(exp).c_str()),
                   exp);
    }
}

template <typename T>
Any FormulaExecutor::CalculateNumberAnyValues(FormulaBinaryOperatorExpression::Operator op, Any anyLVal, Any anyRVal) const
{
    T lVal = anyLVal.Cast<T>();
    T rVal = anyRVal.Cast<T>();
    return CalculateNumberValues<T>(op, lVal, rVal);
}

template <typename T>
Any FormulaExecutor::CalculateIntAnyValues(FormulaBinaryOperatorExpression::Operator op, Any anyLVal, Any anyRVal) const
{
    T lVal = anyLVal.Cast<T>();
    T rVal = anyRVal.Cast<T>();

    if (op == FormulaBinaryOperatorExpression::OP_MOD)
    {
        return Any(lVal % rVal);
    }
    else
    {
        return CalculateNumberValues<T>(op, lVal, rVal);
    }
}

template <typename T>
Any FormulaExecutor::CalculateNumberValues(FormulaBinaryOperatorExpression::Operator op, T lVal, T rVal) const
{
    switch (op)
    {
    case FormulaBinaryOperatorExpression::OP_PLUS:
        return Any(lVal + rVal);
    case FormulaBinaryOperatorExpression::OP_MINUS:
        return Any(lVal - rVal);
    case FormulaBinaryOperatorExpression::OP_MUL:
        return Any(lVal * rVal);
    case FormulaBinaryOperatorExpression::OP_DIV:
        return Any(lVal / rVal);
    case FormulaBinaryOperatorExpression::OP_EQ:
        return Any(lVal == rVal);
    case FormulaBinaryOperatorExpression::OP_NOT_EQ:
        return Any(lVal != rVal);
    case FormulaBinaryOperatorExpression::OP_LE:
        return Any(lVal <= rVal);
    case FormulaBinaryOperatorExpression::OP_LT:
        return Any(lVal < rVal);
    case FormulaBinaryOperatorExpression::OP_GE:
        return Any(lVal >= rVal);
    case FormulaBinaryOperatorExpression::OP_GT:
        return Any(lVal > rVal);

    default:
        DVASSERT("Invalid operands to binary expression");
        return Any();
    }
}
}
