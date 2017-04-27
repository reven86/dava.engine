#include "LayoutFormula.h"

namespace DAVA
{
LayoutFormula::LayoutFormula()
{
    formula.Parse(source);
}

LayoutFormula::~LayoutFormula() = default;

const String& LayoutFormula::GetSource() const
{
    return source;
}

void LayoutFormula::SetSource(const String& str)
{
    source = str;
    status = eStatus::UNPROCESSED;
}

LayoutFormula::eStatus LayoutFormula::GetStatus() const
{
    return status;
}

bool LayoutFormula::IsValid() const
{
    return status == eStatus::OK && formula.IsValid();
}

LayoutFormula::eProcessResult LayoutFormula::Process()
{
    switch (status)
    {
    case eStatus::OK:
        // do nothing
        return eProcessResult::NOTHING_CHANGED;

    case LayoutFormula::eStatus::ERROR:
        // do nothing
        return eProcessResult::NOTHING_CHANGED;

    case eStatus::UNPROCESSED:
        if (!formula.Parse(source))
        {
            status = eStatus::ERROR;
            errorMsg = formula.GetParsingError();

            return eProcessResult::ERROR_GENERATED;
        }
        return eProcessResult::PARSED;

    case LayoutFormula::eStatus::RUNTIME_ERROR:
        status = eStatus::ERROR;
        return eProcessResult::ERROR_GENERATED;
    }
    return eProcessResult::NOTHING_CHANGED;
}

float32 LayoutFormula::Calculate(const Reflection& ref)
{
    Any res = formula.Calculate(ref);

    if (res.CanCast<float32>())
    {
        return res.Cast<float32>();
    }
    else if (res.CanCast<int32>())
    {
        return static_cast<float32>(res.Cast<int32>());
    }
    else if (res.IsEmpty())
    {
        errorMsg = formula.GetCalculationError();
        status = eStatus::RUNTIME_ERROR;

        DVASSERT(!errorMsg.empty());
    }

    return 0.0f;
}
}
