#pragma once

#include "Base/BaseTypes.h"
#include "UI/Formula/Formula.h"

namespace DAVA
{
class LayoutFormula final
{
public:
    enum class eStatus
    {
        UNPROCESSED,
        OK,
        RUNTIME_ERROR,
        ERROR
    };

    enum class eProcessResult
    {
        NOTHING_CHANGED,
        PARSED,
        ERROR_GENERATED,
    };

    LayoutFormula();
    ~LayoutFormula();

    const String& GetSource() const;
    void SetSource(const String& str);

    eStatus GetStatus() const;

    bool IsValid() const;
    eProcessResult Process();
    float32 Calculate(const Reflection& ref);

private:
    String source;
    Formula formula;
    eStatus status = eStatus::UNPROCESSED;
    String errorMsg;
};
}
