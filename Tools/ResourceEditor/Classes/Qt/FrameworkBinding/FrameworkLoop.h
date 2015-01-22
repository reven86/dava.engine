#ifndef __FRAMEWORKLOOP_H__
#define __FRAMEWORKLOOP_H__


#include "DavaLoop.h"
#include "Platform/Qt5/QtLayer.h"


class FrameworkLoop
    : public LoopItem
    , public DAVA::QtLayerDelegate
{
    Q_OBJECT

public:
    FrameworkLoop();
    ~FrameworkLoop();

    // LoopItem
    void ProcessFrame() override;

    // QtLayerDelegate
    void Quit() override;
    void ShowAssertMessage( const char* message ) override;

private:
};



#endif // __FRAMEWORKLOOP_H__
