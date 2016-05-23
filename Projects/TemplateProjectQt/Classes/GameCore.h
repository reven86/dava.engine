#ifndef __GAMECORE_H__
#define __GAMECORE_H__

#include "DAVAEngine.h"

class GameCore : public DAVA::ApplicationCore
{
public:
    GameCore();
    ~GameCore() override;

    void OnAppStarted() override;
    void OnAppFinished() override;
};

#endif // __GAMECORE_H__