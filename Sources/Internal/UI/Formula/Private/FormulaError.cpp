#include "FormulaError.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
FormulaError::FormulaError(const String& message, int32 lineNumber_, int32 positionInLine_, const char* file, size_t line)
    : Exception(message, file, line)
    , lineNumber(lineNumber_)
    , positionInLine(positionInLine_)
{
}

FormulaError::FormulaError(const char* message, int32 lineNumber_, int32 positionInLine_, const char* file, size_t line)
    : Exception(message, file, line)
    , lineNumber(lineNumber_)
    , positionInLine(positionInLine_)
{
}

FormulaError::FormulaError(const String& message, const FormulaExpression* exp, const char* file, size_t line)
    : Exception(message, file, line)
    , lineNumber(exp->GetLineNumber())
    , positionInLine(exp->GetPositionInLine())
{
}

FormulaError::FormulaError(const char* message, const FormulaExpression* exp, const char* file, size_t line)
    : Exception(message, file, line)
    , lineNumber(exp->GetLineNumber())
    , positionInLine(exp->GetPositionInLine())
{
}

FormulaError::~FormulaError()
{
}

int32 FormulaError::GetLineNumber() const
{
    return lineNumber;
}

int32 FormulaError::GetPositionInLine() const
{
    return positionInLine;
}

String FormulaError::GetMessage() const
{
    return what();
}

String FormulaError::GetFormattedMessage() const
{
    return Format("[%d, %d] %s", lineNumber, positionInLine, what());
}
}
