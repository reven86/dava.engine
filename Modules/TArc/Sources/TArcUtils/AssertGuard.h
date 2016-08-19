#pragma once

#include <QObject>

#include "Concurrency/Mutex.h"
#include "Debug/DVAssertMessage.h"
#include "Base/StaticSingleton.h"

class ToolsAssetGuard : public QObject, public DAVA::StaticSingleton<ToolsAssetGuard>
{
    Q_OBJECT
public:
    void Init();
    bool InnerShow(DAVA::DVAssertMessage::eModalType modalType, const DAVA::char8* message);

private:
    class EventFilter;
    DAVA::Mutex mutex;
};
