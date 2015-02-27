#include "result.h"

Result::Result(ResultType type, QString error)
{
    if (type != Result::Count)
    {
        types << type;
        errors << error;
    }
}

Result Result::addError(Result::ResultType type, QString errorText)
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
