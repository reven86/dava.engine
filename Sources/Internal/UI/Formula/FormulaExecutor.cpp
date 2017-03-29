#include "FormulaExecutor.h"

#include "UI/Formula/FormulaContext.h"
#include "UI/Formula/FormulaError.h"
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

Any FormulaExecutor::Execute(FormulaExpression* exp)
{
    exp->Accept(this);

    return res;
}

void FormulaExecutor::Visit(FormulaValueExpression* exp)
{
    res = exp->GetValue();

    if (res.CanGet<std::shared_ptr<FormulaDataMap>>())
    {
        std::shared_ptr<FormulaDataMap> ptr = res.Get<std::shared_ptr<FormulaDataMap>>();
        data = Reflection::Create(ReflectedObject(ptr.get()));
    }
    else if (res.CanGet<std::shared_ptr<FormulaDataVector>>())
    {
        std::shared_ptr<FormulaDataVector> ptr = res.Get<std::shared_ptr<FormulaDataVector>>();
        data = Reflection::Create(ReflectedObject(ptr.get()));
    }
    else
    {
        data = Reflection();
    }
}

void FormulaExecutor::Visit(FormulaNegExpression* exp)
{
    Any val = Calculate(exp->GetExp());
    if (res.CanGet<float32>())
    {
        res = Any(-val.Get<float32>());
    }
    else if (val.CanCast<int32>())
    {
        res = Any(-val.Cast<int32>());
    }
    else
    {
        throw FormulaCalculationError("");
    }
}

void FormulaExecutor::Visit(FormulaNotExpression* exp)
{
    Any val = Calculate(exp->GetExp());
    if (val.CanGet<bool>())
    {
        res = Any(!val.Get<bool>());
    }
    else
    {
        throw FormulaCalculationError("");
    }
}

void FormulaExecutor::Visit(FormulaBinaryOperatorExpression* exp)
{
    Any l = Calculate(exp->GetLhs());
    Any r = Calculate(exp->GetRhs());

    bool isLhsInt = l.CanGet<int>();
    bool isRhsInt = r.CanGet<int>();

    if (isLhsInt && isRhsInt)
    {
        int lVal = l.Get<int>();
        int rVal = r.Get<int>();

        if (exp->GetOperator() == FormulaBinaryOperatorExpression::OP_MOD)
        {
            res = Any(lVal % rVal);
        }
        else
        {
            res = CalculateNumberValues(exp->GetOperator(), lVal, rVal);
        }
    }
    else
    {
        bool isLhsFloat = l.CanGet<float>();
        bool isRhsFloat = r.CanGet<float>();
        if ((isLhsFloat || isLhsInt) && (isRhsFloat || isRhsInt))
        {
            float lVal = isLhsFloat ? l.Get<float>() : l.Get<int>();
            float rVal = isRhsFloat ? r.Get<float>() : r.Get<int>();
            res = CalculateNumberValues(exp->GetOperator(), lVal, rVal);
        }
        else if (l.CanGet<bool>() && r.CanGet<bool>())
        {
            bool lVal = l.Get<bool>();
            bool rVal = r.Get<bool>();
            switch (exp->GetOperator())
            {
            case FormulaBinaryOperatorExpression::OP_AND:
                res = Any(lVal && rVal);
                break;

            case FormulaBinaryOperatorExpression::OP_OR:
                res = Any(lVal || rVal);
                break;

            case FormulaBinaryOperatorExpression::OP_EQ:
                res = Any(lVal == rVal);
                break;

            case FormulaBinaryOperatorExpression::OP_NOT_EQ:
                res = Any(lVal != rVal);
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
                res = Any(lVal + rVal);
                break;

            case FormulaBinaryOperatorExpression::OP_EQ:
                res = Any(lVal == rVal);
                break;

            case FormulaBinaryOperatorExpression::OP_NOT_EQ:
                res = Any(lVal != rVal);
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
        Any res = Calculate(exp.get());
        if (res.IsEmpty())
        {
            throw FormulaCalculationError("Can't execute function ");
        }
        types.push_back(res.GetType());
        values.push_back(res);
    }

    const AnyFn* fn = context->FindFunction(exp->GetName(), types);
    if (fn != nullptr)
    {
        throw FormulaCalculationError(Format("Function '%s' not found.", exp->GetName().c_str()));
    }
    int32 index = 0;
    for (Any& v : values)
    {
        if (v.GetType() == Type::Instance<int>() && fn->GetInvokeParams().argsType[index] == Type::Instance<float>())
        {
            v = Any(static_cast<float>(v.Get<int>()));
        }

        index++;
    }

    switch (params.size())
    {
    case 0:
        res = fn->Invoke();
        break;

    case 1:
        res = fn->Invoke(values[0]);
        break;

    case 2:
        res = fn->Invoke(values[0], values[1]);
        break;

    case 3:
        res = fn->Invoke(values[0], values[1], values[2]);
        break;

    case 4:
        res = fn->Invoke(values[0], values[1], values[2], values[3]);
        break;

    case 5:
        res = fn->Invoke(values[0], values[1], values[2], values[3], values[4]);
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
        Reflection d = GetData(exp->GetExp());
        if (d.IsValid())
        {
            data = data.GetField(exp->GetFieldName());
            dependencies.push_back(data.GetValueObject().GetVoidPtr());
        }
        else
        {
            data = Reflection();
        }
    }
    else
    {
        data = context->FindReflection(exp->GetFieldName());
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
        throw FormulaCalculationError("Type of index expression must be int");
    }

    Reflection d = GetData(exp->GetExp());

    if (d.IsValid())
    {
        data = d.GetField(index);
        dependencies.push_back(data.GetValueObject().GetVoidPtr());
    }
    else
    {
        data = Reflection();
    }
}

const Any& FormulaExecutor::Calculate(FormulaExpression* exp)
{
    res = Any();
    data = Reflection();

    exp->Accept(this);

    if (data.IsValid())
    {
        res = data.GetValue();
    }

    return res;
}

const Reflection& FormulaExecutor::GetData(FormulaExpression* exp)
{
    res = Any();
    data = Reflection();

    exp->Accept(this);

    if (data.IsValid())
    {
        return data;
    }
    else
    {
        throw FormulaCalculationError("Type of index expression must be int");
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
