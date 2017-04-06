#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class FormulaToken
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

    FormulaToken();
    FormulaToken(Type type_, int lineNumber, int positionInLine);
    FormulaToken(Type type, int val, int lineNumber, int positionInLine);
    FormulaToken(Type type, float val, int lineNumber, int positionInLine);
    FormulaToken(Type type, bool val, int lineNumber, int positionInLine);
    FormulaToken(Type type, int startPos, int len, int lineNumber, int positionInLine);

    Type GetType() const;
    int GetInt() const;
    float GetFloat() const;
    bool GetBool() const;
    int GetStringPos() const;
    int GetStringLen() const;

    int32 GetLineNumber() const;
    int32 GetPositionInLine() const;

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

    int lineNumber = 0;
    int positionInLine = 0;
};

class FormulaTokenizer
{
public:
    FormulaTokenizer(const String& str);
    ~FormulaTokenizer();

    FormulaToken ReadToken();
    const String& GetString() const;
    String GetTokenStringValue(const FormulaToken& token);

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
