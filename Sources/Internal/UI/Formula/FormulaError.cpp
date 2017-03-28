#include "FormulaError.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
FormulaError::FormulaError(const char *message, int32 lineNumber_, int32 positionInLine_)
    : std::runtime_error(message)
    , lineNumber(lineNumber_)
    , positionInLine(positionInLine_)
{
}

FormulaError::FormulaError(const String &message, int32 lineNumber_, int32 positionInLine_)
    : std::runtime_error(message)
    , lineNumber(lineNumber_)
    , positionInLine(positionInLine_)
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
    
FormulaCalculationError::FormulaCalculationError(const char *message) : std::runtime_error(message)
{
}

FormulaCalculationError::FormulaCalculationError(const String &message) : std::runtime_error(message)
{
}

FormulaCalculationError::~FormulaCalculationError()
{
    
}

String FormulaCalculationError::GetMessage() const
{
    return what();
}

}

