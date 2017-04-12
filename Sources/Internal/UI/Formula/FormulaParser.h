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
    std::shared_ptr<FormulaExpression> ParseLogicalOr();
    std::shared_ptr<FormulaExpression> ParseLogicalAnd();
    std::shared_ptr<FormulaExpression> ParseEquality();
    std::shared_ptr<FormulaExpression> ParseRelation();
    std::shared_ptr<FormulaExpression> ParseAdditive();
    std::shared_ptr<FormulaExpression> ParseMultiplicative();
    std::shared_ptr<FormulaExpression> ParseUnary();
    std::shared_ptr<FormulaExpression> ParsePostfix();
    std::shared_ptr<FormulaExpression> ParsePrimary();
    std::shared_ptr<FormulaExpression> ParseFunction(const String& identifier);
    std::shared_ptr<FormulaExpression> ParseAccess(const String& identifier);

    FormulaToken LookToken();
    FormulaToken NextToken();
    bool IsIdentifier(const FormulaToken& token, const String& identifier);
    String GetTokenStringValue(const FormulaToken& token);
    FormulaBinaryOperatorExpression::Operator TokenTypeToBinaryOp(FormulaToken::Type type);

    FormulaToken token;
    FormulaTokenizer tokenizer;
};
}
