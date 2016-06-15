#ifndef __QT_TOOLS_MESSAGE_HANDLER_H__
#define __QT_TOOLS_MESSAGE_HANDLER_H__

#include "Debug/DVAssert.h"

void DAVAMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    switch (type)
    {
    case QtDebugMsg:
        DAVA::Logger::Debug("Qt debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        DAVA::Logger::Warning("Qt Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        DAVA::Logger::Error("Qt Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        DVASSERT(false);
        break;
    case QtFatalMsg:
        DAVA::Logger::Error("Qt Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        DVASSERT(false);
        break;
    default:
        DAVA::Logger::Info("Qt Unknown: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    }
}

#endif // __QT_TOOLS_MESSAGE_HANDLER_H__
