#ifndef __DAVAENGINE_MUSIC_IOS_H__
#define __DAVAENGINE_MUSIC_IOS_H__

#ifdef __DAVAENGINE_IPHONE__

#include "Sound/SoundEvent.h"

namespace DAVA
{
class MusicIOSSoundEvent : public SoundEvent
{
public:
    static MusicIOSSoundEvent* CreateMusicEvent(const FilePath& path);

    virtual bool Trigger();
    virtual bool IsActive() const;
    virtual void Stop(bool force = false);
    virtual void SetPaused(bool paused);

    virtual void SetVolume(float32 volume);

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
    MusicIOSSoundEvent(const FilePath& path);
    virtual bool Init();
    virtual ~MusicIOSSoundEvent();

    void* avSound;
    FilePath filePath;
};
};

#endif //#ifdef __DAVAENGINE_IPHONE__

#endif //__DAVAENGINE_MUSIC_IOS_H__