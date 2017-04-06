#pragma once

#include "Base/BaseTypes.h"

#include "UI/Formula/FormulaExpression.h"
#include "UI/Formula/FormulaData.h"
#include "UI/Formula/FormulaTokenizer.h"
#include "UI/Formula/FormulaError.h"

namespace DAVA
{
class FormulaParser
{
public:
    FormulaParser(const String& str);
    ~FormulaParser();

    std::shared_ptr<FormulaExpression> ParseExpression();
    std::shared_ptr<FormulaDataMap> ParseMap();

private:
    std::shared_ptr<FormulaDataVector> ParseVector();
    std::shared_ptr<FormulaExpression> ParseBinaryOp(int priority);
    std::shared_ptr<FormulaExpression> ParseUnary();
    std::shared_ptr<FormulaExpression> ParseRef();
    std::shared_ptr<FormulaExpression> ParseFunction(const String& identifier);
    std::shared_ptr<FormulaExpression> ParseValue();

    FormulaToken LookToken();
    FormulaToken NextToken();
    bool IsIdentifier(const FormulaToken& token, const String& identifier);
    String GetTokenStringValue(const FormulaToken& token);

    FormulaToken token;
    struct OperatorPriority
    {
        OperatorPriority(FormulaBinaryOperatorExpression::Operator op_, int priority_)
            : op(op_)
            , priority(priority_)
        {
        }

        FormulaBinaryOperatorExpression::Operator op;
        int priority;
    };
    UnorderedMap<FormulaToken::Type, OperatorPriority> operators;

    FormulaTokenizer tokenizer;
};
}
