#include "result.h"

Result::Result(ResultType type, const QString &error)
{
        types << type;
        errors << error;
}

Result::operator bool() const
{
    for (auto type : types)
    {
        if (type != Success)
        {
            return false;
        }
    }
    return true;
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
