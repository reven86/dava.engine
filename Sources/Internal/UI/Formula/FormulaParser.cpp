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
    operators.insert({ Token::MUL, OperatorPriority(FormulaBinaryOperatorExpression::OP_MUL, 1) });
    operators.insert({ Token::DIV, OperatorPriority(FormulaBinaryOperatorExpression::OP_DIV, 1) });
    operators.insert({ Token::MOD, OperatorPriority(FormulaBinaryOperatorExpression::OP_MOD, 1) });

    operators.insert({ Token::PLUS, OperatorPriority(FormulaBinaryOperatorExpression::OP_PLUS, 2) });
    operators.insert({ Token::MINUS, OperatorPriority(FormulaBinaryOperatorExpression::OP_MINUS, 2) });

    operators.insert({ Token::LT, OperatorPriority(FormulaBinaryOperatorExpression::OP_LT, 3) });
    operators.insert({ Token::LE, OperatorPriority(FormulaBinaryOperatorExpression::OP_LE, 3) });
    operators.insert({ Token::GT, OperatorPriority(FormulaBinaryOperatorExpression::OP_GT, 3) });
    operators.insert({ Token::GE, OperatorPriority(FormulaBinaryOperatorExpression::OP_GE, 3) });

    operators.insert({ Token::EQ, OperatorPriority(FormulaBinaryOperatorExpression::OP_EQ, 4) });
    operators.insert({ Token::NOT_EQ, OperatorPriority(FormulaBinaryOperatorExpression::OP_NOT_EQ, 4) });

    operators.insert({ Token::AND, OperatorPriority(FormulaBinaryOperatorExpression::OP_AND, 5) });

    operators.insert({ Token::OR, OperatorPriority(FormulaBinaryOperatorExpression::OP_OR, 6) });
}

FormulaParser::~FormulaParser()
{
}

shared_ptr<FormulaExpression> FormulaParser::ParseExpression()
{
    Token token = LookToken();
    if (token.GetType() == Token::END)
    {
        return shared_ptr<FormulaExpression>(); // empty exp
    }
    else if (token.GetType() == Token::OPEN_CURLY_BRACKET)
    {
        NextToken();
        shared_ptr<FormulaDataMap> data = ParseMap();
        token = LookToken();
        if (token.GetType() != Token::CLOSE_CURLY_BRACKET)
        {
            throw FormulaError("'}' expected", token.GetLineNumber(), token.GetPositionInLine());
        }
        NextToken(); // close bracket
        return make_shared<FormulaValueExpression>(data);
    }
    else if (token.GetType() == Token::OPEN_SQUARE_BRACKET)
    {
        NextToken();
        shared_ptr<FormulaDataVector> data = ParseVector();
        token = LookToken();
        if (token.GetType() != Token::CLOSE_SQUARE_BRACKET)
        {
            throw FormulaError("']' expected", token.GetLineNumber(), token.GetPositionInLine());
        }
        NextToken(); // close bracket
        return make_shared<FormulaValueExpression>(data);
    }
    else
    {
        return ParseBinaryOp(6);
    }
}

shared_ptr<FormulaDataMap> FormulaParser::ParseMap()
{
    shared_ptr<FormulaDataMap> map = make_shared<FormulaDataMap>();

    Token token = LookToken();
    while (token.GetType() == Token::IDENTIFIER)
    {
        token = NextToken();
        String name = GetTokenStringValue(token);

        token = NextToken();
        if (token.GetType() != Token::ASSIGN_SIGN)
        {
            throw FormulaError("'=' expected", token.GetLineNumber(), token.GetPositionInLine());
        }

        token = LookToken();
        shared_ptr<FormulaExpression> exp = ParseExpression();
        if (!exp)
        {
            throw FormulaError("Map value expected", token.GetLineNumber(), token.GetPositionInLine());
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
    Token token = LookToken();
    while (token.GetType() != Token::CLOSE_SQUARE_BRACKET)
    {
        shared_ptr<FormulaExpression> exp = ParseExpression();
        if (!exp)
        {
            throw FormulaError("Vector value expected", token.GetLineNumber(), token.GetPositionInLine());
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
    Token token = LookToken();
    UnorderedMap<Token::Type, OperatorPriority>::iterator it;
    while ((it = operators.find(token.GetType())) != operators.end() && it->second.priority == priority)
    {
        NextToken();
        shared_ptr<FormulaExpression> exp2 = ParseBinaryOp(priority - 1);
        exp1 = make_shared<FormulaBinaryOperatorExpression>(it->second.op, exp1, exp2);
        token = LookToken();
    }
    return exp1;
}

shared_ptr<FormulaExpression> FormulaParser::ParseUnary()
{
    Token token = LookToken();
    if (token.GetType() == Token::IDENTIFIER)
    {
        return ParseRef();
    }
    else if (token.GetType() == Token::OPEN_BRACKET)
    {
        NextToken();
        shared_ptr<FormulaExpression> exp = ParseExpression();

        token = LookToken();
        if (token.GetType() != Token::CLOSE_BRACKET)
        {
            throw FormulaError("')' expected", token.GetLineNumber(), token.GetPositionInLine());
        }
        NextToken(); // close bracket

        return exp;
    }
    else if (token.GetType() == Token::NOT)
    {
        NextToken();
        token = LookToken();
        shared_ptr<FormulaExpression> exp = ParseExpression();
        if (!exp)
        {
            throw FormulaError("Expression expected", token.GetLineNumber(), token.GetPositionInLine());
        }

        return make_shared<FormulaNotExpression>(exp);
    }
    else if (token.GetType() == Token::MINUS)
    {
        NextToken();
        token = LookToken();
        shared_ptr<FormulaExpression> exp = ParseExpression();
        if (!exp)
        {
            throw FormulaError("Expression expected", token.GetLineNumber(), token.GetPositionInLine());
        }

        return make_shared<FormulaNegExpression>(exp);
    }
    else
    {
        return ParseValue();
    }
}

std::shared_ptr<FormulaExpression> FormulaParser::ParseRef()
{
    Token token = LookToken();

    if (token.GetType() != Token::IDENTIFIER)
    {
        throw FormulaError("Expected identifier.", token.GetLineNumber(), token.GetPositionInLine());
    }
    NextToken();

    String identifier = GetTokenStringValue(token);

    token = LookToken();

    shared_ptr<FormulaExpression> exp;
    if (token.GetType() == Token::OPEN_BRACKET)
    {
        exp = ParseFunction(identifier);
    }
    else
    {
        exp = make_shared<FormulaFieldAccessExpression>(nullptr, identifier); // TODO: make spec expression
    }

    token = LookToken();

    while (token.GetType() == Token::DOT || token.GetType() == Token::OPEN_SQUARE_BRACKET)
    {
        if (token.GetType() == Token::OPEN_SQUARE_BRACKET)
        {
            NextToken(); // [
            token = LookToken();
            shared_ptr<FormulaExpression> indexExp = ParseExpression();
            if (!indexExp)
            {
                throw FormulaError("Index expression expected", token.GetLineNumber(), token.GetPositionInLine());
            }
            exp = make_shared<FormulaIndexExpression>(exp, indexExp);

            token = NextToken();
            if (token.GetType() != Token::CLOSE_SQUARE_BRACKET)
            {
                throw FormulaError("']' expected", token.GetLineNumber(), token.GetPositionInLine());
            }
        }
        else
        {
            NextToken(); // DOT
            token = NextToken();
            if (token.GetType() == Token::IDENTIFIER)
            {
                String identifier = GetTokenStringValue(token);
                token = LookToken();
                exp = make_shared<FormulaFieldAccessExpression>(exp, identifier);
            }
            else
            {
                throw FormulaError("Expected identifier.", token.GetLineNumber(), token.GetPositionInLine());
            }
        }

        token = LookToken();
    }

    return exp;
}

shared_ptr<FormulaExpression> FormulaParser::ParseFunction(const String& identifier)
{
    Token token = LookToken();

    if (token.GetType() != Token::OPEN_BRACKET)
    {
        throw FormulaError("'(' expected", token.GetLineNumber(), token.GetPositionInLine());
    }
    
    NextToken(); // skip open bracket
    
    token = LookToken();
    Vector<shared_ptr<FormulaExpression>> params;
    
    if (token.GetType() == Token::CLOSE_BRACKET)
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
                throw FormulaError("Function param expected", token.GetLineNumber(), token.GetPositionInLine());
            }
            
            params.push_back(exp);
            
            token = LookToken();
            if (token.GetType() == Token::COMMA)
            {
                NextToken(); // skip comma and continue
            }
            else if (token.GetType() == Token::CLOSE_BRACKET)
            {
                NextToken(); // finish function
                break;
            }
            else
            {
                throw FormulaError("expected ')'", token.GetLineNumber(), token.GetPositionInLine());
            }
        }
    }
    
    return make_shared<FormulaFunctionExpression>(identifier, params);
}

shared_ptr<FormulaExpression> FormulaParser::ParseValue()
{
    Token token = NextToken();
    
    switch (token.GetType())
    {
    case Token::INT:
        return make_shared<FormulaValueExpression>(Any(token.GetInt()));

    case Token::BOOLEAN:
        return make_shared<FormulaValueExpression>(Any(token.GetBool()));

    case Token::FLOAT:
        return make_shared<FormulaValueExpression>(Any(token.GetFloat()));

    case Token::STRING:
        return make_shared<FormulaValueExpression>(Any(GetTokenStringValue(token)));

    default:
        break;
    }

    throw FormulaError("Expected literal", token.GetLineNumber(), token.GetPositionInLine());
}

Token FormulaParser::LookToken()
{
    if (token.GetType() == Token::INVALID)
    {
        token = tokenizer.ReadToken();
    }
    return token;
}

Token FormulaParser::NextToken()
{
    if (token.GetType() != Token::INVALID)
    {
        Token result = token;
        token = Token();
        return result;
    }
    else
    {
        return tokenizer.ReadToken();
    }
}

bool FormulaParser::IsIdentifier(const Token& token, const String& identifier)
{
    return token.GetType() == Token::IDENTIFIER && GetTokenStringValue(token) == identifier;
}

String FormulaParser::GetTokenStringValue(const Token& token)
{
    return tokenizer.GetTokenStringValue(token);
}

}
