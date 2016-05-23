#include "Input/Accelerometer.h"
namespace DAVA
{
Accelerometer::Accelerometer()
    : eventDispatcher(new EventDispatcher())
{
    accelerationData = Vector3(0.0f, 0.0f, 0.0f);
}

Accelerometer::~Accelerometer()
{
}

void Accelerometer::Enable(float32 /*updateRate*/)
{
}
};
