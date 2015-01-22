#ifndef __FRAMEWORKLOOP_H__
#define __FRAMEWORKLOOP_H__


#include "DavaLoop.h"


class FrameworkLoop
    : public LoopItem
{
    Q_OBJECT

public:
    FrameworkLoop();
    ~FrameworkLoop();

    void ProcessFrame() override;

private:
};



#endif // __FRAMEWORKLOOP_H__
