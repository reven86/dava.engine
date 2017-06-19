#pragma once

#include <Math/Vector.h>

namespace DAVA
{
struct PhysicsSceneConfig
{
    Vector3 gravity = { 0, 0, -9.81f }; //physics gravity
    //uint32 simulationBlockSize = 16 * 1024 * 512; //must be 16K multiplier
    uint32 threadCount = 2; //number of threads created for physics task dispatcher
};
}