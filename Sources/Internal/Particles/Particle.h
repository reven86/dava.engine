#ifndef __DAVAENGINE_PARTICLE_H__
#define __DAVAENGINE_PARTICLE_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/AllocatorFactory.h"

namespace DAVA
{
struct Particle
{
    IMPLEMENT_POOL_ALLOCATOR(Particle, 1000);

    Particle* next;

    float32 life;
    float32 lifeTime;

    Vector3 position;
    Vector3 speed;

    float32 angle;
    float32 spin;

    int32 frame;
    float32 animTime;

    float currRadius; //for bbox computation
    Vector2 baseSize, currSize;

    Color color;

    int32 positionTarget; //superemitter particles only
};
}

#endif // __DAVAENGINE_PARTICLE_H__
