#pragma once

#include "Physics/PhysicsModule.h"

#include <Logger/Logger.h>
#include <MemoryManager/MemoryManager.h>

#include <physx/PxPhysicsAPI.h>

namespace DAVA
{
class Physics::PhysicsAllocator : public physx::PxAllocatorCallback
{
public:
    void* allocate(size_t size, const char* typeName, const char* filename, int line) override
    {
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
        return MemoryManager::Instance()->AlignedAllocate(size, 16, ALLOC_POOL_PHYSICS);
#else
        return defaultAllocator.allocate(size, typeName, filename, line);
#endif
    }

    void deallocate(void* ptr) override
    {
#if defined(DAVA_MEMORY_PROFILING_ENABLE)
        MemoryManager::Instance()->Deallocate(ptr);
#else
        defaultAllocator.deallocate(ptr);
#endif
    }

private:
    physx::PxDefaultAllocator defaultAllocator;
};

class Physics::PhysicsErrotCallback : public physx::PxErrorCallback
{
public:
    void reportError(physx::PxErrorCode::Enum code, const char* message, const char* file, int line) override
    {
        defaultErrorCallback.reportError(code, message, file, line);
    }

private:
    physx::PxDefaultErrorCallback defaultErrorCallback;
};

Physics::Physics(Engine* engine)
    : IModule(engine)
{
}

void Physics::Init()
{
    using namespace physx;

    allocator = new PhysicsAllocator();
    errorCallback = new PhysicsErrotCallback();

    foundation = PxCreateFoundation(PX_FOUNDATION_VERSION, *allocator, *errorCallback);
    DVASSERT(foundation);

    physics = PxCreateBasePhysics(PX_PHYSICS_VERSION, *foundation, PxTolerancesScale(), true, nullptr); // TODO add profiler zone from PhysicsDebugging
    DVASSERT(physics);
    PxRegisterHeightFields(*physics);
}

void Physics::Shutdown()
{
    physics->release();
    foundation->release();
    SafeDelete(allocator);
    SafeDelete(errorCallback);
}

bool Physics::IsInitialized() const
{
    return foundation != nullptr && physics != nullptr;
}
}