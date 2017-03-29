#include "FormulaExpression.h"

#include "Debug/DVAssert.h"

#include "UI/Formula/FormulaContext.h"
#include "UI/Formula/FormulaError.h"
#include "UI/Formula/FormulaData.h"
#include "Reflection/ReflectedTypeDB.h"

#include "Utils/StringFormat.h"

namespace DAVA
{
using std::shared_ptr;

FormulaExpression::FormulaExpression()
{
}

FormulaExpression::~FormulaExpression()
{
}

bool FormulaExpression::ApplyValue(FormulaContext* context, const Any& value) const
{
    DVASSERT(false); // TODO: change to exception
    return false;
}

Reflection FormulaExpression::GetData(FormulaContext* context) const
{
    DVASSERT(false); //TODO: change to exception
    return Reflection();
}

bool FormulaExpression::IsValue() const
{
    return false;
}

FormulaValueExpression::FormulaValueExpression(const Any& value_)
    : value(value_)
{
}

Any FormulaValueExpression::Calculate(FormulaContext* context) const
{
    return value;
}

Reflection FormulaValueExpression::GetData(FormulaContext* context) const
{
    if (value.CanGet<shared_ptr<FormulaDataMap>>())
    {
        shared_ptr<FormulaDataMap> ptr = value.Get<shared_ptr<FormulaDataMap>>();
        return Reflection::Create(ReflectedObject(ptr.get()));
    }
    if (value.CanGet<shared_ptr<FormulaDataVector>>())
    {
        shared_ptr<FormulaDataVector> ptr = value.Get<shared_ptr<FormulaDataVector>>();
        return Reflection::Create(ReflectedObject(ptr.get()));
    }
    return Reflection::Create(value);
}

void FormulaValueExpression::Accept(FormulaExpressionVisitor* visitor)
{
    visitor->Visit(this);
}

bool FormulaValueExpression::IsValue() const
{
    return true;
}

void FormulaValueExpression::CollectDepencencies(FormulaContext *context, Vector<void*> &dependencies) const
{
    // do nothing
}
    
const Any& FormulaValueExpression::GetValue() const
{
    return value;
}

FormulaNegExpression::FormulaNegExpression(const std::shared_ptr<FormulaExpression>& exp_)
    : exp(exp_)
{
}

Any FormulaNegExpression::Calculate(FormulaContext* context) const
{
    Any res = exp->Calculate(context);
    if (res.CanGet<int>())
    {
        return Any(-res.Get<int>());
    }
    if (res.CanGet<float>())
    {
        return Any(-res.Get<float>());
    }

    DVASSERT(false);
    return Any();
}

void FormulaNegExpression::Accept(FormulaExpressionVisitor* visitor)
{
    visitor->Visit(this);
}

void FormulaNegExpression::CollectDepencencies(FormulaContext *context, Vector<void*> &dependencies) const
{
    exp->CollectDepencencies(context, dependencies);
}
    
FormulaExpression* FormulaNegExpression::GetExp() const
{
    return exp.get();
}

FormulaNotExpression::FormulaNotExpression(const std::shared_ptr<FormulaExpression>& exp_)
    : exp(exp_)
{
}

Any FormulaNotExpression::Calculate(FormulaContext* context) const
{
    Any res = exp->Calculate(context);
    if (res.CanGet<bool>())
    {
        return Any(!res.Get<bool>());
    }

    DVASSERT(false);
    return Any();
}

void FormulaNotExpression::Accept(FormulaExpressionVisitor* visitor)
{
    visitor->Visit(this);
}

void FormulaNotExpression::CollectDepencencies(FormulaContext *context, Vector<void*> &dependencies) const
{
    exp->CollectDepencencies(context, dependencies);
}

FormulaExpression* FormulaNotExpression::GetExp() const
{
    return exp.get();
}

FormulaBinaryOperatorExpression::FormulaBinaryOperatorExpression(Operator op_, const std::shared_ptr<FormulaExpression>& lhs_, const std::shared_ptr<FormulaExpression>& rhs_)
    : op(op_)
    , lhs(lhs_)
    , rhs(rhs_)
{
}

Any FormulaBinaryOperatorExpression::Calculate(FormulaContext* context) const
{
    Any l = lhs->Calculate(context);
    Any r = rhs->Calculate(context);

    bool isLhsInt = l.CanGet<int>();
    bool isRhsInt = r.CanGet<int>();

    if (isLhsInt && isRhsInt)
    {
        int lVal = l.Get<int>();
        int rVal = r.Get<int>();

        if (op == OP_MOD)
        {
            return Any(lVal % rVal);
        }

        return CalculateNumberValues(op, lVal, rVal);
    }

    bool isLhsFloat = l.CanGet<float>();
    bool isRhsFloat = r.CanGet<float>();
    if ((isLhsFloat || isLhsInt) && (isRhsFloat || isRhsInt))
    {
        float lVal = isLhsFloat ? l.Get<float>() : l.Get<int>();
        float rVal = isRhsFloat ? r.Get<float>() : r.Get<int>();
        return CalculateNumberValues(op, lVal, rVal);
    }

    if (l.CanGet<bool>() && r.CanGet<bool>())
    {
        bool lVal = l.Get<bool>();
        bool rVal = r.Get<bool>();
        switch (op)
        {
        case OP_AND:
            return Any(lVal && rVal);
        case OP_OR:
            return Any(lVal || rVal);
        case OP_EQ:
            return Any(lVal == rVal);
        case OP_NOT_EQ:
            return Any(lVal != rVal);
        default:
            throw FormulaCalculationError("Invalid operands to binary expression");
        }
    }

    if (l.CanGet<String>() && r.CanGet<String>())
    {
        String lVal = l.Get<String>();
        String rVal = r.Get<String>();
        switch (op)
        {
        case OP_PLUS:
            return Any(lVal + rVal);
        case OP_EQ:
            return Any(lVal == rVal);
        case OP_NOT_EQ:
            return Any(lVal != rVal);
        default:
            throw FormulaCalculationError("Invalid operands to binary expression");
        }
    }

    throw FormulaCalculationError("Invalid operands to binary expression");
}

FormulaBinaryOperatorExpression::Operator FormulaBinaryOperatorExpression::GetOperator() const
{
    return op;
}

String FormulaBinaryOperatorExpression::GetOperatorAsString() const
{
    switch (op)
    {
    case OP_PLUS:
        return "+";
    case OP_MINUS:
        return "-";
    case OP_MUL:
        return "*";
    case OP_DIV:
        return "/";
    case OP_AND:
        return "&&";
    case OP_OR:
        return "||";
    case OP_EQ:
        return "==";
    case OP_NOT_EQ:
        return "!=";
    case OP_LE:
        return "<=";
    case OP_LT:
        return "<";
    case OP_GE:
        return ">=";
    case OP_GT:
        return ">";

    default:
        DVASSERT("Invalid operator.");
        return "?";
    }
}

FormulaExpression* FormulaBinaryOperatorExpression::GetLhs() const
{
    return lhs.get();
}

FormulaExpression* FormulaBinaryOperatorExpression::GetRhs() const
{
    return rhs.get();
}

template <typename T>
Any FormulaBinaryOperatorExpression::CalculateNumberValues(Operator op, T lVal, T rVal) const
{
    switch (op)
    {
    case OP_PLUS:
        return Any(lVal + rVal);
    case OP_MINUS:
        return Any(lVal - rVal);
    case OP_MUL:
        return Any(lVal * rVal);
    case OP_DIV:
        return Any(lVal / rVal);
    case OP_EQ:
        return Any(lVal == rVal);
    case OP_NOT_EQ:
        return Any(lVal != rVal);
    case OP_LE:
        return Any(lVal <= rVal);
    case OP_LT:
        return Any(lVal < rVal);
    case OP_GE:
        return Any(lVal >= rVal);
    case OP_GT:
        return Any(lVal > rVal);

    default:
        DVASSERT("Invalid operands to binary expression");
        return Any();
    }
}

void FormulaBinaryOperatorExpression::Accept(FormulaExpressionVisitor* visitor)
{
    visitor->Visit(this);
}

void FormulaBinaryOperatorExpression::CollectDepencencies(FormulaContext *context, Vector<void*> &dependencies) const
{
    lhs->CollectDepencencies(context, dependencies);
    rhs->CollectDepencencies(context, dependencies);
}
    
int32 FormulaBinaryOperatorExpression::GetOperatorPriority() const
{
    switch (op)
    {
    case OP_MUL:
    case OP_DIV:
    case OP_MOD:
        return 1;

    case OP_PLUS:
    case OP_MINUS:
        return 2;

    case OP_LE:
    case OP_LT:
    case OP_GE:
    case OP_GT:
        return 3;

    case OP_EQ:
    case OP_NOT_EQ:
        return 4;

    case OP_AND:
        return 5;

    case OP_OR:
        return 6;

    default:
        DVASSERT("Invalid operator.");
        return 0;
    }
}

FormulaFunctionExpression::FormulaFunctionExpression(const String& name_, const Vector<std::shared_ptr<FormulaExpression>>& params_)
    : name(name_)
{
    params.reserve(params_.size());

    for (const std::shared_ptr<FormulaExpression>& exp : params_)
    {
        params.push_back(exp);
    }
}

Any FormulaFunctionExpression::Calculate(FormulaContext* context) const
{
    Vector<const Type*> types;
    types.reserve(params.size());

    Vector<Any> values;
    values.reserve(params.size());

    for (const shared_ptr<FormulaExpression>& exp : params)
    {
        Any res = exp->Calculate(context);
        if (res.IsEmpty())
        {
            return Any();
        }
        types.push_back(res.GetType());
        values.push_back(res);
    }

    const AnyFn* fn = context->FindFunction(name, types);
    if (fn != nullptr)
    {
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
            return fn->Invoke();

        case 1:
            return fn->Invoke(values[0]);

        case 2:
            return fn->Invoke(values[0], values[1]);

        case 3:
            return fn->Invoke(values[0], values[1], values[2]);

        case 4:
            return fn->Invoke(values[0], values[1], values[2], values[3]);

        case 5:
            return fn->Invoke(values[0], values[1], values[2], values[3], values[4]);

        default:
            DVASSERT(false);
            return Any();
        }
    }

    //    DVASSERT(false);
    return Any();
}

Reflection FormulaFunctionExpression::GetData(FormulaContext* context) const
{
    Any res = Calculate(context);
    return Reflection::Create(res);
}

const String& FormulaFunctionExpression::GetName() const
{
    return name;
}

const Vector<std::shared_ptr<FormulaExpression>>& FormulaFunctionExpression::GetParms() const
{
    return params;
}

void FormulaFunctionExpression::Accept(FormulaExpressionVisitor* visitor)
{
    visitor->Visit(this);
}

void FormulaFunctionExpression::CollectDepencencies(FormulaContext *context, Vector<void*> &dependencies) const
{
    for (const shared_ptr<FormulaExpression>& exp : params)
    {
        exp->CollectDepencencies(context, dependencies);
    }
}

FormulaFieldAccessExpression::FormulaFieldAccessExpression(const std::shared_ptr<FormulaExpression>& exp_, const String& fieldName_)
    : exp(exp_)
    , fieldName(fieldName_)
{
}

Any FormulaFieldAccessExpression::Calculate(FormulaContext* context) const
{
    Reflection ref = GetData(context);
    if (ref.IsValid())
    {
        Any res = ref.GetValue();
        if (res.CanCast<shared_ptr<FormulaExpression>>())
        {
            shared_ptr<FormulaExpression> exp = res.Cast<shared_ptr<FormulaExpression>>();
            return exp->Calculate(context);
        }

        return res;
    }
    else
    {
        DVASSERT(false);
        return Any(); // TODO: Fix
    }
}

bool FormulaFieldAccessExpression::ApplyValue(FormulaContext* context, const Any& value) const
{
    Reflection ref = GetData(context);
    if (ref.GetValue() != value)
    {
        return ref.SetValue(value);
    }
    return false;
}

Reflection FormulaFieldAccessExpression::GetData(FormulaContext* context) const
{
    Reflection res;
    if (exp)
    {
        Reflection data = exp->GetData(context);
        if (data.IsValid())
        {
            res = data.GetField(fieldName);
        }
        else
        {
            DVASSERT(false);
        }
    }
    else
    {
        res = context->FindReflection(fieldName);
    }

    if (!res.IsValid())
    {
        throw FormulaCalculationError(Format("Data not found (%s)", fieldName.c_str()));
    }
    return res;
}

void FormulaFieldAccessExpression::Accept(FormulaExpressionVisitor* visitor)
{
    visitor->Visit(this);
}

void FormulaFieldAccessExpression::CollectDepencencies(FormulaContext *context, Vector<void*> &dependencies) const
{
    if (exp)
    {
        exp->CollectDepencencies(context, dependencies);
        
        Reflection data = exp->GetData(context);
        if (data.IsValid())
        {
            Reflection res = data.GetField(fieldName);
            if (res.IsValid())
            {
                dependencies.push_back(res.GetValueObject().GetVoidPtr());
            }
            else
            {
                DVASSERT(false);
            }
        }
        else
        {
            DVASSERT(false);
        }
    }
    else
    {
        Reflection data = context->FindReflection(fieldName);
        if (data.IsValid())
        {
            dependencies.push_back(data.GetValueObject().GetVoidPtr());
        }
//        else
//        {
//             DVASSERT(false);
//        }
    }
}

FormulaExpression* FormulaFieldAccessExpression::GetExp() const
{
    return exp.get();
}

const String& FormulaFieldAccessExpression::GetFieldName() const
{
    return fieldName;
}

FormulaIndexExpression::FormulaIndexExpression(const std::shared_ptr<FormulaExpression>& exp_, const std::shared_ptr<FormulaExpression>& indexExp_)
    : exp(exp_)
    , indexExp(indexExp_)
{
}

Any FormulaIndexExpression::Calculate(FormulaContext* context) const
{
    Reflection data = GetData(context);
//    return data->Calculate(context);
    return data.GetValue();
}

bool FormulaIndexExpression::ApplyValue(FormulaContext* context, const Any& value) const
{
    Reflection ref = GetData(context);
    if (ref.GetValue() != value)
    {
        ref.SetValue(value);
        return true;
    }
    return false;
}

Reflection FormulaIndexExpression::GetData(FormulaContext* context) const
{
    if (exp)
    {
        Any indexVal = indexExp->Calculate(context);
        int32 index = -1;
        if (indexVal.CanCast<int32>())
        {
            index = indexVal.Cast<int32>();
        }
        else
        {
            Any indexVal = indexExp->Calculate(context);
            int32 index = -1;
            if (indexVal.CanCast<int32>())
            {
                index = indexVal.Cast<int32>();
            }
            throw FormulaCalculationError("Type of index expression must be int");
        }

        Reflection data = exp->GetData(context);

        if (data.IsValid())
        {
            return data.GetField(index);
        }
        else
        {
            DVASSERT(false);
            return Reflection();
        }
    }
    else
    {
        DVASSERT(false);
        return Reflection();
    }
}

void FormulaIndexExpression::Accept(FormulaExpressionVisitor* visitor)
{
    visitor->Visit(this);
}

void FormulaIndexExpression::CollectDepencencies(FormulaContext *context, Vector<void*> &dependencies) const
{
    if (exp)
    {
        indexExp->CollectDepencencies(context, dependencies);
        
        Any indexVal = indexExp->Calculate(context);
        int32 index = -1;
        if (indexVal.CanGet<int32>())
        {
            index = indexVal.Get<int32>();
        }
        
        Reflection data = exp->GetData(context);
        
        if (data.IsValid())
        {
            dependencies.push_back(data.GetValueObject().GetVoidPtr());
        }
        else
        {
            DVASSERT(false);
        }
    }
    else
    {
        DVASSERT(false);
    }
}

FormulaExpression* FormulaIndexExpression::GetExp() const
{
    return exp.get();
}

FormulaExpression* FormulaIndexExpression::GetIndexExp() const
{
    return indexExp.get();
}
}
