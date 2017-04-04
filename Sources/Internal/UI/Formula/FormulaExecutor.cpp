#include "FormulaExecutor.h"

#include "UI/Formula/FormulaContext.h"
#include "UI/Formula/FormulaError.h"
#include "UI/Formula/FormulaFormatter.h"
#include "Utils/StringFormat.h"

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
    else if (val.CanCast<int32>())
    {
        calculationResult = Any(-val.Cast<int32>());
    }
    else
    {
        throw FormulaCalculationError("Invalid argument type to unary expression");
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
        throw FormulaCalculationError("Invalid argument type to unary expression");
    }
}

void FormulaExecutor::Visit(FormulaBinaryOperatorExpression* exp)
{
    Any l = CalculateImpl(exp->GetLhs());
    Any r = CalculateImpl(exp->GetRhs());

    bool isLhsInt = l.CanGet<int32>();
    bool isRhsInt = r.CanGet<int32>();

    if (isLhsInt && isRhsInt)
    {
        int32 lVal = l.Get<int32>();
        int32 rVal = r.Get<int32>();

        if (exp->GetOperator() == FormulaBinaryOperatorExpression::OP_MOD)
        {
            calculationResult = Any(lVal % rVal);
        }
        else
        {
            calculationResult = CalculateNumberValues(exp->GetOperator(), lVal, rVal);
        }
    }
    else
    {
        bool isLhsFloat = l.CanGet<float32>();
        bool isRhsFloat = r.CanGet<float32>();
        if ((isLhsFloat || isLhsInt) && (isRhsFloat || isRhsInt))
        {
            float32 lVal = isLhsFloat ? l.Get<float32>() : l.Get<int32>();
            float32 rVal = isRhsFloat ? r.Get<float32>() : r.Get<int32>();
            calculationResult = CalculateNumberValues(exp->GetOperator(), lVal, rVal);
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
                throw FormulaCalculationError("Invalid operands to binary expression");
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
                throw FormulaCalculationError("Invalid operands to binary expression");
            }
        }
    }

    throw FormulaCalculationError("Invalid operands to binary expression");
}

void FormulaExecutor::Visit(FormulaFunctionExpression* exp)
{
    const Vector<std::shared_ptr<FormulaExpression>>& params = exp->GetParms();
    Vector<const Type*> types;
    types.reserve(params.size());

    Vector<Any> values;
    values.reserve(params.size());

    for (const std::shared_ptr<FormulaExpression>& exp : params)
    {
        Any res = CalculateImpl(exp.get());
        if (res.IsEmpty())
        {
            throw FormulaCalculationError("Can't execute function ");
        }
        types.push_back(res.GetType());
        values.push_back(res);
    }

    AnyFn fn = context->FindFunction(exp->GetName(), types);
    if (!fn.IsValid())
    {
        throw FormulaCalculationError(Format("Can't resolve function '%s'", exp->GetName().c_str()));
    }

    int32 index = 0;
    for (Any& v : values)
    {
        if (v.GetType() == Type::Instance<int32>() && fn.GetInvokeParams().argsType[index] == Type::Instance<float32>())
        {
            v = Any(static_cast<float32>(v.Get<int32>()));
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

    default:
        throw FormulaCalculationError(Format("To much function arguments (%d)", params.size()));
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
        throw FormulaCalculationError(Format("Can't resolve symbol %s", exp->GetFieldName().c_str()));
    }
}

void FormulaExecutor::Visit(FormulaIndexExpression* exp)
{
    Any indexVal = Calculate(exp->GetIndexExp());

    int32 index = -1;
    if (indexVal.CanCast<int32>())
    {
        index = indexVal.Cast<int32>();
    }
    else
    {
        throw FormulaCalculationError(Format("Type of index expression (%s) must be int", FormulaFormatter().Format(exp->GetIndexExp()).c_str()));
    }

    Reflection data = GetDataReference(exp->GetExp());

    if (data.IsValid())
    {
        dataReference = data.GetField(index);
    }
    else
    {
        throw FormulaCalculationError(Format("It's not data access expression (%s)", FormulaFormatter().Format(exp).c_str()));
    }

    if (dataReference.IsValid())
    {
        dependencies.push_back(dataReference.GetValueObject().GetVoidPtr());
    }
    else
    {
        throw FormulaCalculationError(Format("Can't get data by index (%s)", FormulaFormatter().Format(exp).c_str()));
    }
}

const Any& FormulaExecutor::CalculateImpl(FormulaExpression* exp)
{
    exp->Accept(this);

    if (calculationResult.IsEmpty())
    {
        if (dataReference.IsValid())
        {
            calculationResult = dataReference.GetValue();
        }
        else
        {
            throw FormulaCalculationError("Can't calculate expression");
        }
    }

    dataReference = Reflection();

    return calculationResult;
}

const Reflection& FormulaExecutor::GetDataReferenceImpl(FormulaExpression* exp)
{
    exp->Accept(this);

    if (dataReference.IsValid())
    {
        calculationResult.Clear();
        return dataReference;
    }
    else
    {
        throw FormulaCalculationError("Can't get data reference.");
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
