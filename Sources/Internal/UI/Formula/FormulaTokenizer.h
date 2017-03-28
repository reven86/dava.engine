#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class Token
{
public:
    enum Type
    {
        INVALID,
        INT,
        FLOAT,
        BOOLEAN,
        STRING,
        IDENTIFIER,
        COMMA,
        DOT,
        PLUS,
        MINUS,
        MUL,
        DIV,
        MOD,
        AND,
        OR,
        NOT,
        LE,
        LT,
        GE,
        GT,
        EQ,
        NOT_EQ,
        ASSIGN_SIGN, // =
        OPEN_BRACKET, // (
        CLOSE_BRACKET, // )
        OPEN_CURLY_BRACKET, // {
        CLOSE_CURLY_BRACKET, // }
        OPEN_SQUARE_BRACKET, // [
        CLOSE_SQUARE_BRACKET, // ]
        END,
    };

    Token();
    Token(Type type_);
    Token(Type type, int val);
    Token(Type type, float val);
    Token(Type type, bool val);
    Token(Type type, int startPos, int len);

    Type GetType() const;
    int GetInt() const;
    float GetFloat() const;
    bool GetBool() const;
    int GetStringPos() const;
    int GetStringLen() const;

private:
    Type type;

    union {
        struct
        {
            int start = 0;
            int len = 0;
        };

        int32 iVal;
        float32 fVal;
    };
};

class FormulaTokenizer
{
public:
    FormulaTokenizer(const String& str);
    ~FormulaTokenizer();

    Token ReadToken();
    const String& GetString() const;
    int32 GetLineNumber() const;
    int32 GetPositionInLine() const;

private:
    void SkipWhitespaces();
    bool IsIdentifierStart(char ch);
    bool IsIdentifierPart(char ch);
    bool IsDigit(char ch);
    void ReadChar();

    String str;
    char ch = '\0';
    int currentPosition = -1;
    int lineNumber = 0;
    int positionInLine = 0;
};
}
