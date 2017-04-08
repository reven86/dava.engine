#ifndef __DAVAENGINE_ACCELEROMETER_ANDROID_H__
#define __DAVAENGINE_ACCELEROMETER_ANDROID_H__

#include "Input/Accelerometer.h"
#include "Base/EventDispatcher.h"

#ifdef __DAVAENGINE_ANDROID__

namespace DAVA
{
class AccelerometerAndroidImpl : public Accelerometer
{
    IMPLEMENT_EVENT_DISPATCHER(eventDispatcher);

public:
    AccelerometerAndroidImpl();
    ~AccelerometerAndroidImpl();

    virtual void Enable(float32 updateRate);
    virtual void Disable();
    void SetAccelerationData(float x, float y, float z);
    EventDispatcher* GetEventDispatcher();

private:
    bool enabled;
    float32 updRate;
    int64 lastUpdate;
};
};
#endif // __DAVAENGINE_ANDROID__
#endif // __DAVAENGINE_ACCELEROMETER_ANDROID_H__
