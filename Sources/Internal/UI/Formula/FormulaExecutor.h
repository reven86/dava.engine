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

    Any Calculate(FormulaExpression* exp);
    Reflection GetDataReference(FormulaExpression* exp);

private:
    void Visit(FormulaValueExpression* exp) override;
    void Visit(FormulaNegExpression* exp) override;
    void Visit(FormulaNotExpression* exp) override;
    void Visit(FormulaBinaryOperatorExpression* exp) override;
    void Visit(FormulaFunctionExpression* exp) override;
    void Visit(FormulaFieldAccessExpression* exp) override;
    void Visit(FormulaIndexExpression* exp) override;

    const Any& CalculateImpl(FormulaExpression* exp);
    const Reflection& GetDataReferenceImpl(FormulaExpression* exp);

    template <typename T>
    Any CalculateNumberValues(FormulaBinaryOperatorExpression::Operator op, T lVal, T rVal) const;

    FormulaContext* context = nullptr;
    Any calculationResult;
    Reflection dataReference;
    Vector<void*> dependencies;
};
}
