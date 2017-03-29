#include "FormulaFormatter.h"

#include "UI/Formula/AnyConverter.h"

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

void FormulaFormatter::Visit(FormulaValueExpression* exp)
{
    Any value = exp->GetValue();
    if (value.CanGet<String>())
    {
        stream << "\"" << value.Get<String>() << "\"";
    }
    else
    {
        stream << AnyConverter::ToString(exp->GetValue());
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

    stream << " " << GetOperatorAsString(exp->GetOperator()) << " ";

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

String FormulaFormatter::GetOperatorAsString(FormulaBinaryOperatorExpression::Operator op) const
{
    switch (op)
    {
    case FormulaBinaryOperatorExpression::OP_PLUS:
        return "+";
    case FormulaBinaryOperatorExpression::OP_MINUS:
        return "-";
    case FormulaBinaryOperatorExpression::OP_MUL:
        return "*";
    case FormulaBinaryOperatorExpression::OP_DIV:
        return "/";
    case FormulaBinaryOperatorExpression::OP_AND:
        return "&&";
    case FormulaBinaryOperatorExpression::OP_OR:
        return "||";
    case FormulaBinaryOperatorExpression::OP_EQ:
        return "==";
    case FormulaBinaryOperatorExpression::OP_NOT_EQ:
        return "!=";
    case FormulaBinaryOperatorExpression::OP_LE:
        return "<=";
    case FormulaBinaryOperatorExpression::OP_LT:
        return "<";
    case FormulaBinaryOperatorExpression::OP_GE:
        return ">=";
    case FormulaBinaryOperatorExpression::OP_GT:
        return ">";

    default:
        DVASSERT("Invalid operator.");
        return "?";
    }
}
}
