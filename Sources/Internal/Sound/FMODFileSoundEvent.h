#ifdef DAVA_FMOD

#ifndef __DAVAENGINE_FMOD_FILE_SOUND_EVENT_H__
#define __DAVAENGINE_FMOD_FILE_SOUND_EVENT_H__

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Base/BaseMath.h"
#include "FileSystem/FilePath.h"
#include "Base/EventDispatcher.h"
#include "Sound/SoundEvent.h"
#include "Concurrency/Mutex.h"
#include "Sound/FMODUtils.h"

namespace FMOD
{
class Sound;
class ChannelGroup;
class Channel;
};

namespace DAVA
{
class FMODFileSoundEvent : public SoundEvent
{
public:
    static FMOD_RESULT F_CALLBACK SoundInstanceEndPlaying(FMOD_CHANNEL* channel, FMOD_CHANNEL_CALLBACKTYPE type, void* commanddata1, void* commanddata2);

    virtual int32 Release();

    virtual void SetVolume(float32 volume);

    virtual bool IsActive() const;
    virtual bool Trigger();
    virtual void Stop(bool force = false);
    virtual void SetPaused(bool paused);

    virtual void SetPosition(const Vector3& position);
    virtual void SetDirection(const Vector3& direction){};
    virtual void UpdateInstancesPosition();
    virtual void SetVelocity(const Vector3& velocity){};

    virtual void SetLoopCount(int32 looping); // -1 = infinity
    virtual int32 GetLoopCount();

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
        return fileName.GetFrameworkPath();
    };
    virtual float32 GetMaxDistance() const
    {
        return -1.f;
    };

protected:
    FMODFileSoundEvent(const FilePath& fileName, uint32 flags, int32 priority);
    virtual ~FMODFileSoundEvent();

    static FMODFileSoundEvent* CreateWithFlags(const FilePath& fileName, uint32 flags, int32 priority = 128);

    static Mutex soundMapMutex;

    Vector3 position;

    FilePath fileName;
    int32 priority;
    uint32 flags;

    FMOD::Sound* fmodSound;
    FMOD::ChannelGroup* fmodInstanceGroup;

    friend class SoundSystem;
};
};

#endif //__DAVAENGINE_FMOD_FILE_SOUND_EVENT_H__

#endif //DAVA_FMOD