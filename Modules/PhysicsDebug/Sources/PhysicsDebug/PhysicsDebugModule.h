#pragma once

#include <ModuleManager/IModule.h>
#include <Math/Vector.h>

namespace physx
{
class PxFoundation;
class PxPvd;
class PxPvdTransport;
} // namespace physx

namespace DAVA
{
class PhysicsDebug : public IModule
{
public:
    PhysicsDebug(Engine* engine);
    ~PhysicsDebug();
    void Init() override;
    void Shutdown() override;

private:
    physx::PxPvd* CreatePvd(physx::PxFoundation* foundation);
    void ReleasePvd();

    physx::PxPvd* pvd = nullptr;
    physx::PxPvdTransport* transport = nullptr;

    DAVA_VIRTUAL_REFLECTION(PhysicsDebug, IModule);
};
}