#include "result.h"

Result::Result(ResultType type, const QString &error)
{
    if (type != Result::Count)
    {
        types << type;
        errors << error;
    }
}

Result::operator bool() const
{
    return types.isEmpty() || types.contains(StupidError) || types.contains(CriticalError);
}

Result Result::addError(Result::ResultType type, const QString &errorText)
{
    types << type;
    errors << errorText;
    return *this;
}

Result Result::addError(const Result &err)
{
    types << err.types;
    errors << err.errors;
    return *this;
}
