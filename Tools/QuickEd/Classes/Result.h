#ifndef QUICKED_RESULT_H_
#define QUICKED_RESULT_H_

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
    Result(ResultType type = Count, const QString &error = "");
    operator bool() const;
    QStringList errors;
    QList<ResultType> types;
    Result addError(ResultType type, const QString &errorText);
    Result addError(const Result &err);
};
#endif // QUICKED_RESULT_H_
