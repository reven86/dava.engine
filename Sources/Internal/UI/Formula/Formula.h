#pragma once

#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class FormulaExpression;
class FormulaContext;

class Formula final
{
public:
    Formula();
    ~Formula();

    bool Parse(const String& str);
    bool IsValid() const;

    Any Calculate(FormulaContext* context);
    Any Calculate(const Reflection& ref);

    const String& GetParsingError() const;
    const String& GetCalculationError() const;

    String ToString() const;

private:
    std::shared_ptr<FormulaExpression> exp;

    String parsingError;
    String calculationError;
};
}
