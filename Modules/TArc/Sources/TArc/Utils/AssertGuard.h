#pragma once

#include <QObject>

#include "Concurrency/Mutex.h"
#include "Debug/DVAssertMessage.h"
#include "Base/StaticSingleton.h"

namespace DAVA
{
namespace TArc
{
class ToolsAssetGuard : public QObject, public StaticSingleton<ToolsAssetGuard>
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
