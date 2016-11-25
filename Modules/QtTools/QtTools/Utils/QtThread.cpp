#include "QtThread.h"

QtThread::QtThread(QObject* parent /*= nullptr*/)
    : QThread(parent)
{
}

#if defined(__DAVAENGINE_WINDOWS__)
void QtThread::run()
{
    QThread::exec();
}
#endif
