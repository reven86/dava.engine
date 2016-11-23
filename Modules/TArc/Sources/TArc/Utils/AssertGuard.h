#pragma once

#include <QObject>

#include "Concurrency/Mutex.h"
#include "Debug/DVAssertMessage.h"
#include "Base/StaticSingleton.h"

namespace DAVA
{
namespace TArc
{
class ToolsAssertGuard : public QObject, public StaticSingleton<ToolsAssertGuard>
{
    Q_OBJECT
public:
    void Init();
    bool InnerShow(DVAssertMessage::eModalType modalType, const char8* message);

private:
    class EventFilter;
    Mutex mutex;
};
} // namespace TArc
} // namespace DAVA
