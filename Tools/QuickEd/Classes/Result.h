#ifndef ERROR_H
#define ERROR_H

#include <QString>
#include <QStringList>

struct Result
{
    enum ResultType {
        Success,
        DAVAError,
        StupidError,
        CriticalError,
        Count
    };
    Result(ResultType type = Count, QString error = "");
    operator bool()
    {
        return types.isEmpty() || types.contains(StupidError) || types.contains(CriticalError);
    }
    QStringList errors;
    QList<ResultType> types;
    Result addError(ResultType type, QString errorText);
    Result addError(const Result &err);
};
#endif // ERROR_H
