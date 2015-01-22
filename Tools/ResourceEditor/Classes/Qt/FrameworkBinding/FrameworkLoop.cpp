#include "FrameworkLoop.h"

#include "Platform/Qt5/QtLayer.h"


FrameworkLoop::FrameworkLoop()
    : LoopItem()
{
    SetMaxFps( 60 );
}

FrameworkLoop::~FrameworkLoop()
{
}

void FrameworkLoop::ProcessFrame()
{
    DAVA::QtLayer::Instance()->ProcessFrame();
}
