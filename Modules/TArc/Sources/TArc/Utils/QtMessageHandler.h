#pragma once

#include <QtGlobal>

class QString;
namespace DAVA
{
namespace TArc
{
void DAVAMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg);
} // namespace TArc
} // namespace DAVA
