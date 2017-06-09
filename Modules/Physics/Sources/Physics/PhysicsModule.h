#pragma once

#include "ModuleManager/IModule.h"
#include "ModuleManager/ModuleManager.h"

#include "Base/Singleton.h"
#include "Base/BaseTypes.h"

namespace physx
{
class PxFoundation;
class PxPhysics;
class PxPvd;
}

namespace DAVA
{
class Physics : public IModule
{
public:
    Physics(Engine* engine);

    void Init() override;
    void Shutdown() override;

    bool IsInitialized() const;

private:
    physx::PxPvd* pvd = nullptr;
    physx::PxFoundation* foundation = nullptr;
    physx::PxPhysics* physics = nullptr;

    class PhysicsAllocator;
    PhysicsAllocator* allocator = nullptr;

    class PhysicsErrotCallback;
    PhysicsErrotCallback* errorCallback = nullptr;

    DAVA_VIRTUAL_REFLECTION(Physics, IModule);
};
};