#ifndef PROCESSHELPER_H
#define PROCESSHELPER_H

#include <QString>
#ifdef Q_OS_DARWIN
//#include "Processes.h"
#endif

class ProcessHelper
{
public:
    static bool IsProcessRuning(const QString& path);
    static void SetActiveProcess(const QString& path);
    static void RunProcess(const QString& path);

private:
#ifdef Q_OS_DARWIN
//    static bool GetProcessPSN(const QString& path, ProcessSerialNumber& psn);
#elif defined Q_OS_WIN
    static bool GetProcessID(QString path, quint32& dwPID);
#endif
};

#endif // PROCESSHELPER_H
