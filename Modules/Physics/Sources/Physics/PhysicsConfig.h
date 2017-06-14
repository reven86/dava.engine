#pragma once

#include "Math/Vector.h"

namespace DAVA
{
struct PhysicsConfig
{
    Vector3 gravity = { 0, 0, -9.81f }; //physics gravity
    uint32 simulationBlockSize = 16 * 1024 * 512; //must be 16K multiplier
    uint32 threadCount = 2; //number of threads created for physics task dispatcher
    bool enableProfile = true; //enable SDK and PVD profiling
    bool enableCooking = true; //enable cooking subsystem. Should be true for editors and false for game
};
}