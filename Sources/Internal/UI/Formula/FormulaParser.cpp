#include "FormulaParser.h"

#include "Logger/Logger.h"
#include "Debug/DVAssert.h"

namespace DAVA
{
using std::shared_ptr;
using std::make_shared;

FormulaParser::FormulaParser(const String& str)
    : tokenizer(str)
{
    operators.insert({ FormulaToken::MUL, OperatorPriority(FormulaBinaryOperatorExpression::OP_MUL, 1) });
    operators.insert({ FormulaToken::DIV, OperatorPriority(FormulaBinaryOperatorExpression::OP_DIV, 1) });
    operators.insert({ FormulaToken::MOD, OperatorPriority(FormulaBinaryOperatorExpression::OP_MOD, 1) });

    operators.insert({ FormulaToken::PLUS, OperatorPriority(FormulaBinaryOperatorExpression::OP_PLUS, 2) });
    operators.insert({ FormulaToken::MINUS, OperatorPriority(FormulaBinaryOperatorExpression::OP_MINUS, 2) });

    operators.insert({ FormulaToken::LT, OperatorPriority(FormulaBinaryOperatorExpression::OP_LT, 3) });
    operators.insert({ FormulaToken::LE, OperatorPriority(FormulaBinaryOperatorExpression::OP_LE, 3) });
    operators.insert({ FormulaToken::GT, OperatorPriority(FormulaBinaryOperatorExpression::OP_GT, 3) });
    operators.insert({ FormulaToken::GE, OperatorPriority(FormulaBinaryOperatorExpression::OP_GE, 3) });

    operators.insert({ FormulaToken::EQ, OperatorPriority(FormulaBinaryOperatorExpression::OP_EQ, 4) });
    operators.insert({ FormulaToken::NOT_EQ, OperatorPriority(FormulaBinaryOperatorExpression::OP_NOT_EQ, 4) });

    operators.insert({ FormulaToken::AND, OperatorPriority(FormulaBinaryOperatorExpression::OP_AND, 5) });

    operators.insert({ FormulaToken::OR, OperatorPriority(FormulaBinaryOperatorExpression::OP_OR, 6) });
}

FormulaParser::~FormulaParser()
{
}

shared_ptr<FormulaExpression> FormulaParser::ParseExpression()
{
    FormulaToken token = LookToken();
    int32 lineNumber = token.GetLineNumber();
    int32 posInLine = token.GetPositionInLine();

    if (token.GetType() == FormulaToken::END)
    {
        return shared_ptr<FormulaExpression>(); // empty exp
    }
    else if (token.GetType() == FormulaToken::OPEN_CURLY_BRACKET)
    {
        NextToken();
        shared_ptr<FormulaDataMap> data = ParseMap();
        token = LookToken();
        if (token.GetType() != FormulaToken::CLOSE_CURLY_BRACKET)
        {
            DAVA_THROW(FormulaError, "'}' expected", token.GetLineNumber(), token.GetPositionInLine());
        }
        NextToken(); // close bracket
        return make_shared<FormulaValueExpression>(data, lineNumber, posInLine);
    }
    else if (token.GetType() == FormulaToken::OPEN_SQUARE_BRACKET)
    {
        NextToken();
        shared_ptr<FormulaDataVector> data = ParseVector();
        token = LookToken();
        if (token.GetType() != FormulaToken::CLOSE_SQUARE_BRACKET)
        {
            DAVA_THROW(FormulaError, "']' expected", token.GetLineNumber(), token.GetPositionInLine());
        }
        NextToken(); // close bracket
        return make_shared<FormulaValueExpression>(data, lineNumber, posInLine);
    }
    else
    {
        return ParseBinaryOp(6);
    }
}

shared_ptr<FormulaDataMap> FormulaParser::ParseMap()
{
    shared_ptr<FormulaDataMap> map = make_shared<FormulaDataMap>();

    FormulaToken token = LookToken();
    while (token.GetType() == FormulaToken::IDENTIFIER)
    {
        token = NextToken();
        String name = GetTokenStringValue(token);

        token = NextToken();
        if (token.GetType() != FormulaToken::ASSIGN_SIGN)
        {
            DAVA_THROW(FormulaError, "'=' expected", token.GetLineNumber(), token.GetPositionInLine());
        }

        token = LookToken();
        shared_ptr<FormulaExpression> exp = ParseExpression();
        if (!exp)
        {
            DAVA_THROW(FormulaError, "Map value expected", token.GetLineNumber(), token.GetPositionInLine());
        }

        if (exp->IsValue())
        {
            FormulaValueExpression* value = static_cast<FormulaValueExpression*>(exp.get());
            map->Add(name, value->GetValue());
        }
        else
        {
            map->Add(name, Any(exp));
        }


        token = LookToken();
    }

    return map;
}

std::shared_ptr<FormulaDataVector> FormulaParser::ParseVector()
{
    shared_ptr<FormulaDataVector> vector = make_shared<FormulaDataVector>();

    int index = 0;
    FormulaToken token = LookToken();
    while (token.GetType() != FormulaToken::CLOSE_SQUARE_BRACKET)
    {
        shared_ptr<FormulaExpression> exp = ParseExpression();
        if (!exp)
        {
            DAVA_THROW(FormulaError, "Vector value expected", token.GetLineNumber(), token.GetPositionInLine());
        }

        if (exp->IsValue())
        {
            FormulaValueExpression* value = static_cast<FormulaValueExpression*>(exp.get());
            vector->Add(value->GetValue());
        }
        else
        {
            vector->Add(Any(exp));
        }
        
        token = LookToken();
        index++;
    }

    return vector;
}

shared_ptr<FormulaExpression> FormulaParser::ParseBinaryOp(int priority)
{
    if (priority == 0)
    {
        return ParseUnary();
    }

    shared_ptr<FormulaExpression> exp1 = ParseBinaryOp(priority - 1);
    FormulaToken token = LookToken();
    UnorderedMap<FormulaToken::Type, OperatorPriority>::iterator it;
    while ((it = operators.find(token.GetType())) != operators.end() && it->second.priority == priority)
    {
        NextToken();
        shared_ptr<FormulaExpression> exp2 = ParseBinaryOp(priority - 1);
        exp1 = make_shared<FormulaBinaryOperatorExpression>(it->second.op, exp1, exp2, token.GetLineNumber(), token.GetPositionInLine());
        token = LookToken();
    }
    return exp1;
}

shared_ptr<FormulaExpression> FormulaParser::ParseUnary()
{
    FormulaToken token = LookToken();
    if (token.GetType() == FormulaToken::IDENTIFIER)
    {
        return ParseRef();
    }
    else if (token.GetType() == FormulaToken::OPEN_BRACKET)
    {
        NextToken();
        shared_ptr<FormulaExpression> exp = ParseExpression();

        token = LookToken();
        if (token.GetType() != FormulaToken::CLOSE_BRACKET)
        {
            DAVA_THROW(FormulaError, "')' expected", token.GetLineNumber(), token.GetPositionInLine());
        }
        NextToken(); // close bracket

        return exp;
    }
    else if (token.GetType() == FormulaToken::NOT)
    {
        NextToken();
        token = LookToken();
        shared_ptr<FormulaExpression> exp = ParseExpression();
        if (!exp)
        {
            DAVA_THROW(FormulaError, "Expression expected", token.GetLineNumber(), token.GetPositionInLine());
        }

        return make_shared<FormulaNotExpression>(exp, token.GetLineNumber(), token.GetPositionInLine());
    }
    else if (token.GetType() == FormulaToken::MINUS)
    {
        NextToken();
        token = LookToken();
        shared_ptr<FormulaExpression> exp = ParseExpression();
        if (!exp)
        {
            DAVA_THROW(FormulaError, "Expression expected", token.GetLineNumber(), token.GetPositionInLine());
        }

        return make_shared<FormulaNegExpression>(exp, token.GetLineNumber(), token.GetPositionInLine());
    }
    else
    {
        return ParseValue();
    }
}

std::shared_ptr<FormulaExpression> FormulaParser::ParseRef()
{
    FormulaToken token = LookToken();

    if (token.GetType() != FormulaToken::IDENTIFIER)
    {
        DAVA_THROW(FormulaError, "Expected identifier.", token.GetLineNumber(), token.GetPositionInLine());
    }
    NextToken();

    String identifier = GetTokenStringValue(token);

    token = LookToken();

    shared_ptr<FormulaExpression> exp;
    if (token.GetType() == FormulaToken::OPEN_BRACKET)
    {
        exp = ParseFunction(identifier);
    }
    else
    {
        exp = make_shared<FormulaFieldAccessExpression>(nullptr, identifier, token.GetLineNumber(), token.GetPositionInLine());
    }

    token = LookToken();

    while (token.GetType() == FormulaToken::DOT || token.GetType() == FormulaToken::OPEN_SQUARE_BRACKET)
    {
        if (token.GetType() == FormulaToken::OPEN_SQUARE_BRACKET)
        {
            NextToken(); // [
            token = LookToken();
            shared_ptr<FormulaExpression> indexExp = ParseExpression();
            if (!indexExp)
            {
                DAVA_THROW(FormulaError, "Index expression expected", token.GetLineNumber(), token.GetPositionInLine());
            }
            exp = make_shared<FormulaIndexExpression>(exp, indexExp, token.GetLineNumber(), token.GetPositionInLine());

            token = NextToken();
            if (token.GetType() != FormulaToken::CLOSE_SQUARE_BRACKET)
            {
                DAVA_THROW(FormulaError, "']' expected", token.GetLineNumber(), token.GetPositionInLine());
            }
        }
        else
        {
            NextToken(); // DOT
            token = NextToken();
            if (token.GetType() == FormulaToken::IDENTIFIER)
            {
                String identifier = GetTokenStringValue(token);
                token = LookToken();
                exp = make_shared<FormulaFieldAccessExpression>(exp, identifier, token.GetLineNumber(), token.GetPositionInLine());
            }
            else
            {
                DAVA_THROW(FormulaError, "Expected identifier.", token.GetLineNumber(), token.GetPositionInLine());
            }
        }

        token = LookToken();
    }

    return exp;
}

shared_ptr<FormulaExpression> FormulaParser::ParseFunction(const String& identifier)
{
    FormulaToken token = LookToken();

    if (token.GetType() != FormulaToken::OPEN_BRACKET)
    {
        DAVA_THROW(FormulaError, "'(' expected", token.GetLineNumber(), token.GetPositionInLine());
    }
    
    NextToken(); // skip open bracket
    
    token = LookToken();
    Vector<shared_ptr<FormulaExpression>> params;

    if (token.GetType() == FormulaToken::CLOSE_BRACKET)
    {
        NextToken(); // skip close token
    }
    else
    {
        while (true)
        {
            token = LookToken();
            shared_ptr<FormulaExpression> exp = ParseExpression();
            if (!exp)
            {
                DAVA_THROW(FormulaError, "Function param expected", token.GetLineNumber(), token.GetPositionInLine());
            }
            
            params.push_back(exp);
            
            token = LookToken();
            if (token.GetType() == FormulaToken::COMMA)
            {
                NextToken(); // skip comma and continue
            }
            else if (token.GetType() == FormulaToken::CLOSE_BRACKET)
            {
                NextToken(); // finish function
                break;
            }
            else
            {
                DAVA_THROW(FormulaError, "expected ')'", token.GetLineNumber(), token.GetPositionInLine());
            }
        }
    }

    return make_shared<FormulaFunctionExpression>(identifier, params, token.GetLineNumber(), token.GetPositionInLine());
}

shared_ptr<FormulaExpression> FormulaParser::ParseValue()
{
    FormulaToken token = NextToken();

    switch (token.GetType())
    {
    case FormulaToken::INT:
        return make_shared<FormulaValueExpression>(Any(token.GetInt()));

    case FormulaToken::BOOLEAN:
        return make_shared<FormulaValueExpression>(Any(token.GetBool()));

    case FormulaToken::FLOAT:
        return make_shared<FormulaValueExpression>(Any(token.GetFloat()));

    case FormulaToken::STRING:
        return make_shared<FormulaValueExpression>(Any(GetTokenStringValue(token)));

    default:
        break;
    }

    DAVA_THROW(FormulaError, "Expected literal", token.GetLineNumber(), token.GetPositionInLine());
}

FormulaToken FormulaParser::LookToken()
{
    if (token.GetType() == FormulaToken::INVALID)
    {
        token = tokenizer.ReadToken();
    }
    return token;
}

FormulaToken FormulaParser::NextToken()
{
    if (token.GetType() != FormulaToken::INVALID)
    {
        FormulaToken result = token;
        token = FormulaToken();
        return result;
    }
    else
    {
        return tokenizer.ReadToken();
    }
}

bool FormulaParser::IsIdentifier(const FormulaToken& token, const String& identifier)
{
    return token.GetType() == FormulaToken::IDENTIFIER && GetTokenStringValue(token) == identifier;
}

String FormulaParser::GetTokenStringValue(const FormulaToken& token)
{
    return tokenizer.GetTokenStringValue(token);
}

}
