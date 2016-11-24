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
    void OnPlaySoundGroup1(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnPlaySoundGroup2(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnApplySpeedGroup1(DAVA::BaseObject* sender, void* data, void* callerData);
    void OnApplySpeedGroup2(DAVA::BaseObject* sender, void* data, void* callerData);

    DAVA::UITextField* speedTextFieldGroup1;
    DAVA::UITextField* speedTextFieldGroup2;

    DAVA::SoundEvent* eventGroup1;
    DAVA::SoundEvent* eventGroup2;
};
