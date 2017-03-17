#pragma once

#include "TArc/Controls/PropertyPanel/PropertiesView.h"
#include "TArc/Utils/QtConnections.h"

class QTimer;
namespace DAVA
{
namespace TArc
{
class TimerUpdater final : public PropertiesView::Updater
{
public:
    static const int32 DisableFastUpdate;
    TimerUpdater(int32 fullUpdateMS, int32 fastUpdateMS);
    ~TimerUpdater();

private:
    std::unique_ptr<QTimer> fullUpdateTimer;
    std::unique_ptr<QTimer> fastUpdateTimer;
    int32 counter = 1;
    QtConnections connections;
};
} // namespace TArc
} // namespace DAVA