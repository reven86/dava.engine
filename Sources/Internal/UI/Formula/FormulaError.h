#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class FormulaError: public std::runtime_error
{
public:
    FormulaError(const char *message, int32 lineNumber, int32 positionInLine);
    FormulaError(const String &message, int32 lineNumber, int32 positionInLine);
    ~FormulaError();

    int32 GetLineNumber() const;
    int32 GetPositionInLine() const;
    String GetMessage() const;
    String GetFormattedMessage() const;

private:
    int32 lineNumber = -1;
    int32 positionInLine = -1;
};
  
    
class FormulaCalculationError : public std::runtime_error
{
public:
    FormulaCalculationError(const char *message);
    FormulaCalculationError(const String &message);
    ~FormulaCalculationError();
    
    String GetMessage() const;
};
}
