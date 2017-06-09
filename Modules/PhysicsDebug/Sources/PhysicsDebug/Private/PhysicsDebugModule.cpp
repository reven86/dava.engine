#include "PhysicsDebug/PhysicsDebugModule.h"
#include "Reflection/Reflection.h"

#include <physx/PxPhysicsAPI.h>
#include <PxShared/pvd/PxPvd.h>
#include <PxShared/pvd/PxPvdTransport.h>

namespace DAVA
{
PhysicsDebug::PhysicsDebug(Engine* engine)
    : IModule(engine)
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PhysicsDebug);
}

void PhysicsDebug::Init()
{
}

void PhysicsDebug::Shutdown()
{
}

} // namespace DAVA
