#pragma once

#include "QtTools/WarningGuard/QtWarningsHandler.h"
PUSH_QT_WARNING_SUPRESSOR
#include <QObject>
POP_QT_WARNING_SUPRESSOR

#include "Concurrency/Mutex.h"
#include "Debug/DVAssertMessage.h"
#include "Base/StaticSingleton.h"

class ToolsAssetGuard : public QObject, public DAVA::StaticSingleton<ToolsAssetGuard>
{
    PUSH_QT_WARNING_SUPRESSOR
    Q_OBJECT
    POP_QT_WARNING_SUPRESSOR
public:
    void Init();
    bool InnerShow(DAVA::DVAssertMessage::eModalType modalType, const DAVA::char8* message);

private:
    class EventFilter;
    DAVA::Mutex mutex;
};
