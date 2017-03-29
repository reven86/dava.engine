#pragma once

#include "Base/BaseTypes.h"
#include "UI/Formula/FormulaExpression.h"
#include "UI/Formula/FormulaContext.h"

namespace DAVA
{
class FormulaExecutor : private FormulaExpressionVisitor
{
public:
    FormulaExecutor(FormulaContext* context);
    ~FormulaExecutor() override;

    Any Execute(FormulaExpression* exp);

private:
    void Visit(FormulaValueExpression* exp) override;
    void Visit(FormulaNegExpression* exp) override;
    void Visit(FormulaNotExpression* exp) override;
    void Visit(FormulaBinaryOperatorExpression* exp) override;
    void Visit(FormulaFunctionExpression* exp) override;
    void Visit(FormulaFieldAccessExpression* exp) override;
    void Visit(FormulaIndexExpression* exp) override;

    const Any& Calculate(FormulaExpression* exp);
    const Reflection& GetData(FormulaExpression* exp);

    template <typename T>
    Any CalculateNumberValues(FormulaBinaryOperatorExpression::Operator op, T lVal, T rVal) const;

    FormulaContext* context = nullptr;
    Any res;
    Reflection data;
    Vector<void*> dependencies;
};
}
