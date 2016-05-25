#ifndef ERRORVALIDATOR_H
#define ERRORVALIDATOR_H

#include "defines.h"
#include <QFile>

class ErrorMessanger
{
public:
    enum ErrorID
    {
        ERROR_DOC_ACCESS = 0,
        ERROR_NETWORK,
        ERROR_CONFIG,
        ERROR_UNPACK,
        ERROR_IS_RUNNING,

        ERROR_COUNT
    };

    ~ErrorMessanger();
    static ErrorMessanger* Instance();

    void ShowErrorMessage(ErrorID id, int errorCode = 0, const QString& addInfo = "");
    int ShowRetryDlg(bool canCancel);
    void ShowNotificationDlg(const QString& info);

    void LogMessage(QtMsgType, const QString& msg);

private:
    ErrorMessanger();

    static ErrorMessanger* instatnce;

    QFile logFile;
};

#endif // ERRORVALIDATOR_H
