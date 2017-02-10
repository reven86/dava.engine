#include "TArc/Utils/QtMessageHandler.h"

#include <Logger/Logger.h>
#include <Debug/DVAssert.h>

#include <QString>

namespace DAVA
{
namespace TArc
{
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
    {
        Vector<QString> ignoredStrings =
        {
          "QFileSystemWatcher: FindNextChangeNotification failed"
        };

        auto it = std::find_if(ignoredStrings.begin(), ignoredStrings.end(), [&msg](const QString ignored)
                               {
                                   return msg.contains(ignored);
                               });

        if (it == ignoredStrings.end())
        { //we should log only not ignored messages
            DAVA::Logger::Error("Qt Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        }

        break;
    }
    case QtFatalMsg:
        DAVA::Logger::Error("Qt Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        DVASSERT(false);
        break;
    default:
        DAVA::Logger::Info("Qt Unknown: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
        break;
    }
}

} // namespace TArc
} // namespace DAVA
