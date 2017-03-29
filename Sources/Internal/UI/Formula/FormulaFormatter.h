#pragma once

#include "UI/Formula/FormulaExpression.h"

namespace DAVA
{
class FormulaFormatter : private FormulaExpressionVisitor
{
public:
    FormulaFormatter();
    ~FormulaFormatter() override;

    String Format(FormulaExpression* exp);

private:
    void Visit(FormulaValueExpression* exp) override;
    void Visit(FormulaNegExpression* exp) override;
    void Visit(FormulaNotExpression* exp) override;
    void Visit(FormulaBinaryOperatorExpression* exp) override;
    void Visit(FormulaFunctionExpression* exp) override;
    void Visit(FormulaFieldAccessExpression* exp) override;
    void Visit(FormulaIndexExpression* exp) override;

    int GetExpPriority(FormulaExpression* exp) const;

    StringStream stream;
};
}
