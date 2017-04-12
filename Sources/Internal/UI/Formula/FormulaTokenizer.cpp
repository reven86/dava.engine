#include "FormulaTokenizer.h"
#include "Debug/DVAssert.h"
#include "Utils/StringFormat.h"

#include "UI/Formula/FormulaError.h"

namespace DAVA
{
FormulaToken::FormulaToken()
    : type(INVALID)
{
}

FormulaToken::FormulaToken(Type type_, int lineNumber_, int positionInLine_)
    : type(type_)
    , lineNumber(lineNumber_)
    , positionInLine(positionInLine_)
{
    DVASSERT(type >= COMMA && type <= END);
}

FormulaToken::FormulaToken(Type type_, int val_, int lineNumber_, int positionInLine_)
    : type(type_)
    , iVal(val_)
    , lineNumber(lineNumber_)
    , positionInLine(positionInLine_)
{
    DVASSERT(type == INT);
}

FormulaToken::FormulaToken(Type type_, float val_, int lineNumber_, int positionInLine_)
    : type(type_)
    , fVal(val_)
    , lineNumber(lineNumber_)
    , positionInLine(positionInLine_)
{
    DVASSERT(type == FLOAT);
}

FormulaToken::FormulaToken(Type type_, bool val_, int lineNumber_, int positionInLine_)
    : type(type_)
    , iVal(val_ ? 1 : 0)
    , lineNumber(lineNumber_)
    , positionInLine(positionInLine_)
{
    DVASSERT(type == BOOLEAN);
}

FormulaToken::FormulaToken(Type type_, int startPos_, int len_, int lineNumber_, int positionInLine_)
    : type(type_)
    , start(startPos_)
    , len(len_)
    , lineNumber(lineNumber_)
    , positionInLine(positionInLine_)
{
    DVASSERT(type == IDENTIFIER || type == STRING);
}

FormulaToken::Type FormulaToken::GetType() const
{
    return type;
}

int FormulaToken::GetInt() const
{
    DVASSERT(type == INT);
    return iVal;
}

float FormulaToken::GetFloat() const
{
    DVASSERT(type == FLOAT);
    return fVal;
}

bool FormulaToken::GetBool() const
{
    DVASSERT(type == BOOLEAN);
    return iVal != 0;
}

int FormulaToken::GetStringPos() const
{
    DVASSERT(type == IDENTIFIER || type == STRING);
    return start;
}

int FormulaToken::GetStringLen() const
{
    DVASSERT(type == IDENTIFIER || type == STRING);
    return len;
}

int32 FormulaToken::GetLineNumber() const
{
    return lineNumber;
}

int32 FormulaToken::GetPositionInLine() const
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

FormulaToken FormulaTokenizer::ReadToken()
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
            return FormulaToken(FormulaToken::END, line, column);

    case ',':
            ReadChar();
            return FormulaToken(FormulaToken::COMMA, line, column);

    case '.':
            ReadChar();
            return FormulaToken(FormulaToken::DOT, line, column);

    case '<':
            ReadChar();
            if (ch == '=')
            {
                ReadChar();
                return FormulaToken(FormulaToken::LE, line, column);
            }
            return FormulaToken(FormulaToken::LT, line, column);

    case '>':
            ReadChar();
            if (ch == '=')
            {
                ReadChar();
                return FormulaToken(FormulaToken::GE, line, column);
            }
            return FormulaToken(FormulaToken::GT, line, column);

    case '+':
            ReadChar();
            return FormulaToken(FormulaToken::PLUS, line, column);

    case '-':
        ReadChar();
        return FormulaToken(FormulaToken::MINUS, line, column);

    case '*':
            ReadChar();
            return FormulaToken(FormulaToken::MUL, line, column);

    case '/':
        ReadChar();
        return FormulaToken(FormulaToken::DIV, line, column);

    case '%':
        ReadChar();
        return FormulaToken(FormulaToken::MOD, line, column);

    case '=':
        ReadChar();
        if (ch == '=')
        {
            ReadChar();
            return FormulaToken(FormulaToken::EQ, line, column);
        }
        return FormulaToken(FormulaToken::ASSIGN_SIGN, line, column);

    case '!':
        ReadChar();
        if (ch == '=')
        {
            ReadChar();
            return FormulaToken(FormulaToken::NOT_EQ, line, column);
        }
        return FormulaToken(FormulaToken::NOT, line, column);

    case '&':
        ReadChar();
        if (ch == '&')
        {
            ReadChar();
            return FormulaToken(FormulaToken::AND, line, column);
        }
        DAVA_THROW(FormulaError, "Can't resolve symbol '&'", line, column);

    case '|':
        ReadChar();
        if (ch == '|')
        {
            ReadChar();
            return FormulaToken(FormulaToken::OR, line, column);
        }
        DAVA_THROW(FormulaError, "Can't resolve symbol '|'", line, column);

    case '(':
        ReadChar();
        return FormulaToken(FormulaToken::OPEN_BRACKET, line, column);

    case ')':
        ReadChar();
        return FormulaToken(FormulaToken::CLOSE_BRACKET, line, column);

    case '{':
        ReadChar();
        return FormulaToken(FormulaToken::OPEN_CURLY_BRACKET, line, column);

    case '}':
        ReadChar();
        return FormulaToken(FormulaToken::CLOSE_CURLY_BRACKET, line, column);

    case '[':
        ReadChar();
        return FormulaToken(FormulaToken::OPEN_SQUARE_BRACKET, line, column);

    case ']':
        ReadChar();
        return FormulaToken(FormulaToken::CLOSE_SQUARE_BRACKET, line, column);

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
                DAVA_THROW(FormulaError, "Illegal line end in string literal", lineNumber, positionInLine);
            }
            ReadChar();

            return FormulaToken(FormulaToken::STRING, p, currentPosition - p - 1, line, column);
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
            return FormulaToken(FormulaToken::BOOLEAN, true, line, column);
        }
        else if (id == "false")
        {
            return FormulaToken(FormulaToken::BOOLEAN, false, line, column);
        }

        return FormulaToken(FormulaToken::IDENTIFIER, p, len, line, column);
    }

    if (IsDigit(ch))
    {
        int64 num = 0; // optimize

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

            return FormulaToken(FormulaToken::FLOAT, fl, line, column);
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

        return FormulaToken(FormulaToken::INT, (int32)num, line, column);
    }
    DAVA_THROW(FormulaError, "Can't resolve symbol", lineNumber, positionInLine);
}

const String& FormulaTokenizer::GetString() const
{
    return str;
}

String FormulaTokenizer::GetTokenStringValue(const FormulaToken& FormulaToken)
{
    return str.substr(FormulaToken.GetStringPos(), FormulaToken.GetStringLen());
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
