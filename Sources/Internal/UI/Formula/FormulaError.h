#pragma once

#include "Base/BaseTypes.h"
#include "Base/Exception.h"
#include "UI/Formula/FormulaExpression.h"

namespace DAVA
{
/**
 \ingroup formula
 
 Exception class for parsing and execution of formulas with information about
 problem location.
 */
class FormulaError : public Exception
{
public:
    FormulaError(const String& message, int32 lineNumber, int32 positionInLine, const char* file, size_t line);
    FormulaError(const char* message, int32 lineNumber, int32 positionInLine, const char* file, size_t line);
    FormulaError(const String& message, const FormulaExpression* exp, const char* file, size_t line);
    FormulaError(const char* message, const FormulaExpression* exp, const char* file, size_t line);
    ~FormulaError();

    int32 GetLineNumber() const;
    int32 GetPositionInLine() const;
    String GetMessage() const;
    String GetFormattedMessage() const;

private:
    int32 lineNumber = -1;
    int32 positionInLine = -1;
};
}
