#ifndef QUICKED_RESULT_H_
#define QUICKED_RESULT_H_

#include <QString>
#include <QStringList>

struct Result
{
    enum ResultType {
        Success,
        DAVAError,
        Warning,
        CriticalError,
    };
    explicit Result(ResultType type = Success, const QString &error = QString());
    operator bool() const;
    QStringList errors;
    QList<ResultType> types;
    Result addError(ResultType type, const QString &errorText);
    Result addError(const Result &err); 
};
#endif // QUICKED_RESULT_H_
