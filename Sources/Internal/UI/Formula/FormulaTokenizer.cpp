#include "FormulaTokenizer.h"
#include "Debug/DVAssert.h"
#include "Utils/StringFormat.h"

#include "UI/Formula/FormulaError.h"

namespace DAVA
{
Token::Token()
    : type(INVALID)
{
}

Token::Token(Type type_)
    : type(type_)
{
    DVASSERT(type >= COMMA && type <= END);
}

Token::Token(Type type_, int val_)
    : type(type_)
    , iVal(val_)
{
    DVASSERT(type == INT);
}

Token::Token(Type type_, float val_)
    : type(type_)
    , fVal(val_)
{
    DVASSERT(type == FLOAT);
}

Token::Token(Type type_, bool val_)
    : type(type_)
    , iVal(val_ ? 1 : 0)
{
    DVASSERT(type == BOOLEAN);
}

Token::Token(Type type_, int startPos_, int len_)
    : type(type_)
    , start(startPos_)
    , len(len_)
{
    DVASSERT(type == IDENTIFIER || type == STRING);
}

Token::Type Token::GetType() const
{
    return type;
}

int Token::GetInt() const
{
    DVASSERT(type == INT);
    return iVal;
}

float Token::GetFloat() const
{
    DVASSERT(type == FLOAT);
    return fVal;
}

bool Token::GetBool() const
{
    DVASSERT(type == BOOLEAN);
    return iVal != 0;
}

int Token::GetStringPos() const
{
    DVASSERT(type == IDENTIFIER || type == STRING);
    return start;
}

int Token::GetStringLen() const
{
    DVASSERT(type == IDENTIFIER || type == STRING);
    return len;
}

FormulaTokenizer::FormulaTokenizer(const String& str_)
    : str(str_)
{
}

FormulaTokenizer::~FormulaTokenizer()
{
}

Token FormulaTokenizer::ReadToken()
{
    if (currentPosition == -1)
    {
        lineNumber = 1;
        ReadChar();
    }
    SkipWhitespaces();

    if (ch == '\0')
    {
        ReadChar();
        return Token(Token::END);
    }

    if (ch == ',')
    {
        ReadChar();
        return Token(Token::COMMA);
    }

    if (ch == '.')
    {
        ReadChar();
        return Token(Token::DOT);
    }

    if (ch == '<')
    {
        ReadChar();
        if (ch == '=')
        {
            ReadChar();
            return Token(Token::LE);
        }
        return Token(Token::LT);
    }

    if (ch == '>')
    {
        ReadChar();
        if (ch == '=')
        {
            ReadChar();
            return Token(Token::GE);
        }
        return Token(Token::GT);
    }

    if (ch == '+')
    {
        ReadChar();
        return Token(Token::PLUS);
    }

    if (ch == '*')
    {
        ReadChar();
        return Token(Token::MUL);
    }

    if (ch == '/')
    {
        ReadChar();
        return Token(Token::DIV);
    }

    if (ch == '%')
    {
        ReadChar();
        return Token(Token::MOD);
    }
    
    if (ch == '=')
    {
        ReadChar();
        if (ch == '=')
        {
            ReadChar();
            return Token(Token::EQ);
        }
        return Token(Token::ASSIGN_SIGN);
    }

    if (ch == '!')
    {
        ReadChar();
        if (ch == '=')
        {
            ReadChar();
            return Token(Token::NOT_EQ);
        }
        return Token(Token::NOT);
    }
    
    if (ch == '&')
    {
        ReadChar();
        if (ch == '&')
        {
            ReadChar();
            return Token(Token::AND);
        }
        throw FormulaError("Can't resolve symbol '&'", lineNumber, positionInLine);
    }
    
    if (ch == '|')
    {
        ReadChar();
        if (ch == '|')
        {
            ReadChar();
            return Token(Token::OR);
        }
        throw FormulaError("Can't resolve symbol '|'", lineNumber, positionInLine);
    }
    
    if (ch == '(')
    {
        ReadChar();
        return Token(Token::OPEN_BRACKET);
    }

    if (ch == ')')
    {
        ReadChar();
        return Token(Token::CLOSE_BRACKET);
    }

    if (ch == '{')
    {
        ReadChar();
        return Token(Token::OPEN_CURLY_BRACKET);
    }

    if (ch == '}')
    {
        ReadChar();
        return Token(Token::CLOSE_CURLY_BRACKET);
    }

    if (ch == '[')
    {
        ReadChar();
        return Token(Token::OPEN_SQUARE_BRACKET);
    }

    if (ch == ']')
    {
        ReadChar();
        return Token(Token::CLOSE_SQUARE_BRACKET);
    }

    if (ch == '"')
    {
        ReadChar();

        int p = currentPosition;
        while (ch != '\0' && ch != '"')
        {
            ReadChar();
        }

        if (ch == '\0')
        {
            throw FormulaError("Illegal line end in string literal", lineNumber, positionInLine);
        }
        ReadChar();

        return Token(Token::STRING, p, currentPosition - p - 1);
    }

    if (IsIdentifierStart(ch))
    {
        int p = currentPosition;
        while (IsIdentifierPart(ch))
        {
            ReadChar();
        }

        int len = currentPosition - p;
        String id = str.substr(p, len);
        if (id == "true")
        {
            return Token(Token::BOOLEAN, true);
        }
        else if (id == "false")
        {
            return Token(Token::BOOLEAN, false);
        }

        return Token(Token::IDENTIFIER, p, len);
    }

    if (IsDigit(ch) || ch == '-')
    {
        bool negative = false;
        int64 num = 0; // optimize

        if (ch == '-')
        {
            negative = true;
            ReadChar();

            if (!IsDigit(ch))
            {
                return Token(Token::MINUS);
            }
        }

        while (IsDigit(ch))
        {
            num = num * 10 + (ch - '0');
            ReadChar();
        }

        if (ch == '.')
        {
            ReadChar();

            float n = 0.1f;
            float fl = (float)num;
            while (IsDigit(ch))
            {
                fl += (float)(ch - '0') * n;
                n *= 0.1f;

                ReadChar();
            }

            return Token(Token::FLOAT, negative ? -fl : fl);
        }
        else if (ch == 'L')
        {
            ReadChar();
        }
        else if (ch == 'U')
        {
            ReadChar();
            if (ch == 'L')
            {
                ReadChar();
            }
        }

        return Token(Token::INT, negative ? -(int32)num : (int32)num);
    }

    throw FormulaError("Can't resolve symbol", lineNumber, positionInLine);
}

const String& FormulaTokenizer::GetString() const
{
    return str;
}

String FormulaTokenizer::GetTokenStringValue(const Token& token)
{
    return str.substr(token.GetStringPos(), token.GetStringLen());
}

int32 FormulaTokenizer::GetLineNumber() const
{
    return lineNumber;
}

int32 FormulaTokenizer::GetPositionInLine() const
{
    return positionInLine;
}

void FormulaTokenizer::SkipWhitespaces()
{
    int prevCh = 0;
    while (ch == ' ' || ch == '\t' || ch == 10 || ch == 13)
    {
        positionInLine++;
        if (ch == 10 || ch == 13)
        {
            if (prevCh != 13 || ch != 10)
            {
                lineNumber++;
            }
            positionInLine = 0;
        }
        prevCh = ch;
        ReadChar();
    }
}

bool FormulaTokenizer::IsIdentifierStart(char ch)
{
    return ('a' <= ch && ch <= 'z') || ('A' <= ch && ch <= 'Z') || ch == '_';
}

bool FormulaTokenizer::IsIdentifierPart(char ch)
{
    return IsIdentifierStart(ch) || IsDigit(ch);
}

bool FormulaTokenizer::IsDigit(char ch)
{
    return '0' <= ch && ch <= '9';
}

void FormulaTokenizer::ReadChar()
{
    currentPosition++;
    positionInLine++;
    if (currentPosition < str.size())
    {
        ch = str.at(currentPosition);
    }
    else
    {
        ch = '\0';
        currentPosition = str.size();
    }
}
}
