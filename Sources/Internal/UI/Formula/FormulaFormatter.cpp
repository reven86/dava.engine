#include "FormulaFormatter.h"

#include "Utils/StringFormat.h"
#include "FileSystem/FilePath.h"

#include "Debug/DVAssert.h"

namespace DAVA
{
FormulaFormatter::FormulaFormatter()
{
}

FormulaFormatter::~FormulaFormatter()
{
}

String FormulaFormatter::Format(FormulaExpression* exp)
{
    exp->Accept(this);
    String result = stream.str();
    stream.clear();
    return result;
}

String FormulaFormatter::AnyToString(const Any& val)
{
    if (val.CanGet<int32>())
    {
        return Format("%d", val.Get<int32>());
    }
    else if (val.CanGet<uint64>())
    {
        return Format("%ld", val.Get<uint64>());
    }
    else if (val.CanGet<int64>())
    {
        return Format("%ldL", val.Get<int64>());
    }
    else if (val.CanGet<uint16>())
    {
        return Format("%d", val.Get<int16>());
    }
    else if (val.CanGet<int16>())
    {
        return Format("%d", val.Get<int16>());
    }
    else if (val.CanGet<uint8>())
    {
        return Format("%d", val.Get<uint8>());
    }
    else if (val.CanGet<int8>())
    {
        return Format("%d", val.Get<int8>());
    }
    else if (val.CanGet<float>())
    {
        return Format("%f", val.Get<float>());
    }
    else if (val.CanGet<String>())
    {
        return val.Get<String>();
    }
    else if (val.CanGet<FilePath>())
    {
        return val.Get<FilePath>().GetFrameworkPath();
    }
    else if (val.CanGet<bool>())
    {
        return val.Get<bool>() ? "true" : "false";
    }

    return String("");
}

void FormulaFormatter::Visit(FormulaValueExpression* exp)
{
    Any value = exp->GetValue();
    if (value.CanGet<String>())
    {
        stream << "\"" << value.Get<String>() << "\"";
    }
    else
    {
        stream << AnyToString(exp->GetValue());
    }
}

void FormulaFormatter::Visit(FormulaNegExpression* exp)
{
    stream << "-";

    bool squares = dynamic_cast<FormulaBinaryOperatorExpression*>(exp->GetExp()) != nullptr;
    if (squares)
    {
        stream << "(";
    }

    exp->GetExp()->Accept(this);

    if (squares)
    {
        stream << ")";
    }
}

void FormulaFormatter::Visit(FormulaNotExpression* exp)
{
    stream << "!";

    bool squares = dynamic_cast<FormulaBinaryOperatorExpression*>(exp->GetExp()) != nullptr;
    if (squares)
    {
        stream << "(";
    }

    exp->GetExp()->Accept(this);

    if (squares)
    {
        stream << ")";
    }
}

void FormulaFormatter::Visit(FormulaBinaryOperatorExpression* exp)
{
    bool lhsSq = GetExpPriority(exp->GetLhs()) > exp->GetOperatorPriority();
    bool rhsSq = GetExpPriority(exp->GetRhs()) > exp->GetOperatorPriority();

    if (lhsSq)
    {
        stream << "(";
    }
    exp->GetLhs()->Accept(this);
    if (lhsSq)
    {
        stream << ")";
    }

    switch (exp->GetOperator())
    {
    case FormulaBinaryOperatorExpression::OP_PLUS:
        stream << " + ";
        break;
    case FormulaBinaryOperatorExpression::OP_MINUS:
        stream << " - ";
        break;
    case FormulaBinaryOperatorExpression::OP_MUL:
        stream << " * ";
        break;
    case FormulaBinaryOperatorExpression::OP_DIV:
        stream << " / ";
        break;
    case FormulaBinaryOperatorExpression::OP_MOD:
        stream << " % ";
        break;
    case FormulaBinaryOperatorExpression::OP_AND:
        stream << " && ";
        break;
    case FormulaBinaryOperatorExpression::OP_OR:
        stream << " || ";
        break;
    case FormulaBinaryOperatorExpression::OP_EQ:
        stream << " == ";
        break;
    case FormulaBinaryOperatorExpression::OP_NOT_EQ:
        stream << " != ";
        break;
    case FormulaBinaryOperatorExpression::OP_LE:
        stream << " <= ";
        break;
    case FormulaBinaryOperatorExpression::OP_LT:
        stream << " < ";
        break;
    case FormulaBinaryOperatorExpression::OP_GE:
        stream << " >= ";
        break;
    case FormulaBinaryOperatorExpression::OP_GT:
        stream << " > ";
        break;

    default:
        DVASSERT("Invalid operator.");
        break;
    }

    if (rhsSq)
    {
        stream << "(";
    }
    exp->GetRhs()->Accept(this);
    if (rhsSq)
    {
        stream << ")";
    }
}

void FormulaFormatter::Visit(FormulaFunctionExpression* exp)
{
    stream << exp->GetName();
    stream << "(";

    bool first = true;
    for (const std::shared_ptr<FormulaExpression>& param : exp->GetParms())
    {
        if (first)
        {
            first = false;
        }
        else
        {
            stream << ", ";
        }
        param->Accept(this);
    }

    stream << ")";
}

void FormulaFormatter::Visit(FormulaFieldAccessExpression* exp)
{
    if (exp->GetExp())
    {
        exp->GetExp()->Accept(this);
        stream << ".";
        stream << exp->GetFieldName();
    }
    else
    {
        stream << exp->GetFieldName();
    }
}

void FormulaFormatter::Visit(FormulaIndexExpression* exp)
{
    exp->GetExp()->Accept(this);
    stream << "[";
    exp->GetIndexExp()->Accept(this);
    stream << "]";
}

int FormulaFormatter::GetExpPriority(FormulaExpression* exp) const
{
    FormulaBinaryOperatorExpression* binaryExp = dynamic_cast<FormulaBinaryOperatorExpression*>(exp);
    if (binaryExp != nullptr)
    {
        return binaryExp->GetOperatorPriority();
    }
    return 0;
}

}
