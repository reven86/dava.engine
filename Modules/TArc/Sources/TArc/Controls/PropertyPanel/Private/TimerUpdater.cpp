#include "TArc/Controls/PropertyPanel/TimerUpdater.h"

#include <QTimer>

namespace DAVA
{
namespace TArc
{
const int32 TimerUpdater::DisableFastUpdate = -1;

TimerUpdater::TimerUpdater(int32 fullUpdateMS, int32 fastUpdateMS)
{
    DVASSERT(fullUpdateMS > fastUpdateMS);
    DVASSERT(fullUpdateMS > 0);
    DVASSERT(fastUpdateMS != 0);
    if (fastUpdateMS == DisableFastUpdate)
    {
        fullUpdateTimer.reset(new QTimer());
        fullUpdateTimer->setInterval(fullUpdateMS);
        connections.AddConnection(fullUpdateTimer.get(), &QTimer::timeout, Bind(&Signal<PropertiesView::UpdatePolicy>::Emit, &update, PropertiesView::FullUpdate));
    }
    else if (fullUpdateMS % fastUpdateMS == 0)
    {
        int32 fullUpdatePeriod = fullUpdateMS / fastUpdateMS;
        fastUpdateTimer.reset(new QTimer());
        fastUpdateTimer->setInterval(fastUpdateMS);
        connections.AddConnection(fastUpdateTimer.get(), &QTimer::timeout, [this, fullUpdatePeriod]()
                                  {
                                      if (counter == fullUpdatePeriod)
                                      {
                                          update.Emit(PropertiesView::FullUpdate);
                                          counter = 1;
                                      }
                                      else
                                      {
                                          update.Emit(PropertiesView::FastUpdate);
                                          ++counter;
                                      }
                                  });
    }
    else
    {
        fastUpdateTimer.reset(new QTimer());
        fastUpdateTimer->setInterval(fastUpdateMS);
        connections.AddConnection(fastUpdateTimer.get(), &QTimer::timeout, Bind(&Signal<PropertiesView::UpdatePolicy>::Emit, &update, PropertiesView::FastUpdate));

        fullUpdateTimer.reset(new QTimer());
        fullUpdateTimer->setInterval(fullUpdateMS);
        connections.AddConnection(fullUpdateTimer.get(), &QTimer::timeout, Bind(&Signal<PropertiesView::UpdatePolicy>::Emit, &update, PropertiesView::FullUpdate));
    }

    if (fastUpdateTimer != nullptr)
    {
        fastUpdateTimer->start();
    }

    if (fullUpdateTimer != nullptr)
    {
        fullUpdateTimer->start();
    }
}

TimerUpdater::~TimerUpdater() = default;
} // namespace TArc
} // namespace DAVA