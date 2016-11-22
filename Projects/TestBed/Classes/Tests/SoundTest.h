#pragma once

#include "Infrastructure/BaseScreen.h"

class TestBed;

class SoundTest final : public BaseScreen
{
public:
    SoundTest(TestBed& app);

protected:
    void LoadResources() override;

private:
    void OnPlaySoundGroup1(BaseObject* sender, void* data, void* callerData);
    void OnPlaySoundGroup2(BaseObject* sender, void* data, void* callerData);
    void OnApplySpeedGroup1(BaseObject* sender, void* data, void* callerData);
    void OnApplySpeedGroup2(BaseObject* sender, void* data, void* callerData);

    UITextField* speedTextFieldGroup1;
    UITextField* speedTextFieldGroup2;

    SoundEvent* eventGroup1;
    SoundEvent* eventGroup2;
};
