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

Token::Token(Type type_, int lineNumber_, int positionInLine_)
    : type(type_)
    , lineNumber(lineNumber_)
    , positionInLine(positionInLine_)
{
    DVASSERT(type >= COMMA && type <= END);
}

Token::Token(Type type_, int val_, int lineNumber_, int positionInLine_)
    : type(type_)
    , iVal(val_)
    , lineNumber(lineNumber_)
    , positionInLine(positionInLine_)
{
    DVASSERT(type == INT);
}

Token::Token(Type type_, float val_, int lineNumber_, int positionInLine_)
    : type(type_)
    , fVal(val_)
    , lineNumber(lineNumber_)
    , positionInLine(positionInLine_)
{
    DVASSERT(type == FLOAT);
}

Token::Token(Type type_, bool val_, int lineNumber_, int positionInLine_)
    : type(type_)
    , iVal(val_ ? 1 : 0)
    , lineNumber(lineNumber_)
    , positionInLine(positionInLine_)
{
    DVASSERT(type == BOOLEAN);
}

Token::Token(Type type_, int startPos_, int len_, int lineNumber_, int positionInLine_)
    : type(type_)
    , start(startPos_)
    , len(len_)
    , lineNumber(lineNumber_)
    , positionInLine(positionInLine_)
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

int32 Token::GetLineNumber() const
{
    return lineNumber;
}

int32 Token::GetPositionInLine() const
{
    return positionInLine;
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

    int line = lineNumber;
    int column = positionInLine;
    switch (ch)
    {
    case '\0':
            ReadChar();
            return Token(Token::END, line, column);

    case ',':
            ReadChar();
            return Token(Token::COMMA, line, column);

    case '.':
            ReadChar();
            return Token(Token::DOT, line, column);

    case '<':
            ReadChar();
            if (ch == '=')
            {
                ReadChar();
                return Token(Token::LE, line, column);
            }
            return Token(Token::LT, line, column);

    case '>':
            ReadChar();
            if (ch == '=')
            {
                ReadChar();
                return Token(Token::GE, line, column);
            }
            return Token(Token::GT, line, column);

    case '+':
            ReadChar();
            return Token(Token::PLUS, line, column);

    case '*':
            ReadChar();
            return Token(Token::MUL, line, column);

    case '/':
        ReadChar();
        return Token(Token::DIV, line, column);

    case '%':
        ReadChar();
        return Token(Token::MOD, line, column);

    case '=':
        ReadChar();
        if (ch == '=')
        {
            ReadChar();
            return Token(Token::EQ, line, column);
        }
        return Token(Token::ASSIGN_SIGN, line, column);

    case '!':
        ReadChar();
        if (ch == '=')
        {
            ReadChar();
            return Token(Token::NOT_EQ, line, column);
        }
        return Token(Token::NOT, line, column);

    case '&':
        ReadChar();
        if (ch == '&')
        {
            ReadChar();
            return Token(Token::AND, line, column);
        }
        throw FormulaError("Can't resolve symbol '&'", line, column);

    case '|':
        ReadChar();
        if (ch == '|')
        {
            ReadChar();
            return Token(Token::OR, line, column);
        }
        throw FormulaError("Can't resolve symbol '|'", line, column);

    case '(':
        ReadChar();
        return Token(Token::OPEN_BRACKET, line, column);

    case ')':
        ReadChar();
        return Token(Token::CLOSE_BRACKET, line, column);

    case '{':
        ReadChar();
        return Token(Token::OPEN_CURLY_BRACKET, line, column);

    case '}':
        ReadChar();
        return Token(Token::CLOSE_CURLY_BRACKET, line, column);

    case '[':
        ReadChar();
        return Token(Token::OPEN_SQUARE_BRACKET, line, column);

    case ']':
        ReadChar();
        return Token(Token::CLOSE_SQUARE_BRACKET, line, column);

    case '"':
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

            return Token(Token::STRING, p, currentPosition - p - 1, line, column);
        }

        default:
            // do nothing
            break;
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
            return Token(Token::BOOLEAN, true, line, column);
        }
        else if (id == "false")
        {
            return Token(Token::BOOLEAN, false, line, column);
        }

        return Token(Token::IDENTIFIER, p, len, line, column);
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
                return Token(Token::MINUS, line, column);
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

            return Token(Token::FLOAT, negative ? -fl : fl, line, column);
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

        return Token(Token::INT, negative ? -(int32)num : (int32)num, line, column);
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
    if (currentPosition < str.size())
    {
        ch = str.at(currentPosition);
        positionInLine++;
    }
    else
    {
        ch = '\0';
        currentPosition = str.size();
    }
}
}
