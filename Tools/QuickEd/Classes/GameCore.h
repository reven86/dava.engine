#ifndef __GAMECORE_H__
#define __GAMECORE_H__

#include "Base/BaseTypes.h"
#include "Core/ApplicationCore.h"

namespace DAVA
{
class GameCore : public DAVA::ApplicationCore
{
protected:
    virtual ~GameCore();

public:
    GameCore();

    void OnAppStarted() override;
    void OnAppFinished() override;

    void OnSuspend() override;
    void OnResume() override;
    void OnBackground() override;

    void BeginFrame() override;
    void Update(DAVA::float32 update) override;
    void Draw() override;

protected:
    void UnpackHelp();
};
}

#endif // __GAMECORE_H__