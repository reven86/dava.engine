#pragma once

#include "Sound/SoundEvent.h"

DAVA_FORWARD_DECLARE_OBJC_CLASS(AvSound);

namespace DAVA
{
class FMODSoundSystem;
class MusicIOSSoundEvent : public SoundEvent
{
public:
    static MusicIOSSoundEvent* CreateMusicEvent(const FilePath& path, FMODSoundSystem* fmodSoundSystem);

    virtual bool Trigger();
    virtual bool IsActive() const;
    virtual void Stop(bool force = false);
    virtual void SetPaused(bool paused);

    virtual void SetVolume(float32 volume);

    virtual void SetSpeed(float32 speed);

    virtual void SetLoopCount(int32 looping); // -1 = infinity
    virtual int32 GetLoopCount() const;

    virtual void SetPosition(const Vector3& position){};
    virtual void SetDirection(const Vector3& direction){};
    virtual void UpdateInstancesPosition(){};
    virtual void SetVelocity(const Vector3& velocity){};

    virtual void SetParameterValue(const FastName& paramName, float32 value){};
    virtual float32 GetParameterValue(const FastName& paramName)
    {
        return 0.f;
    };
    virtual bool IsParameterExists(const FastName& paramName)
    {
        return false;
    };

    virtual void GetEventParametersInfo(Vector<SoundEventParameterInfo>& paramsInfo) const
    {
        return;
    };

    virtual String GetEventName() const
    {
        return filePath.GetFrameworkPath();
    };
    virtual float32 GetMaxDistance() const
    {
        return -1.f;
    };

    void PerformEndCallback();

protected:
    MusicIOSSoundEvent(const FilePath& path, FMODSoundSystem* fmodSoundSystem);
    virtual bool Init();
    virtual ~MusicIOSSoundEvent();

    AvSound* avSound = nullptr;
    FilePath filePath;
    FMODSoundSystem* soundSystem = nullptr;
};
};
